/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Principal author and owner: Atharva Phadnis.
 */

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_task_wdt.h>

#include "AetherLink_Config.h"
#include "Output_PWM_PPM.h"
#include "Protocol_ELRS.h"
#include "Protocol_ESPNOW.h"
#include "Protocol_NRF24.h"
#include "Telemetry.h"
#include "failsafe_manager.h"
#include "hardware_init.h"
#include "web_interface.h"

namespace AetherLink {

static QueueHandle_t s_rx_queue = nullptr;
static QueueHandle_t s_telem_queue = nullptr;
static Preferences s_prefs;
static RxFrame s_last_frame{};
static uint32_t s_last_rx_ms = 0U;
static WiFiServer s_bridge_server(5760U);
static WiFiClient s_bridge_client;

static void LoadConfigFromNvs() {
  s_prefs.begin("aetherlink", false);
  g_config.input_lock = static_cast<InputProtocol>(s_prefs.getUChar("in", static_cast<uint8_t>(InputProtocol::NONE)));
  g_config.output_mode = static_cast<OutputProtocol>(s_prefs.getUChar("out", static_cast<uint8_t>(OutputProtocol::PWM_ONLY)));
  g_config.failsafe_mode = static_cast<FailsafeMode>(s_prefs.getUChar("fsm", static_cast<uint8_t>(FailsafeMode::HOLD)));
  g_config.bridge_mode_enabled = s_prefs.getBool("bridge", false);
}

static void SaveConfigToNvs() {
  s_prefs.putUChar("in", static_cast<uint8_t>(g_config.input_lock));
  s_prefs.putUChar("out", static_cast<uint8_t>(g_config.output_mode));
  s_prefs.putUChar("fsm", static_cast<uint8_t>(g_config.failsafe_mode));
  s_prefs.putBool("bridge", g_config.bridge_mode_enabled);
}

static void SetupBleGATT() {
  NimBLEDevice::init("AetherLink");
  NimBLEServer* server = NimBLEDevice::createServer();
  NimBLEService* service = server->createService("19B10000-E8F2-537E-4F6C-D104768A1214");
  NimBLECharacteristic* cfg_char =
      service->createCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  cfg_char->setValue("AetherLink BLE Ready");
  service->start();
  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(service->getUUID());
  adv->start();
}

static InputProtocol AutoDetectInput() {
  if (ELRS::IsLinked()) {
    return InputProtocol::ELRS;
  }
  if (ESPNOWProto::IsBound()) {
    return InputProtocol::ESPNOW;
  }
  if (NRF24Proto::IsLinked()) {
    return InputProtocol::NRF24;
  }
  return InputProtocol::NONE;
}

static void ReceiverTask(void*) {
  (void)esp_task_wdt_add(nullptr);
  RxFrame frame{};
  for (;;) {
    bool valid = false;
    switch (g_config.input_lock) {
      case InputProtocol::ELRS:
        valid = ELRS::Poll(frame);
        break;
      case InputProtocol::ESPNOW:
        valid = ESPNOWProto::Poll(frame);
        break;
      case InputProtocol::NRF24:
        valid = NRF24Proto::Poll(frame);
        break;
      default:
        g_config.input_lock = AutoDetectInput();
        break;
    }
    if (valid) {
      s_last_rx_ms = millis();
      s_last_frame = frame;
      (void)xQueueOverwrite(s_rx_queue, &frame);
      (void)xQueueOverwrite(s_telem_queue, &frame);
    }
    (void)esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(2U));
  }
}

static void OutputTask(void*) {
  (void)esp_task_wdt_add(nullptr);
  RxFrame frame = s_last_frame;
  for (;;) {
    if (xQueueReceive(s_rx_queue, &frame, pdMS_TO_TICKS(5U)) == pdTRUE) {
      s_last_frame = frame;
    } else {
      frame = s_last_frame;
    }
    (void)ApplyFailsafeIfNeeded(frame, s_last_rx_ms, g_config);
    Output::WriteChannels(frame, g_config);
    if (g_config.output_mode == OutputProtocol::SBUS) {
      Output::WriteSbus(frame, Serial2);
    } else if (g_config.output_mode == OutputProtocol::IBUS) {
      Output::WriteIbus(frame, Serial2);
    } else if (g_config.output_mode == OutputProtocol::CRSF) {
      Output::WriteCrsf(frame, Serial2);
    } else if (g_config.output_mode == OutputProtocol::CPPM) {
      Output::WriteCppm(frame, g_config.pwm_pins[0]);
    }
    (void)esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(4U));
  }
}

static void ServiceBridgeMode() {
  if (!g_config.bridge_mode_enabled) {
    return;
  }
  if (!s_bridge_client || !s_bridge_client.connected()) {
    s_bridge_client = s_bridge_server.available();
  }
  if (!s_bridge_client || !s_bridge_client.connected()) {
    return;
  }
  while (s_bridge_client.available() > 0) {
    Serial2.write(static_cast<uint8_t>(s_bridge_client.read()));
  }
  while (Serial2.available() > 0) {
    s_bridge_client.write(static_cast<uint8_t>(Serial2.read()));
  }
}

static void TelemetryTask(void*) {
  (void)esp_task_wdt_add(nullptr);
  RxFrame frame = s_last_frame;
  for (;;) {
    if (xQueueReceive(s_telem_queue, &frame, pdMS_TO_TICKS(kTelemetryPeriodMs)) != pdTRUE) {
      frame = s_last_frame;
    }
    const Telemetry::Data t = Telemetry::Build(frame);
    ELRS::SendTelemetry(t.rssi, t.battery_v);
    uint8_t compact[4] = {t.rssi, static_cast<uint8_t>(t.battery_v * 10.0F), static_cast<uint8_t>(t.roll_deg + 90.0F),
                          static_cast<uint8_t>(t.pitch_deg + 90.0F)};
    (void)ESPNOWProto::SendTelemetry(compact, sizeof(compact));
    (void)esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(kTelemetryPeriodMs));
  }
}

static void ControlTask(void*) {
  (void)esp_task_wdt_add(nullptr);
  for (;;) {
    WebInterfacePoll();
    ServiceBridgeMode();
    if (Serial.available() > 0) {
      const String cmd = Serial.readStringUntil('\n');
      if (cmd.startsWith("set failsafe hold")) {
        g_config.failsafe_mode = FailsafeMode::HOLD;
      } else if (cmd.startsWith("set failsafe cut")) {
        g_config.failsafe_mode = FailsafeMode::CUT;
      } else if (cmd.startsWith("set output sbus")) {
        g_config.output_mode = OutputProtocol::SBUS;
      } else if (cmd.startsWith("set output ibus")) {
        g_config.output_mode = OutputProtocol::IBUS;
      } else if (cmd.startsWith("set output crsf")) {
        g_config.output_mode = OutputProtocol::CRSF;
      } else if (cmd.startsWith("set output cppm")) {
        g_config.output_mode = OutputProtocol::CPPM;
      } else if (cmd.startsWith("save")) {
        SaveConfigToNvs();
      }
    }
    (void)esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(10U));
  }
}

}  // namespace AetherLink

void setup() {
  using namespace AetherLink;
  InitHardware();
  LoadConfigFromNvs();
  ELRS::Init(Serial1);
  Serial2.begin(100000U, SERIAL_8E2);
  ESPNOWProto::Init();
  NRF24Proto::Init(16U, 15U);
  Output::Init(g_config);
  Telemetry::Init(g_config.battery_adc_pin, g_config.battery_divider_ratio);
  SetupBleGATT();
  WebInterfaceInit(&g_config);
  s_bridge_server.begin();

  s_rx_queue = xQueueCreate(1U, sizeof(RxFrame));
  s_telem_queue = xQueueCreate(1U, sizeof(RxFrame));
  configASSERT(s_rx_queue != nullptr);
  configASSERT(s_telem_queue != nullptr);

  xTaskCreatePinnedToCore(ReceiverTask, "rx_task", 4096U, nullptr, kReceiverTaskPrio, nullptr, 0U);
  xTaskCreatePinnedToCore(OutputTask, "out_task", 4096U, nullptr, kOutputTaskPrio, nullptr, 1U);
  xTaskCreatePinnedToCore(TelemetryTask, "telem_task", 4096U, nullptr, kTelemetryTaskPrio, nullptr, 1U);
  xTaskCreatePinnedToCore(ControlTask, "ctrl_task", 4096U, nullptr, kControlTaskPrio, nullptr, 0U);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000U));
}
