/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AETHERLINK_PROTOCOL_ESPNOW_H
#define AETHERLINK_PROTOCOL_ESPNOW_H

#include <Arduino.h>
#include "AetherLink_Config.h"

namespace AetherLink {
namespace ESPNOWProto {
void Init();
bool Poll(RxFrame& out_frame);
bool SendTelemetry(const uint8_t* data, size_t len);
bool IsBound();
}  // namespace ESPNOWProto
}  // namespace AetherLink

#endif
