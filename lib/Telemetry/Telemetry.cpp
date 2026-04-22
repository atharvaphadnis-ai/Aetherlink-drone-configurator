/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Telemetry.h"

#include <Wire.h>

namespace AetherLink {
namespace Telemetry {

static uint8_t s_adc_pin = 4U;
static float s_divider = 11.0F;
static bool s_imu_ok = false;
static float s_roll = 0.0F;
static float s_pitch = 0.0F;

static bool InitImuMpu6050() {
  Wire.begin();
  Wire.beginTransmission(0x68U);
  Wire.write(0x6BU);
  Wire.write(0x00U);
  if (Wire.endTransmission() != 0U) {
    return false;
  }
  return true;
}

static bool ReadImu(float& roll_deg, float& pitch_deg) {
  Wire.beginTransmission(0x68U);
  Wire.write(0x3BU);
  if (Wire.endTransmission(false) != 0U) {
    return false;
  }
  if (Wire.requestFrom(static_cast<uint8_t>(0x68U), static_cast<uint8_t>(6U)) != 6U) {
    return false;
  }
  const int16_t ax = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
  const int16_t ay = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
  const int16_t az = static_cast<int16_t>((Wire.read() << 8) | Wire.read());
  const float axf = static_cast<float>(ax) / 16384.0F;
  const float ayf = static_cast<float>(ay) / 16384.0F;
  const float azf = static_cast<float>(az) / 16384.0F;
  roll_deg = atan2f(ayf, azf) * 57.2958F;
  pitch_deg = atan2f(-axf, sqrtf(ayf * ayf + azf * azf)) * 57.2958F;
  return true;
}

void Init(uint8_t battery_adc_pin, float divider_ratio) {
  s_adc_pin = battery_adc_pin;
  s_divider = divider_ratio;
  analogReadResolution(12U);
  s_imu_ok = InitImuMpu6050();
}

Data Build(const RxFrame& frame) {
  Data data{};
  const uint16_t raw = analogRead(s_adc_pin);
  data.battery_v = (static_cast<float>(raw) / 4095.0F) * 3.3F * s_divider;
  data.rssi = frame.rssi;
  float roll_sample = 0.0F;
  float pitch_sample = 0.0F;
  if (s_imu_ok && ReadImu(roll_sample, pitch_sample)) {
    s_roll = 0.85F * s_roll + 0.15F * roll_sample;
    s_pitch = 0.85F * s_pitch + 0.15F * pitch_sample;
  } else {
    s_roll = static_cast<float>(frame.channels[0] - 1500) * 0.09F;
    s_pitch = static_cast<float>(frame.channels[1] - 1500) * 0.09F;
  }
  data.roll_deg = s_roll;
  data.pitch_deg = s_pitch;
  return data;
}

bool ImuHealthy() {
  return s_imu_ok;
}

}  // namespace Telemetry
}  // namespace AetherLink
