// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include <Arduino.h>

class YudetamagoClient
{
public:
    static bool GetExistance(const char *objectId, bool& exists, String& error);
    static bool SetExistance(const char *objectId, bool exists, String& error);
};
