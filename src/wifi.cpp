// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include "wifi.h"

#include <Arduino.h>

#include <sstream>

#include <WiFi.h>
#include "log/Log.h"

#include "LedDevice.h"

void connectWifi(const char *ssid, const char *pass)
{
    if (WiFi.isConnected()) {
        WiFi.disconnect();
    }
    // If forget mode(WIFI_STA), mode might be WIFI_AP_STA.
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    Log::Info("WiFi connecting...");
    Log::Info(ssid);
    Log::Info(pass);
    while (WiFi.status() != WL_CONNECTED) {
        LedDevice::SetColor(0, WAITING_COLOR);
        LedDevice::Show();
        Log::Debug(".");
        vTaskDelay(500);

        LedDevice::SetColor(0, BLACK_COLOR);
        LedDevice::Show();
        Log::Debug(".");
        vTaskDelay(500);
    }

    Log::Info("WiFi connected.");
    IPAddress ip = WiFi.localIP();

    std::ostringstream log;
    log << "IP address: " << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3];
    Log::Info(log.str().c_str());
}
