// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"

class SetSsidCommand : public Command {
public:
    SetSsidCommand() {}
    ~SetSsidCommand() {}

public:
    const char *GetName() {
        return "set_ssid";
    }

    int Execute(const CommandLineParser *parser) {
        const char *parsedSsid = parser->GetFirstArg();
        if (parsedSsid == 0) {
            reply("set_ssid: SSID not found.\n");
            return 1;
        }

        const char *parsedPass = parser->NextArg(parsedSsid);
        if (parsedPass == 0) {
            reply("set_ssid: password not found.\n");
            return 1;
        }

        if (parser->NextArg(parsedPass) != 0) {
            reply("set_ssid: too many arguments.\n");
            return 1;
        }

        String ssid(parsedSsid);
        String pass(parsedPass);
        Config::SetWifiConfig(ssid, pass);
        if (Config::Write()) {
            reply("Successed to write wifi config.\n");
            return 0;
        } else {
            reply("Failed to write wifi config.\n");
            return 1;
        }
    }
};
