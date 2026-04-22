/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AETHERLINK_WEB_INTERFACE_H
#define AETHERLINK_WEB_INTERFACE_H

#include "AetherLink_Config.h"

namespace AetherLink {
void WebInterfaceInit(RuntimeConfig* config);
void WebInterfacePoll();
}

#endif
