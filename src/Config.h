// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include <Arduino.h>

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
