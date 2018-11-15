// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.
#ifdef TARGET

#include <WiFi.h>
#include <sstream>

#include "log/Log.h"
#include "command/CommandLine.h"

#include "ota.h"
#include "Config.h"
#include "YudetamagoClient.h"
#include "hardware_defines.h"
#include "LedDevice.h"

#include "command/CommandLine.h"

#include "BluetoothStream.h"

#include "InfoCommand.h"
#include "LogCommand.h"
#include "GetButtonStateCommand.h"
#include "SetLogLevelCommand.h"
#include "SetLedCommand.h"
#include "SetObjectIdCommand.h"
#include "SetSsidCommand.h"
#include "LoopColorChangeCommand.h"
#include "ResetCommand.h"

const int NCMB_BUTTON_INTERVAL       = (100);
const int NCMB_AFTER_BUTTON_INTERVAL = (1000);
const int NCMB_ACCESS_INTERVAL       = (5 * 60 * 1000);

bool object_exists[OBJECT_ID_SIZE] = {true, true, true, true, true};

BluetoothStream        stream;

CommandLine            commandLine;
InfoCommand            infoCommand;
LogCommand             logCommand;
GetButtonStateCommand  getButtonStateCommand;
SetLogLevelCommand     setLogLevelCommand;
SetLedCommand          setLedCommand;
SetObjectIdCommand     setObjectIdCommand;
SetSsidCommand         setSsidCommand;
LoopColorChangeCommand loopColorChangeCommand;
ResetCommand           resetCommand;

static void showErrorOne(int ledMask) {
    LedDevice::SetColorMask(ledMask, ERROR_COLOR);
    LedDevice::Show();
    vTaskDelay(1000);

    LedDevice::SetColorMask(ledMask, BLACK_COLOR);
    LedDevice::Show();
    vTaskDelay(1000);
}

static void showError(int ledMask, int times) {
    for (int i=0; times < 0 || i<times; i++) {
        showErrorOne(ledMask);
    }
}

static void reconnectWifi() {
    String ssid;
    String pass;
    Config::GetWifiConfig(ssid, pass);

    // If forget mode(WIFI_STA), mode might be WIFI_AP_STA.
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Log::Info("WiFi connecting...");
    Log::Info(ssid.c_str());
    Log::Info(pass.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        LedDevice::SetColor(0, WAITING_COLOR);
        LedDevice::Show();
        Log::Debug(".");
        vTaskDelay(500);

        LedDevice::SetColor(0, BLACK_COLOR);
        LedDevice::Show();
        Log::Debug(".");
        vTaskDelay(500);
    }

    Log::Info("WiFi connected.");
    IPAddress ip = WiFi.localIP();

    std::ostringstream log;
    log << "IP address: " << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3];
    Log::Info(log.str().c_str());
}

static void showExistStates() {
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        bool exists = object_exists[i];
        LedDevice::SetColor(i, exists? EXISTS_COLOR: NOT_EXSITS_COLOR);
    }
    LedDevice::Show();
}

static void downloadExistStates() {
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        String object_id;
        Config::GetObjectId(i, object_id);

        String error;
        bool exists;
        if (!YudetamagoClient::GetExistance(object_id.c_str(), exists, error) ) {
            Log::Error(error.c_str());
            showError(1<<i, 10);
            return;
        }
        if (exists) {
            std::ostringstream msg;
            msg << "GetExistance: exist #";
            msg << i;
            Log::Debug(msg.str().c_str());
        } else {
            std::ostringstream msg;
            msg << "GetExistance: not exist #";
            msg << i;
            Log::Debug(msg.str().c_str());
        }
        object_exists[i] = exists;
    }
}

static void uploadExistState(int index) {
    String object_id;
    Config::GetObjectId(index, object_id);

    bool exists = object_exists[index];
    String error;
    if (!YudetamagoClient::SetExistance(object_id.c_str(), exists, error)) {
        Log::Error(error.c_str());
        showError(1<<index, 10);
        return;
    }
    if (exists) {
        std::ostringstream msg;
        msg << "SetExistance: #";
        msg << index;
        msg << " exist";
        Log::Info(msg.str().c_str());
    } else {
        std::ostringstream msg;
        msg << "SetExistance: #";
        msg << index;
        msg << " not exist";
        Log::Info(msg.str().c_str());
    }
}

static void toggleExistState(int index) {
    object_exists[index] = !object_exists[index];
    bool exists = object_exists[index];

    if (exists) {
        std::ostringstream msg;
        msg << "Toggle exist state: not exist -> exist #";
        msg << index;
        Log::Debug(msg.str().c_str());
    } else {
        std::ostringstream msg;
        msg << "Toggle exist state: exist -> not exist #";
        msg << index;
        Log::Debug(msg.str().c_str());
    }
}

static void initializeCommandLine(Stream *stream)
{
    commandLine.Initialize(stream);

    infoCommand.Initialize(stream);
    commandLine.AddCommand(&infoCommand);

    logCommand.Initialize(stream);
    commandLine.AddCommand(&logCommand);

    getButtonStateCommand.Initialize(stream);
    commandLine.AddCommand(&getButtonStateCommand);

    setLogLevelCommand.Initialize(stream);
    commandLine.AddCommand(&setLogLevelCommand);

    setLedCommand.Initialize(stream);
    commandLine.AddCommand(&setLedCommand);

    setObjectIdCommand.Initialize(stream);
    commandLine.AddCommand(&setObjectIdCommand);

    setSsidCommand.Initialize(stream);
    commandLine.AddCommand(&setSsidCommand);

    loopColorChangeCommand.Initialize(stream);
    commandLine.AddCommand(&loopColorChangeCommand);

    resetCommand.Initialize(stream);
    commandLine.AddCommand(&resetCommand);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Yudetamago...");

    LedDevice::Open();

    pinMode(CONFIG_MODE_PIN, INPUT_PULLUP);
    pinMode(OTA_MODE_PIN,    INPUT_PULLUP);
    pinMode(STOCK_0_PIN,     INPUT_PULLUP);
    pinMode(STOCK_1_PIN,     INPUT_PULLUP);
    pinMode(STOCK_2_PIN,     INPUT_PULLUP);
    pinMode(STOCK_3_PIN,     INPUT_PULLUP);
    pinMode(STOCK_4_PIN,     INPUT_PULLUP);

    if (!Config::Initialize()) {
        Log::Error("Faild to Config::Initialize().");
        while (1) {
            showError(OBJECT_ID_MASK, 1);
        }
    }

    if (digitalRead(CONFIG_MODE_PIN) == LOW) {
        Log::Info("Detected Config mode.");

        Config::Read();
        LedDevice::SetColor(0, CONFIG_COLOR);
        LedDevice::Show();

        stream.Initialize();
        initializeCommandLine(&stream);
        while (1) {
            commandLine.Analyze();
            stream.Update();
            vTaskDelay(20);
        }
        // can not reach here.
    }

    initializeCommandLine(0);
    Log::Info("Detected Normal mode.");
    if (!Config::Read()) {
        Log::Error("Faild to read objectId.");
        while (1) {
            showError(OBJECT_ID_MASK, 1);
            while (commandLine.AnalyzeSerial());
        }
    }
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        String object_id;
        Config::GetObjectId(i, object_id);
        String log = "objectId : ";
        log += object_id;
        Log::Info(log.c_str());
    }
    reconnectWifi();

    if (digitalRead(OTA_MODE_PIN) == LOW) {
        Log::Info("Detected OTA mode.");
        LedDevice::SetColor(1, CONFIG_COLOR);
        LedDevice::Show();
        for (int i=0; i<3; i++) {
            execOTA();
        }
        showError(OBJECT_ID_MASK, -1);
    }

    downloadExistStates();
    showExistStates();
    Serial.println("Started Yudetamago.");
}

void loop() {
    for (int times=0; times<NCMB_ACCESS_INTERVAL; times+=NCMB_BUTTON_INTERVAL) {
        while (commandLine.AnalyzeSerial());

        int pressedButtonIndex = -1;
        if (digitalRead(STOCK_0_PIN) == LOW) {
            pressedButtonIndex = 0;
        } else if (digitalRead(STOCK_1_PIN) == LOW) {
            pressedButtonIndex = 1;
        } else if (digitalRead(STOCK_2_PIN) == LOW) {
            pressedButtonIndex = 2;
        } else if (digitalRead(STOCK_3_PIN) == LOW) {
            pressedButtonIndex = 3;
        } else if (digitalRead(STOCK_4_PIN) == LOW) {
            pressedButtonIndex = 4;
        } else {
            vTaskDelay(NCMB_BUTTON_INTERVAL);
            continue;
        }
        toggleExistState(pressedButtonIndex);
        showExistStates();
        uploadExistState(pressedButtonIndex);
        downloadExistStates();
        showExistStates();

        /////////////////////////////////////////////////////////////
        // Use vTaskDelay() instead of delay()                     //
        // https://github.com/espressif/arduino-esp32/issues/595   //
        //                                                         //
        // Trouble log when using delay()                          //
        // - IDLE (CPU 0)                                          //
        // Tasks currently running:                                //
        // CPU 0: wifi                                             //
        // CPU 1: IDLE                                             //
        // Task watchdog got triggered.                            //
        // The following tasks did not reset the watchdog in time: //
        // - IDLE (CPU 0)                                          //
        // Tasks currently running:                                //
        // CPU 0: wifi                                             //
        // CPU 1: IDLE                                             //
        // ...                                                     //
        /////////////////////////////////////////////////////////////
        vTaskDelay(NCMB_AFTER_BUTTON_INTERVAL);
    }

    downloadExistStates();
    showExistStates();
}

#endif // TARGET
