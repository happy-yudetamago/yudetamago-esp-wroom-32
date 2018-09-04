// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"
#include <sstream>
#include <NeoPixelBus.h>

class LoopColorChangeCommand : public Command {
public:
    LoopColorChangeCommand() {}
    ~LoopColorChangeCommand() {}

public:
    const char *GetName() {
        return "loop_color_change";
    }

    int Execute(const CommandLineParser *parser) {
        if (!pixels) {
            reply("loop_color_change: pixels is null.\n");
            Log::Error("loop_color_change: pixels is null.");
            return 1;
        }

        const char *times = parser->GetFirstArg();
        int timesOfLoop = 0;
        if (times) {
            timesOfLoop = atoi(times);
        }
        for (int i=0; timesOfLoop<=0 || i<timesOfLoop; i++) {
            const RgbColor colors[] = {
                RgbColor(0,              0,              0),
                RgbColor(LED_BRIGHTNESS, 0,              0),
                RgbColor(0,              LED_BRIGHTNESS, 0),
                RgbColor(0,              0,              LED_BRIGHTNESS),
                RgbColor(LED_BRIGHTNESS, LED_BRIGHTNESS, 0),
                RgbColor(LED_BRIGHTNESS, 0,              LED_BRIGHTNESS),
                RgbColor(0,              LED_BRIGHTNESS, LED_BRIGHTNESS),
                RgbColor(LED_BRIGHTNESS, LED_BRIGHTNESS, LED_BRIGHTNESS)};
            const int colorsSize = sizeof(colors)/sizeof(colors[0]);
            while (1) {
                for (int c=0; c<colorsSize; c++) {
                    RgbColor color = colors[0];
                    for (int i=0; i<OBJECT_ID_SIZE; i++) {
                        pixels->SetPixelColor(i, color);
                    }
                    // portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
                    // taskENTER_CRITICAL(&mux);
                    pixels->Show();
                    // taskEXIT_CRITICAL(&mux);
                    vTaskDelay(10);
                }
            }
        }

        return 0;
    }

    void SetPixels(NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* pixels) {
        this->pixels = pixels;
    }

private:
    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* pixels;
};
