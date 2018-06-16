#pragma once

#include <Arduino.h>

#define OBJECT_ID_SIZE 5

class Config
{
public:
    static bool Initialize();
    static void GetWifiConfig(String& ssid, String& pass);
    static void SetWifiConfig(const String& ssid, const String& pass);
    static bool GetObjectId(int index, String& objectId);
    static bool SetObjectId(int index, const String& objectId);
    static bool Read();
    static bool Write();
};
