// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"
#include <string>

class LogCommand : public Command {
public:
    LogCommand() {}
    ~LogCommand() {}

public:
    const char *GetName() {
        return "log";
    }

    int Execute(const CommandLineParser *parser) {
        for (int i=0; i<Log::LOG_CAPACITY; i++) {
            std::string log;
            if (!Log::GetLog(i, log)) {
                return 0;
            }
            reply(log.c_str());
            reply("\n");
        }
        return 0;
    }
};
