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

        const char *uri = parser->GetFirstArg();
        E_OTA_RESULT result = execOTA(uri);

        std::stringstream s;
        s << "ota: ";
        if (uri == 0) {
            s << "uri=0, result=";
        } else {
            s << "uri=" << uri << " result=";
        }
        s << (int)result << "\n";
        reply(s.str().c_str());
        return 0;
    }
};
