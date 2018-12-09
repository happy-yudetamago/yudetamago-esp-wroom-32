// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include "Config.h"

#include <SPIFFS.h>
#include <vector>
#include "hardware_defines.h"

const char* config_file = "/config.txt";
const char* objectids_file = "/objectids.txt";

static String wifi_ssid;
static String wifi_pass;
static String objectIds[OBJECT_ID_SIZE];

bool Config::Initialize()
{
    // see
    // https://github.com/espressif/arduino-esp32/issues/638
    return SPIFFS.begin(true); // true: formatOnFail
}

void Config::GetWifiConfig(String& ssid, String& pass)
{
    ssid = wifi_ssid;
    pass = wifi_pass;
}

void Config::SetWifiConfig(const String& ssid, const String& pass)
{
    wifi_ssid = ssid;
    wifi_pass = pass;
}

bool Config::GetObjectId(int index, String& objectId)
{
    if (index < 0 || OBJECT_ID_SIZE <= index) {
        return false;
    }
    objectId = objectIds[index];
    return true;
}

bool Config::SetObjectId(int index, const String& objectId)
{
    if (index < 0 || OBJECT_ID_SIZE <= index) {
        return false;
    }
    objectIds[index] = objectId;
    return true;
}

bool Config::Read()
{
    File fdConfig = SPIFFS.open(config_file, "r");
    if (!fdConfig) {
        return false;
    }
    String ssid = fdConfig.readStringUntil('\n');
    ssid.trim();
    String pass = fdConfig.readStringUntil('\n');
    pass.trim();
    fdConfig.close();
    SetWifiConfig(ssid, pass);

    File fdObjectIds = SPIFFS.open(objectids_file, "r");
    if (!fdObjectIds) {
        return false;
    }
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        String objectId = fdObjectIds.readStringUntil('\n');
        objectId.trim();
        SetObjectId(i, objectId);
    }
    fdObjectIds.close();
    return true;
}

bool Config::Write()
{
    File fdConfig = SPIFFS.open(config_file, "w");
    if (!fdConfig) {
        return false;
    }
    String ssid;
    String pass;
    GetWifiConfig(ssid, pass);
    fdConfig.println(ssid);
    fdConfig.println(pass);
    fdConfig.close();

    File fdObjectIds = SPIFFS.open(objectids_file, "w");
    if (!fdObjectIds) {
        return false;
    }
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        String objectId;
        GetObjectId(i, objectId);
        fdObjectIds.println(objectId);
    }
    fdObjectIds.close();
    return true;
}
