/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Commercial licensing exception is available directly from Atharva Phadnis.
 */

#ifndef AETHERLINK_CONFIG_H
#define AETHERLINK_CONFIG_H

#include <Arduino.h>

namespace AetherLink {

static constexpr uint8_t kMaxChannels = 16U;
static constexpr uint16_t kPwmMinUs = 1000U;
static constexpr uint16_t kPwmMaxUs = 2000U;
static constexpr uint16_t kPwmCenterUs = 1500U;
static constexpr uint16_t kFailsafeTimeoutMs = 250U;
static constexpr uint32_t kTelemetryPeriodMs = 50U;
static constexpr uint32_t kCliBaud = 115200UL;
static constexpr uint8_t kReceiverTaskPrio = 6U;
static constexpr uint8_t kOutputTaskPrio = 7U;
static constexpr uint8_t kTelemetryTaskPrio = 4U;
static constexpr uint8_t kControlTaskPrio = 3U;

enum class InputProtocol : uint8_t {
  NONE = 0,
  ELRS,
  ESPNOW,
  BLE,
  NRF24
};

enum class OutputProtocol : uint8_t {
  PWM_ONLY = 0,
  CPPM,
  SBUS,
  IBUS,
  CRSF
};

enum class FailsafeMode : uint8_t {
  HOLD = 0,
  CUT,
  CUSTOM
};

struct RxFrame {
  uint32_t timestamp_ms;
  int16_t channels[kMaxChannels];
  uint8_t rssi;
  bool frame_valid;
};

struct RuntimeConfig {
  InputProtocol input_lock;
  OutputProtocol output_mode;
  FailsafeMode failsafe_mode;
  int16_t custom_failsafe[kMaxChannels];
  uint8_t pwm_pins[kMaxChannels];
  bool pwm_enabled[kMaxChannels];
  uint8_t battery_adc_pin;
  float battery_divider_ratio;
  bool bridge_mode_enabled;
};

extern RuntimeConfig g_config;

}  // namespace AetherLink

#endif
