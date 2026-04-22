/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Protocol_NRF24.h"

#include <RF24.h>
#include <SPI.h>

namespace AetherLink {
namespace NRF24Proto {

struct NrfPayload {
  uint16_t channels[kMaxChannels];
  uint8_t flags;
  uint8_t crc;
};

static RF24* s_radio = nullptr;
static bool s_link = false;
static uint32_t s_last_ms = 0U;
static const uint8_t kAddress[6] = "AETHR";

static uint8_t CalcCrc(const NrfPayload& p) {
  const uint8_t* raw = reinterpret_cast<const uint8_t*>(&p);
  uint8_t crc = 0U;
  for (size_t i = 0U; i < (sizeof(NrfPayload) - 1U); ++i) {
    crc ^= raw[i];
  }
  return crc;
}

void Init(uint8_t ce_pin, uint8_t csn_pin) {
  static RF24 radio(ce_pin, csn_pin);
  s_radio = &radio;
  if (!s_radio->begin()) {
    return;
  }
  s_radio->setDataRate(RF24_1MBPS);
  s_radio->setPALevel(RF24_PA_LOW);
  s_radio->setRetries(3U, 5U);
  s_radio->openReadingPipe(1U, kAddress);
  s_radio->startListening();
}

bool Poll(RxFrame& out_frame) {
  if (s_radio == nullptr || !s_radio->available()) {
    return false;
  }
  NrfPayload payload{};
  s_radio->read(&payload, sizeof(payload));
  if (CalcCrc(payload) != payload.crc) {
    return false;
  }
  for (uint8_t i = 0U; i < kMaxChannels; ++i) {
    out_frame.channels[i] = static_cast<int16_t>(constrain(payload.channels[i], 1000, 2000));
  }
  out_frame.timestamp_ms = millis();
  out_frame.rssi = 70U;
  out_frame.frame_valid = true;
  s_last_ms = out_frame.timestamp_ms;
  s_link = true;
  return true;
}

bool IsLinked() {
  s_link = (millis() - s_last_ms) < 200U;
  return s_link;
}

}  // namespace NRF24Proto
}  // namespace AetherLink
