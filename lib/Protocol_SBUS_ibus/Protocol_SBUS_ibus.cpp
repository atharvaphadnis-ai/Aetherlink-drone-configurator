/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Protocol_SBUS_ibus.h"

namespace AetherLink {
namespace SBUSIbus {

bool ParseSbusPacket(const uint8_t* packet, size_t len, RxFrame& out_frame) {
  if ((packet == nullptr) || (len != 25U)) {
    return false;
  }
  if ((packet[0] != 0x0FU) || (packet[24] != 0x00U && packet[24] != 0x04U)) {
    return false;
  }

  out_frame.channels[0] = static_cast<int16_t>((packet[1] | packet[2] << 8) & 0x07FFU);
  out_frame.channels[1] = static_cast<int16_t>((packet[2] >> 3 | packet[3] << 5) & 0x07FFU);
  out_frame.channels[2] = static_cast<int16_t>((packet[3] >> 6 | packet[4] << 2 | packet[5] << 10) & 0x07FFU);
  out_frame.channels[3] = static_cast<int16_t>((packet[5] >> 1 | packet[6] << 7) & 0x07FFU);
  out_frame.channels[4] = static_cast<int16_t>((packet[6] >> 4 | packet[7] << 4) & 0x07FFU);
  out_frame.channels[5] = static_cast<int16_t>((packet[7] >> 7 | packet[8] << 1 | packet[9] << 9) & 0x07FFU);
  out_frame.channels[6] = static_cast<int16_t>((packet[9] >> 2 | packet[10] << 6) & 0x07FFU);
  out_frame.channels[7] = static_cast<int16_t>((packet[10] >> 5 | packet[11] << 3) & 0x07FFU);
  out_frame.channels[8] = static_cast<int16_t>((packet[12] | packet[13] << 8) & 0x07FFU);
  out_frame.channels[9] = static_cast<int16_t>((packet[13] >> 3 | packet[14] << 5) & 0x07FFU);
  out_frame.channels[10] = static_cast<int16_t>((packet[14] >> 6 | packet[15] << 2 | packet[16] << 10) & 0x07FFU);
  out_frame.channels[11] = static_cast<int16_t>((packet[16] >> 1 | packet[17] << 7) & 0x07FFU);
  out_frame.channels[12] = static_cast<int16_t>((packet[17] >> 4 | packet[18] << 4) & 0x07FFU);
  out_frame.channels[13] = static_cast<int16_t>((packet[18] >> 7 | packet[19] << 1 | packet[20] << 9) & 0x07FFU);
  out_frame.channels[14] = static_cast<int16_t>((packet[20] >> 2 | packet[21] << 6) & 0x07FFU);
  out_frame.channels[15] = static_cast<int16_t>((packet[21] >> 5 | packet[22] << 3) & 0x07FFU);

  for (uint8_t i = 0U; i < kMaxChannels; ++i) {
    out_frame.channels[i] = map(out_frame.channels[i], 172, 1811, 1000, 2000);
  }
  out_frame.timestamp_ms = millis();
  out_frame.rssi = 80U;
  out_frame.frame_valid = (packet[23] & 0x08U) == 0U;
  return out_frame.frame_valid;
}

size_t BuildIbusPacket(const RxFrame& frame, uint8_t* out_packet, size_t out_len) {
  constexpr uint8_t kLen = 32U;
  if ((out_packet == nullptr) || (out_len < kLen)) {
    return 0U;
  }
  out_packet[0] = kLen;
  out_packet[1] = 0x40;
  for (uint8_t i = 0U; i < 14U; ++i) {
    const uint16_t val = static_cast<uint16_t>(constrain(frame.channels[i], 1000, 2000));
    out_packet[2U + i * 2U] = static_cast<uint8_t>(val & 0xFFU);
    out_packet[3U + i * 2U] = static_cast<uint8_t>((val >> 8U) & 0xFFU);
  }

  uint16_t checksum = 0xFFFFU;
  for (uint8_t i = 0U; i < 30U; ++i) {
    checksum = static_cast<uint16_t>(checksum - out_packet[i]);
  }
  out_packet[30] = static_cast<uint8_t>(checksum & 0xFFU);
  out_packet[31] = static_cast<uint8_t>((checksum >> 8U) & 0xFFU);
  return kLen;
}

}  // namespace SBUSIbus
}  // namespace AetherLink
