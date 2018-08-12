// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"

#include "hardware_defines.h"

class GetButtonStateCommand : public Command {
public:
    GetButtonStateCommand() {
    }
    ~GetButtonStateCommand() {}

public:
    const char *GetName() {
        return "get_button_state";
    }

    int Execute(const CommandLineParser *parser) {
        const char *arg = parser->GetFirstArg();
        if (arg != 0) {
            reply("get_button_state: too many arguments.\n");
            Log::Error("get_button_state: too many arguments.");
            return 1;
        }

        writeButtonState("button #0", STOCK_0_PIN);
        writeButtonState("button #1", STOCK_1_PIN);
        writeButtonState("button #2", STOCK_2_PIN);
        writeButtonState("button #3", STOCK_3_PIN);
        writeButtonState("button #4", STOCK_4_PIN);
        return 0;
    }

private:
    void writeButtonState(const char *name, int pin) {
        String message = name;
        if (digitalRead(pin) == LOW) {
            message += ": pressed\n";
        } else {
            message += ": not pressed\n";
        }
        reply(message.c_str());
    }
};
