// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"
#include "Version.h"

class InfoCommand : public Command {
public:
    InfoCommand() {}
    ~InfoCommand() {}

public:
    const char *GetName() {
        return "info";
    }

    int Execute(const CommandLineParser *parser) {
        reply("version=");
        reply(VERSION);
        reply("\n");

        String ssid;
        String pass;
        Config::GetWifiConfig(ssid, pass);
        reply("ssid=");
        reply(ssid.c_str());
        reply("\npass=");
        reply(pass.c_str());
        reply("\n");

        for (int i=0; i<OBJECT_ID_SIZE; i++) {
            String objectId;
            Config::GetObjectId(i, objectId);
            reply("objectId=");
            reply(objectId.c_str());
            reply("\n");
        }
        return 0;
    }
};
