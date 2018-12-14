// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include "ota.h"

#include <sstream>
#include <HTTPClient.h>
#include <Update.h>

#include "log/Log.h"

static E_OTA_RESULT getStream(const char *uri,
                              HTTPClient *http,
                              int        *contentLength,
                              WiFiClient **client);
static E_OTA_RESULT execOTAOne(const char *firmwareUri,
                               const char *md5Uri);

const int OTA_RETRY_TIMES = 3;
const int MD5_LENGTH      = 32;
static const char *defaultFirmwareUri = "http://oedocowboys.sakura.ne.jp/yudetamago_web/gadget/firmware.bin";
static const char *defaultMd5Uri      = "http://oedocowboys.sakura.ne.jp/yudetamago_web/gadget/firmware.md5";

E_OTA_RESULT execOTA(const char *firmwareUri, const char *md5Uri)
{
    E_OTA_RESULT result;
    for (int i=0; i<OTA_RETRY_TIMES; i++) {
        result = execOTAOne(firmwareUri, md5Uri);
        if (result == OTA_OK) {
            return OTA_OK;
        }
    }
    return result;
}

static E_OTA_RESULT getStream(const char *uri,
                              HTTPClient *http,
                              int        *contentLength,
                              WiFiClient **client)
{
    if (!http->begin(uri)) {
        Log::Error("OTA: Connect failed.");
        return OTA_ERR_CONNECTION;
    }

    int httpCode = http->GET();
    if (httpCode != 200) {
        Log::Error("OTA: Not found the file.");
        return OTA_ERR_NOT_FOUND;
    }

    *contentLength = http->getSize();
    if (*contentLength == 0) {
        Log::Error("OTA: content length is zero.");
        return OTA_ERR_NO_CONTENT;
    }

    *client = http->getStreamPtr();
    return OTA_OK;
}

static E_OTA_RESULT execOTAOne(const char *firmwareUri,
                               const char *md5Uri)
{
    const char *firmware = firmwareUri;
    const char *md5      = md5Uri;
    if (firmware == 0) {
        firmware = defaultFirmwareUri;
        md5      = defaultMd5Uri;
    }
    Log::Info("OTA: connecting...");
    Log::Info(firmware);
    HTTPClient http;
    if (md5 != 0) {
        Log::Info(md5);
        int md5ContentLength  = 0;
        WiFiClient *md5Client = 0;
        Log::Info("OTA: downloading MD5...");
        E_OTA_RESULT result = getStream(
            md5,
            &http,
            &md5ContentLength,
            &md5Client);
        if (result != OTA_OK) {
            return result;
        }
        if (md5ContentLength < 32) {
            Log::Error("OTA: too short MD5.");
            return OTA_ERR_NOT_FOUND;
        }
        uint8_t md5Buffer[MD5_LENGTH + 1] = {'0'};
        md5Client->read(md5Buffer, MD5_LENGTH);
        http.end();

        if (!Update.setMD5((char*)md5Buffer)) {
            Log::Error("OTA: invalid md5 file.");
            return OTA_ERR_INVALID_MD5;
        }
    }

    int firmwareContentLength  = 0;
    WiFiClient *firmwareClient = 0;
    Log::Info("OTA: downloading firmware...");
    E_OTA_RESULT result = getStream(
        firmware,
        &http,
        &firmwareContentLength,
        &firmwareClient);
    if (result != OTA_OK) {
        return result;
    }

    if (!Update.begin(firmwareContentLength)) {
        Log::Error("OTA: no spaces.");
        return OTA_ERR_NO_SPACES;
    }

    Log::Info("OTA: writing...");
    size_t written = Update.writeStream(*firmwareClient);
    if (written != firmwareContentLength) {
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
