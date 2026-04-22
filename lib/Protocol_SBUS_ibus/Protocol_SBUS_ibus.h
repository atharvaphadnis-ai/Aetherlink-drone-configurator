/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AETHERLINK_PROTOCOL_SBUS_IBUS_H
#define AETHERLINK_PROTOCOL_SBUS_IBUS_H

#include <Arduino.h>
#include "AetherLink_Config.h"

namespace AetherLink {
namespace SBUSIbus {
bool ParseSbusPacket(const uint8_t* packet, size_t len, RxFrame& out_frame);
size_t BuildIbusPacket(const RxFrame& frame, uint8_t* out_packet, size_t out_len);
}  // namespace SBUSIbus
}  // namespace AetherLink

#endif
