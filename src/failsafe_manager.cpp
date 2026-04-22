/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "failsafe_manager.h"

#include <Arduino.h>

namespace AetherLink {

bool ApplyFailsafeIfNeeded(RxFrame& frame, uint32_t last_rx_ms, const RuntimeConfig& config) {
  const bool timeout = (millis() - last_rx_ms) > kFailsafeTimeoutMs;
  if (!timeout) {
    return false;
  }
  switch (config.failsafe_mode) {
    case FailsafeMode::HOLD:
      break;
    case FailsafeMode::CUT:
      frame.channels[2] = 1000;
      break;
    case FailsafeMode::CUSTOM:
      for (uint8_t i = 0U; i < kMaxChannels; ++i) {
        frame.channels[i] = config.custom_failsafe[i];
      }
      break;
    default:
      break;
  }
  frame.frame_valid = false;
  return true;
}

}  // namespace AetherLink
