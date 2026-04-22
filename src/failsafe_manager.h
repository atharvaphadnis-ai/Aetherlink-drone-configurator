/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AETHERLINK_FAILSAFE_MANAGER_H
#define AETHERLINK_FAILSAFE_MANAGER_H

#include "AetherLink_Config.h"

namespace AetherLink {
bool ApplyFailsafeIfNeeded(RxFrame& frame, uint32_t last_rx_ms, const RuntimeConfig& config);
}

#endif
