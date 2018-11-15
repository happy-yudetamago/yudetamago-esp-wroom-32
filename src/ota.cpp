// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include "ota.h"

#include <sstream>
#include <HTTPClient.h>
#include <Update.h>

#include "log/Log.h"

static const char *firmwareUri = "http://oedocowboys.sakura.ne.jp/yudetamago_web/gadget/firmware.bin";

E_OTA_RESULT execOTA()
{
    Log::Info("OTA: connecting...");
    Log::Info(firmwareUri);
    HTTPClient http;
    if (!http.begin(firmwareUri)) {
        Log::Error("OTA: Connect failed.");
        return OTA_ERR_CONNECTION;
    }

    int httpCode = http.GET();
    if (httpCode != 200) {
        Log::Error("OTA: Not found the file.");
        return OTA_ERR_NOT_FOUND;
    }

    int contentLength = http.getSize();
    if (contentLength == 0) {
        Log::Error("OTA: content length is zero.");
        return OTA_ERR_NO_CONTENT;
    }

    if (!Update.begin(contentLength)) {
        Log::Error("OTA: no spaces.");
        return OTA_ERR_NO_SPACES;
    }

    Log::Info("OTA: writing...");
    WiFiClient &client = http.getStream();
    size_t written = Update.writeStream(client);
    if (written != contentLength) {
        Log::Error("OTA: fails to write.");
        return OTA_ERR_WRITE;
    }

    if (!Update.end()) {
        Log::Error("OTA: Update not ended.");
        return OTA_ERR_OTHER;
    }

    Log::Info("OTA: done!");

    if (!Update.isFinished()) {
        Log::Error("OTA: Update not finished.");
        return OTA_ERR_OTHER;
    }

    Log::Info("OTA: rebooting...");
    ESP.restart();
    return OTA_OK;
}
