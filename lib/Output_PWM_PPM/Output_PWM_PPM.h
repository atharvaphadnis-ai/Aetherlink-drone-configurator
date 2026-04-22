/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AETHERLINK_OUTPUT_PWM_PPM_H
#define AETHERLINK_OUTPUT_PWM_PPM_H

#include <Arduino.h>
#include "AetherLink_Config.h"

namespace AetherLink {
namespace Output {
void Init(const RuntimeConfig& config);
void WriteChannels(const RxFrame& frame, const RuntimeConfig& config);
void WriteSbus(const RxFrame& frame, HardwareSerial& port);
void WriteIbus(const RxFrame& frame, HardwareSerial& port);
void WriteCrsf(const RxFrame& frame, HardwareSerial& port);
void WriteCppm(const RxFrame& frame, uint8_t pin);
}  // namespace Output
}  // namespace AetherLink

#endif
