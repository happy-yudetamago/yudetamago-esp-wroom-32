// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"

class SetObjectIdCommand : public Command {
public:
    SetObjectIdCommand() {}
    ~SetObjectIdCommand() {}

public:
    const char *GetName() {
        return "set_objectid";
    }

    int Execute(const CommandLineParser *parser) {
        // validate args
        const char *parsedObjectId = parser->GetFirstArg();
        for (int i=0; i<OBJECT_ID_SIZE; i++) {
            if (parsedObjectId == 0) {
                reply("set_objectid: too few arguments.\n");
                return 1;
            }
            parsedObjectId = parser->NextArg(parsedObjectId);
        }
        if (parsedObjectId != 0) {
            reply("set_objectid: too many arguments.\n");
            return 1;
        }

        // write data
        parsedObjectId = parser->GetFirstArg();
        for (int i=0; i<OBJECT_ID_SIZE; i++) {
            String objectId(parsedObjectId);
            Config::SetObjectId(i, objectId);
            parsedObjectId = parser->NextArg(parsedObjectId);
        }
        if (Config::Write()) {
            reply("Successed to write objectId.\n");
            return 0;
        } else {
            reply("Failed to write objectId.\n");
            return 1;
        }
    }
};
