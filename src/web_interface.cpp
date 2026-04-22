/*
 * AetherLink Firmware
 * Copyright (C) 2026 Atharva Phadnis
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "web_interface.h"

#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFi.h>

namespace AetherLink {

static WebServer s_server(80U);
static RuntimeConfig* s_cfg = nullptr;

static void HandleStatus() {
  if (s_cfg == nullptr) {
    s_server.send(500, "application/json", "{\"error\":\"config null\"}");
    return;
  }
  StaticJsonDocument<256U> doc;
  doc["input"] = static_cast<uint8_t>(s_cfg->input_lock);
  doc["output"] = static_cast<uint8_t>(s_cfg->output_mode);
  doc["failsafe"] = static_cast<uint8_t>(s_cfg->failsafe_mode);
  doc["bridge"] = s_cfg->bridge_mode_enabled;
  String out;
  serializeJson(doc, out);
  s_server.send(200, "application/json", out);
}

static void HandleBridgeToggle() {
  if (s_cfg == nullptr) {
    s_server.send(500, "text/plain", "config null");
    return;
  }
  s_cfg->bridge_mode_enabled = !s_cfg->bridge_mode_enabled;
  s_server.send(200, "text/plain", s_cfg->bridge_mode_enabled ? "bridge on" : "bridge off");
}

void WebInterfaceInit(RuntimeConfig* config) {
  s_cfg = config;
  const char* ssid = "AetherLink-Setup";
  const char* pass = "AetherLink123";
  WiFi.softAP(ssid, pass);
  s_server.on("/status", HTTP_GET, HandleStatus);
  s_server.on("/bridge/toggle", HTTP_POST, HandleBridgeToggle);
  s_server.begin();
}

void WebInterfacePoll() {
  s_server.handleClient();
}

}  // namespace AetherLink
