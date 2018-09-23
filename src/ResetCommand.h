// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"

class ResetCommand : public Command {
public:
    ResetCommand() {}
    ~ResetCommand() {}

public:
    const char *GetName() {
        return "reset";
    }

    int Execute(const CommandLineParser *parser) {
        reply("reset...\n");
        ESP.restart();
        return 0;
    }
};
