/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AETHERLINK_PROTOCOL_NRF24_H
#define AETHERLINK_PROTOCOL_NRF24_H

#include <Arduino.h>
#include "AetherLink_Config.h"

namespace AetherLink {
namespace NRF24Proto {
void Init(uint8_t ce_pin, uint8_t csn_pin);
bool Poll(RxFrame& out_frame);
bool IsLinked();
}  // namespace NRF24Proto
}  // namespace AetherLink

#endif
