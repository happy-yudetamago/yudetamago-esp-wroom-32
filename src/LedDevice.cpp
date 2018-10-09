// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include "LedDevice.h"
#include <NeoPixelBus.h>
#include "log/Log.h"

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixels(NUM_OF_NEO_PIXELS, NEO_PIXEL_PIN);

void LedDevice::Open()
{
    pixels.Begin();
    SetColorMask(OBJECT_ID_MASK, BLACK_COLOR);
}

void LedDevice::Close()
{
}

void LedDevice::SetColor(int index, const LedColor& color)
{
    RgbColor c(color.red, color.green, color.blue);
    pixels.SetPixelColor(index, c);
}

void LedDevice::SetColorMask(int ledMask, const LedColor& color)
{
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        int mask = (1 << i);
        if ((ledMask & mask) == 0) {
            continue;
        }

        RgbColor c(color.red, color.green, color.blue);
        pixels.SetPixelColor(i, c);
    }
}

void LedDevice::Show()
{
    // [Problem] Neo Pixel Green LED can not turn off
    // - Chip : ESP-32
    // - When : After WiFi connected?
    //
    // Test result
    // <command>                          <actual color>
    // setPixelColor(0,   0,   0,   0) -> Green
    // setPixelColor(0,   0, 255,   0) -> Green
    // setPixelColor(0,   0,   0, 255) -> Blue
    // setPixelColor(0, 128,   0,   0) -> Yellow (Red + Green?)
    // setPixelColor(0, 255,   0,   0) -> Orange (Red + Green?)
    // setPixelColor(0, 255, 255, 255) -> White
    //
    // WS2812B protocol(Neo Pixel)
    // [Green 8bit] [Red 8bit] [Blue 8bit]
    //
    // Guess
    // - only 1st color can not turn off
    // - only 1st color is garbled
    // - if continuously execute show(), 1st show() fails, but 2nd show() success?
    //
    // Solution
    //   pixels.show();
    //   pixels.show();

    // [Problem] Neo Pixel LED rarely change a different color.
    //
    // Solution
    //   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    //   taskENTER_CRITICAL(&mux);
    //   pixels.show();
    //   taskEXIT_CRITICAL(&mux);
    //
    // portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    // taskENTER_CRITICAL(&mux);
    // pixels->show();
    // pixels->show();
    // taskEXIT_CRITICAL(&mux);

    // Change library from Adafruit to Makuna/NeoPixelBus.
    //
    // EDIT: I just tried https://github.com/Makuna/NeoPixelBus with total success,
    // even with WiFi and Serial. Had to install from git, though,
    // due to Makuna/NeoPixelBus#212 on Linux. I can confirm that's a good workaround
    // until the Adafruit library gets hardware support on ESP32.
    //
    // https://github.com/adafruit/Adafruit_NeoPixel/issues/139
    // portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    // taskENTER_CRITICAL(&mux);
    pixels.Show();
    // taskEXIT_CRITICAL(&mux);
}
