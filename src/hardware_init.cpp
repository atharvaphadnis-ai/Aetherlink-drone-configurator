/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "hardware_init.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <esp_system.h>

#include "AetherLink_Config.h"

namespace AetherLink {

RuntimeConfig g_config = {
    InputProtocol::NONE,
    OutputProtocol::PWM_ONLY,
    FailsafeMode::HOLD,
    {1500, 1500, 1000, 1500, 1500, 1500, 1500, 1500, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000},
    {1, 2, 41, 42, 45, 46, 47, 48, 9, 10, 11, 12, 13, 14, 21, 38},
    {true, true, true, true, true, true, true, true, false, false, false, false, false, false, false, false},
    4,
    11.0F,
    false};

void InitHardware() {
  Serial.begin(kCliBaud);
  delay(20);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  (void)esp_task_wdt_init(6U, true);
  (void)esp_task_wdt_add(nullptr);
}

}  // namespace AetherLink
