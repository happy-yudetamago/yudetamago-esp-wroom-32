// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"

class SetLogLevelCommand : public Command {
public:
    SetLogLevelCommand() {}
    ~SetLogLevelCommand() {}

public:
    const char *GetName() {
        return "set_loglevel";
    }

    int Execute(const CommandLineParser *parser) {
        const char *level = parser->GetFirstArg();
        if (level == 0) {
            reply("loglevel: no log level\n");
            Log::Error("loglevel: no log level");
            return 1;
        }
        if (strcmp(level, "TRACE") == 0) {
            reply("loglevel: set TRACE\n");
            Log::SetLevel(Log::LOG_LEVEL_TRACE);
            return 0;
        }
        if (strcmp(level, "DEBUG") == 0) {
            reply("loglevel: set DEBUG\n");
            Log::SetLevel(Log::LOG_LEVEL_DEBUG);
            return 0;
        }
        if (strcmp(level, "INFO") == 0) {
            reply("loglevel: set INFO\n");
            Log::SetLevel(Log::LOG_LEVEL_INFO);
            return 0;
        }
        if (strcmp(level, "WARN") == 0) {
            reply("loglevel: set WARN\n");
            Log::SetLevel(Log::LOG_LEVEL_WARN);
            return 0;
        }
        if (strcmp(level, "ERROR") == 0) {
            reply("loglevel: set ERROR\n");
            Log::SetLevel(Log::LOG_LEVEL_ERROR);
            return 0;
        }
        if (strcmp(level, "FATAL") == 0) {
            reply("loglevel: set FATAL\n");
            Log::SetLevel(Log::LOG_LEVEL_FATAL);
            return 0;
        }
        reply("loglevel: unknown level\n");
        Log::Error("loglevel: unknown level");
        return 1;
    }
};
