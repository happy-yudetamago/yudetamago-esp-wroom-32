// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#pragma once

#include "command/Command.h"
#include <sstream>
#include "ota.h"

class OtaCommand : public Command {
public:
    OtaCommand() {}
    ~OtaCommand() {}

public:
    const char *GetName() {
        return "ota";
    }

    int Execute(const CommandLineParser *parser) {
        String ssid;
        String pass;
        Config::GetWifiConfig(ssid, pass);
        connectWifi(ssid.c_str(), pass.c_str());

        const char *firmware = parser->GetFirstArg();
        const char *md5      = parser->NextArg(firmware);
        E_OTA_RESULT result  = execOTA(firmware, md5);

        std::stringstream s;
        s << "ota: ";
        if (firmware == 0) {
            s << "firmware=0";
        } else {
            s << "firmware=" << firmware;
        }
        if (md5 == 0) {
            s << ", md5=0, result=";
        } else {
            s << ", md5=" << md5 << ", result=";
        }
        s << (int)result << "\n";
        reply(s.str().c_str());
        return result;
    }
};
