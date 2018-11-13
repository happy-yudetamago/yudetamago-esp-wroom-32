// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include "ota.h"

#include <sstream>
#include <HTTPClient.h>
#include <Update.h>

#include "log/Log.h"

static String    host = "oedocowboys.sakura.ne.jp"; // Host => bucket-name.s3.region.amazonaws.com
static const int port = 80; // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
static String    bin  = "/yudetamago_web/gadget/firmware.bin"; // bin file name with a slash in front.


E_OTA_RESULT execOTA()
{
    Log::Info("OTA: connecting...");
    std::ostringstream log;
    log << "  " << host.c_str() << ":" << port << bin;
    Log::Info(log.str().c_str());

    HTTPClient http;
    if (!http.begin(host.c_str(), port, bin)) {
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
    String contentType = http.header("Content-Type");
    if (contentType != "application/octet-stream") {
        Log::Error("OTA: no octet-stream.");
        return OTA_ERR_NO_OCTET_STREAM;
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
