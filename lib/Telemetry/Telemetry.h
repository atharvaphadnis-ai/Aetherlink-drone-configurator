/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AETHERLINK_TELEMETRY_H
#define AETHERLINK_TELEMETRY_H

#include <Arduino.h>
#include "AetherLink_Config.h"

namespace AetherLink {
namespace Telemetry {
struct Data {
  uint8_t rssi;
  float battery_v;
  float roll_deg;
  float pitch_deg;
};

void Init(uint8_t battery_adc_pin, float divider_ratio);
Data Build(const RxFrame& frame);
bool ImuHealthy();
}  // namespace Telemetry
}  // namespace AetherLink

#endif
