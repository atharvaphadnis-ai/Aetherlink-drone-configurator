/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AETHERLINK_PROTOCOL_ELRS_H
#define AETHERLINK_PROTOCOL_ELRS_H

#include <Arduino.h>
#include "AetherLink_Config.h"

namespace AetherLink {
namespace ELRS {
void Init(HardwareSerial& serial_port);
bool Poll(RxFrame& out_frame);
void SendTelemetry(uint8_t rssi, float battery_v);
bool IsLinked();
}  // namespace ELRS
}  // namespace AetherLink

#endif
