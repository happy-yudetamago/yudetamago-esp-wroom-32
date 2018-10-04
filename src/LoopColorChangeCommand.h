// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"
#include <sstream>

class LoopColorChangeCommand : public Command {
public:
    LoopColorChangeCommand() {}
    ~LoopColorChangeCommand() {}

public:
    const char *GetName() {
        return "loop_color_change";
    }

    int Execute(const CommandLineParser *parser) {
        const int internalLoop = 10;
        const char *times = parser->GetFirstArg();
        int timesOfLoop = 0;
        if (times) {
            timesOfLoop = atoi(times);
        }
        for (int i=0; timesOfLoop<=0 || i<timesOfLoop; i++) {
            Log::Trace("loop_color_change: next loop");
            const LedColor colors[] = {
                LedColor(0,              0,              0),
                LedColor(LED_BRIGHTNESS, 0,              0),
                LedColor(0,              LED_BRIGHTNESS, 0),
                LedColor(0,              0,              LED_BRIGHTNESS),
                LedColor(LED_BRIGHTNESS, LED_BRIGHTNESS, 0),
                LedColor(LED_BRIGHTNESS, 0,              LED_BRIGHTNESS),
                LedColor(0,              LED_BRIGHTNESS, LED_BRIGHTNESS),
                LedColor(LED_BRIGHTNESS, LED_BRIGHTNESS, LED_BRIGHTNESS)};
            const int colorsSize = sizeof(colors)/sizeof(colors[0]);
            for (int c=0; c<colorsSize; c++) {
                LedColor color = colors[c];
                for (int j=0; j<internalLoop; j++) {
                    LedDevice::SetColorMask(OBJECT_ID_MASK, color);
                    LedDevice::Show();
                    vTaskDelay(100);
                }
            }
        }

        return 0;
    }
};
