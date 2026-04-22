/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Protocol_ELRS.h"

#include <CRC8.h>

namespace AetherLink {
namespace ELRS {

static HardwareSerial* s_uart = nullptr;
static bool s_link = false;
static uint32_t s_last_rx_ms = 0U;
static CRC8 s_crc;

void Init(HardwareSerial& serial_port) {
  s_uart = &serial_port;
  s_uart->begin(420000U, SERIAL_8N1);
  s_crc.restart();
}

bool Poll(RxFrame& out_frame) {
  if (s_uart == nullptr) {
    return false;
  }
  constexpr uint8_t kFrameSize = 26U;
  if (s_uart->available() < static_cast<int>(kFrameSize)) {
    return false;
  }
  uint8_t buf[kFrameSize] = {0};
  const size_t read_bytes = s_uart->readBytes(buf, kFrameSize);
  if (read_bytes != kFrameSize) {
    return false;
  }
  s_crc.restart();
  for (uint8_t i = 0U; i < (kFrameSize - 1U); ++i) {
    s_crc.add(buf[i]);
  }
  if (s_crc.calc() != buf[kFrameSize - 1U]) {
    return false;
  }

  out_frame.timestamp_ms = millis();
  out_frame.rssi = buf[2];
  out_frame.frame_valid = true;
  for (uint8_t ch = 0U; ch < kMaxChannels; ++ch) {
    const uint8_t lo = static_cast<uint8_t>(3U + ch);
    out_frame.channels[ch] = static_cast<int16_t>(1000 + (buf[lo] * 4));
  }
  s_last_rx_ms = out_frame.timestamp_ms;
  s_link = true;
  return true;
}

void SendTelemetry(uint8_t rssi, float battery_v) {
  if (s_uart == nullptr) {
    return;
  }
  uint8_t pkt[6] = {0xC8, 0x04, rssi, static_cast<uint8_t>(battery_v * 10.0F), 0U, 0U};
  s_crc.restart();
  for (uint8_t i = 0U; i < 5U; ++i) {
    s_crc.add(pkt[i]);
  }
  pkt[5] = s_crc.calc();
  (void)s_uart->write(pkt, sizeof(pkt));
}

bool IsLinked() {
  s_link = (millis() - s_last_rx_ms) < 300U;
  return s_link;
}

}  // namespace ELRS
}  // namespace AetherLink
