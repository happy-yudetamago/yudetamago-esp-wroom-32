// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include <hardware_defines.h>

class LedColor {
public:
    int red;
    int green;
    int blue;

public:
    LedColor(int red, int green, int blue) : red(red), green(green), blue(blue) {}
};

#define BLACK_COLOR      LedColor(0, 0, 0)
#define ERROR_COLOR      LedColor(LED_BRIGHTNESS, 0, 0)
#define WAITING_COLOR    LedColor(LED_BRIGHTNESS, LED_BRIGHTNESS, 0)
#define CONFIG_COLOR     LedColor(LED_BRIGHTNESS, LED_BRIGHTNESS, LED_BRIGHTNESS)
#define EXISTS_COLOR     LedColor(0, 0, 0)
#define NOT_EXISTS_COLOR LedColor(LED_BRIGHTNESS, 0, 0)

class LedDevice {
public:
    static void Open();
    static void Close();
    static void SetColor(int index, const LedColor& color);
    static void SetColorMask(int ledMask, const LedColor& color);
    static void Show();
};
