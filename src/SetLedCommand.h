// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"
#include <sstream>
#include <NeoPixelBus.h>

class SetLedCommand : public Command {
public:
    SetLedCommand() {}
    ~SetLedCommand() {}

public:
    const char *GetName() {
        return "set_led";
    }

    int Execute(const CommandLineParser *parser) {
        if (!pixels) {
            reply("set_led: pixels is null.\n");
            Log::Error("set_led: pixels is null.");
            return 1;
        }

        String info;
        const char *parsedLedIndex = parser->GetFirstArg();
        if (parsedLedIndex == 0) {
            reply("set_led: led index not found.\n");
            Log::Error("set_led: led index not found.");
            return 1;
        }
        int ledIndex = atoi(parsedLedIndex);
        if (ledIndex < 0) {
            reply("set_led: led index region error.\n");
            Log::Error("set_led: led index region error.");
            return 1;
        }

        const char *parsedRedColor = parser->NextArg(parsedLedIndex);
        if (parsedRedColor == 0) {
            reply("set_led: red color not found.\n");
            Log::Error("set_led: red color not found.");
            return 1;
        }
        uint8_t red = atoi(parsedRedColor);

        const char *parsedGreenColor = parser->NextArg(parsedRedColor);
        if (parsedGreenColor == 0) {
            reply("set_led: green color not found.\n");
            Log::Error("set_led: green color not found.");
            return 1;
        }
        uint8_t green = atoi(parsedGreenColor);

        const char *parsedBlueColor = parser->NextArg(parsedGreenColor);
        if (parsedBlueColor == 0) {
            reply("set_led: blue color not found.\n");
            Log::Error("set_led: blue color not found.");
            return 1;
        }
        uint8_t blue = atoi(parsedBlueColor);

        std::ostringstream s;
        s << "set_led: index=" << ledIndex
          << ", red="   << red
          << ", green=" << green
          << ", blue="  << blue << "\n";
        reply(s.str().c_str());
        pixels->SetPixelColor(ledIndex, RgbColor(red, green, blue));

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
        pixels->Show();
        // taskEXIT_CRITICAL(&mux);
        return 0;
    }

    void SetPixels(NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* pixels) {
        this->pixels = pixels;
    }

private:
    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* pixels;
};
