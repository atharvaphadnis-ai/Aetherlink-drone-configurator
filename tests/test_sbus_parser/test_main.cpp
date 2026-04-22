/*
 * AetherLink Firmware unit tests
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <Arduino.h>
#include <unity.h>

#include "Protocol_SBUS_ibus.h"

using namespace AetherLink;

void test_sbus_parser_accepts_valid_frame() {
  uint8_t sbus[25] = {0};
  sbus[0] = 0x0F;
  sbus[1] = 0xFF;
  sbus[2] = 0x03;
  sbus[3] = 0x20;
  sbus[4] = 0x00;
  sbus[5] = 0x40;
  sbus[23] = 0x00;
  sbus[24] = 0x00;

  RxFrame frame{};
  const bool ok = SBUSIbus::ParseSbusPacket(sbus, sizeof(sbus), frame);
  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_TRUE(frame.channels[0] >= 1000 && frame.channels[0] <= 2000);
}

void test_sbus_parser_rejects_bad_header() {
  uint8_t sbus[25] = {0};
  sbus[0] = 0x00;
  sbus[24] = 0x00;
  RxFrame frame{};
  TEST_ASSERT_FALSE(SBUSIbus::ParseSbusPacket(sbus, sizeof(sbus), frame));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_sbus_parser_accepts_valid_frame);
  RUN_TEST(test_sbus_parser_rejects_bad_header);
  UNITY_END();
}

void loop() {}
