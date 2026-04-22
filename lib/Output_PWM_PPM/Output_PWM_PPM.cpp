/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Output_PWM_PPM.h"

#include <ESP32Servo.h>
#include "Protocol_SBUS_ibus.h"

namespace AetherLink {
namespace Output {

static Servo s_servos[kMaxChannels];
static bool s_attached[kMaxChannels] = {false};

void Init(const RuntimeConfig& config) {
  pinMode(config.pwm_pins[0], OUTPUT);
  for (uint8_t i = 0U; i < kMaxChannels; ++i) {
    if (config.pwm_enabled[i]) {
      s_servos[i].setPeriodHertz(50U);
      s_attached[i] = s_servos[i].attach(config.pwm_pins[i], kPwmMinUs, kPwmMaxUs) > 0;
    }
  }
}

void WriteChannels(const RxFrame& frame, const RuntimeConfig& config) {
  for (uint8_t i = 0U; i < kMaxChannels; ++i) {
    if (config.pwm_enabled[i] && s_attached[i]) {
      s_servos[i].writeMicroseconds(constrain(frame.channels[i], kPwmMinUs, kPwmMaxUs));
    }
  }
}

void WriteSbus(const RxFrame& frame, HardwareSerial& port) {
  uint8_t packet[25] = {0};
  packet[0] = 0x0F;
  uint16_t ch[kMaxChannels];
  for (uint8_t i = 0U; i < kMaxChannels; ++i) {
    ch[i] = static_cast<uint16_t>(map(frame.channels[i], 1000, 2000, 172, 1811));
  }
  packet[1] = static_cast<uint8_t>(ch[0] & 0xFFU);
  packet[2] = static_cast<uint8_t>((ch[0] >> 8U) | ((ch[1] & 0x07U) << 3U));
  packet[3] = static_cast<uint8_t>((ch[1] >> 5U) | ((ch[2] & 0x3FU) << 6U));
  packet[24] = 0x00;
  (void)port.write(packet, sizeof(packet));
}

void WriteIbus(const RxFrame& frame, HardwareSerial& port) {
  uint8_t packet[32] = {0};
  const size_t len = SBUSIbus::BuildIbusPacket(frame, packet, sizeof(packet));
  if (len > 0U) {
    (void)port.write(packet, len);
  }
}

void WriteCrsf(const RxFrame& frame, HardwareSerial& port) {
  uint8_t pkt[26] = {0};
  pkt[0] = 0xC8;
  pkt[1] = 24U;
  pkt[2] = 0x16;
  for (uint8_t i = 0U; i < kMaxChannels; ++i) {
    pkt[3U + i] = static_cast<uint8_t>(map(frame.channels[i], 1000, 2000, 0, 255));
  }
  uint8_t crc = 0U;
  for (uint8_t i = 2U; i < 25U; ++i) {
    crc ^= pkt[i];
  }
  pkt[25] = crc;
  (void)port.write(pkt, sizeof(pkt));
}

void WriteCppm(const RxFrame& frame, uint8_t pin) {
  constexpr uint16_t kPulseUs = 300U;
  uint32_t frame_sum = 0U;
  for (uint8_t i = 0U; i < 8U; ++i) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(kPulseUs);
    digitalWrite(pin, LOW);
    const uint16_t ch = static_cast<uint16_t>(constrain(frame.channels[i], 1000, 2000));
    delayMicroseconds(ch - kPulseUs);
    frame_sum += ch;
  }
  const uint16_t sync_us = static_cast<uint16_t>(max(3000L, 22500L - static_cast<long>(frame_sum)));
  delayMicroseconds(sync_us);
}

}  // namespace Output
}  // namespace AetherLink
