/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Protocol_ESPNOW.h"

#include <esp_now.h>
#include <esp_wifi.h>
#include <string.h>

namespace AetherLink {
namespace ESPNOWProto {

struct EspNowFrame {
  uint32_t ts_ms;
  int16_t channels[kMaxChannels];
  uint8_t rssi;
  uint8_t crc;
};

static volatile bool s_new_frame = false;
static EspNowFrame s_frame{};
static bool s_bound = false;
static uint8_t s_last_peer[6] = {0};

static uint8_t XorCrc(const uint8_t* data, size_t len) {
  uint8_t crc = 0U;
  for (size_t i = 0U; i < len; ++i) {
    crc ^= data[i];
  }
  return crc;
}

static void OnDataRecv(const esp_now_recv_info_t* info, const uint8_t* incoming, int len) {
  if ((incoming == nullptr) || (len != static_cast<int>(sizeof(EspNowFrame))) || (info == nullptr)) {
    return;
  }
  EspNowFrame local{};
  memcpy(&local, incoming, sizeof(local));
  if (XorCrc(incoming, sizeof(EspNowFrame) - 1U) != local.crc) {
    return;
  }
  memcpy(&s_frame, &local, sizeof(local));
  memcpy(s_last_peer, info->src_addr, 6U);
  s_new_frame = true;
  s_bound = true;
}

void Init() {
  (void)esp_wifi_set_channel(1U, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() == ESP_OK) {
    static const uint8_t kPmk[16] = {'A', 'e', 't', 'h', 'e', 'r', 'L', 'i', 'n', 'k', 'P', 'M', 'K', '0', '1', '!'};
    (void)esp_now_set_pmk(kPmk);
    (void)esp_now_register_recv_cb(OnDataRecv);
  }
}

bool Poll(RxFrame& out_frame) {
  if (!s_new_frame) {
    return false;
  }
  s_new_frame = false;
  out_frame.timestamp_ms = s_frame.ts_ms;
  out_frame.rssi = s_frame.rssi;
  out_frame.frame_valid = true;
  memcpy(out_frame.channels, s_frame.channels, sizeof(out_frame.channels));
  return true;
}

bool SendTelemetry(const uint8_t* data, size_t len) {
  if ((!s_bound) || (data == nullptr) || (len == 0U)) {
    return false;
  }
  return esp_now_send(s_last_peer, data, len) == ESP_OK;
}

bool IsBound() {
  return s_bound;
}

}  // namespace ESPNOWProto
}  // namespace AetherLink
