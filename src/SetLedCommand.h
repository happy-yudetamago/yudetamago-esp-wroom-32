// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"
#include <sstream>
#include "LedDevice.h"

class SetLedCommand : public Command {
public:
    SetLedCommand() {}
    ~SetLedCommand() {}

public:
    const char *GetName() {
        return "set_led";
    }

    int Execute(const CommandLineParser *parser) {
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

        std::stringstream s;
        s << "set_led: index=" << ledIndex
          << ", red="   << (int)red
          << ", green=" << (int)green
          << ", blue="  << (int)blue << "\n";
        reply(s.str().c_str());
        LedDevice::SetColor(ledIndex, LedColor(red, green, blue));
        LedDevice::Show();
        return 0;
    }
};
