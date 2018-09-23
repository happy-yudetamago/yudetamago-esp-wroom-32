// Copyright 2018 Yudetamago. All rights reserved.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.
#ifdef TARGET

#include <NeoPixelBus.h>
#include <WiFi.h>
#include <sstream>

#include "log/Log.h"
#include "command/CommandLine.h"

#include "Config.h"
#include "YudetamagoClient.h"
#include "hardware_defines.h"

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

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixels(NUM_OF_NEO_PIXELS, NEO_PIXEL_PIN);

RgbColor BLACK_COLOR(0, 0, 0);
RgbColor ERROR_COLOR(LED_BRIGHTNESS, 0, 0);
RgbColor WAITING_COLOR(LED_BRIGHTNESS, LED_BRIGHTNESS, 0);
RgbColor CONFIG_COLOR(LED_BRIGHTNESS, LED_BRIGHTNESS, LED_BRIGHTNESS);
RgbColor EXISTS_COLOR(0, 0, 0);
RgbColor NOT_EXSITS_COLOR(LED_BRIGHTNESS, 0, 0);
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

static void showNeoPixel() {
    // [Problem] Neo Pixel Green LED can not turn off
    // - Chip : ESP-32
    // - When : After WiFi connected?
    //
    // Test result
    // <command>                          <actual color>
    // setPixelColor(0,   0,   0,   0) -> Green
    // setPixelColor(0,   0, 255,   0) -> Green
    // setPixelColor(0,   0,   0, 255) -> Blue
    // setPixelColor(0, 128,   0,   0) -> Yellow (Red + Green?)
    // setPixelColor(0, 255,   0,   0) -> Orange (Red + Green?)
    // setPixelColor(0, 255, 255, 255) -> White
    //
    // WS2812B protocol(Neo Pixel)
    // [Green 8bit] [Red 8bit] [Blue 8bit]
    //
    // Guess
    // - only 1st color can not turn off
    // - only 1st color is garbled
    // - if continuously execute show(), 1st show() fails, but 2nd show() success?
    //
    // Solution
    //   pixels.show();
    //   pixels.show();

    // [Problem] Neo Pixel LED rarely change a different color.
    //
    // Solution
    //   portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    //   taskENTER_CRITICAL(&mux);
    //   pixels.show();
    //   taskEXIT_CRITICAL(&mux);
    //
    // portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    // taskENTER_CRITICAL(&mux);
    // pixels->show();
    // pixels->show();
    // taskEXIT_CRITICAL(&mux);

    // Change library from Adafruit to Makuna/NeoPixelBus.
    //
    // EDIT: I just tried https://github.com/Makuna/NeoPixelBus with total success,
    // even with WiFi and Serial. Had to install from git, though,
    // due to Makuna/NeoPixelBus#212 on Linux. I can confirm that's a good workaround
    // until the Adafruit library gets hardware support on ESP32.
    //
    // https://github.com/adafruit/Adafruit_NeoPixel/issues/139
    // portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    // taskENTER_CRITICAL(&mux);
    pixels.Show();
    // taskEXIT_CRITICAL(&mux);
}

static void showError(int times) {
    for (int i=0; times < 0 || i<times; i++) {
        pixels.SetPixelColor(NEO_PIXEL_STOCK_0, ERROR_COLOR);
        showNeoPixel();
        vTaskDelay(1000);

        pixels.SetPixelColor(NEO_PIXEL_STOCK_0, BLACK_COLOR);
        showNeoPixel();
        vTaskDelay(1000);
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
        pixels.SetPixelColor(NEO_PIXEL_STOCK_0, WAITING_COLOR);
        showNeoPixel();
        Log::Debug(".");
        vTaskDelay(500);

        pixels.SetPixelColor(NEO_PIXEL_STOCK_0, BLACK_COLOR);
        showNeoPixel();
        Log::Debug(".");
        vTaskDelay(500);
    }

    Log::Info("WiFi connected.");
    IPAddress ip = WiFi.localIP();

    std::ostringstream log;
    log << "IP address: " << (int)ip[0] << "." << (int)ip[1] << "." << (int)ip[2] << "." << (int)ip[3];
    Log::Info(log.str().c_str());
}

static void showExistState() {
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        String object_id;
        Config::GetObjectId(i, object_id);

        String error;
        bool exists;
        if (!YudetamagoClient::GetExistance(object_id.c_str(), exists, error) ) {
            Log::Error(error.c_str());
            showError(10);
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

        pixels.SetPixelColor(i, exists? EXISTS_COLOR: NOT_EXSITS_COLOR);
    }
    showNeoPixel();
}

static void toggleExistState(int index) {
    object_exists[index] = !object_exists[index];
    bool exists = object_exists[index];

    if (exists) {
        std::ostringstream msg;
        msg << "Detected button pressed: exist #";
        msg << index;
        Log::Debug(msg.str().c_str());
    } else {
        std::ostringstream msg;
        msg << "Detected button pressed: not exist #";
        msg << index;
        Log::Debug(msg.str().c_str());
    }

    String object_id;
    Config::GetObjectId(index, object_id);

    String error;
    if (!YudetamagoClient::SetExistance(object_id.c_str(), exists, error)) {
        Log::Error(error.c_str());
        showError(10);
        return;
    }
    if (exists) {
        std::ostringstream msg;
        msg << "SetExistance: exist #";
        msg << index;
        Log::Info(msg.str().c_str());
    } else {
        std::ostringstream msg;
        msg << "SetExistance: not exist #";
        msg << index;
        Log::Info(msg.str().c_str());
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
    setLedCommand.SetPixels(&pixels);
    commandLine.AddCommand(&setLedCommand);

    setObjectIdCommand.Initialize(stream);
    commandLine.AddCommand(&setObjectIdCommand);

    setSsidCommand.Initialize(stream);
    commandLine.AddCommand(&setSsidCommand);

    loopColorChangeCommand.Initialize(stream);
    loopColorChangeCommand.SetPixels(&pixels);
    commandLine.AddCommand(&loopColorChangeCommand);

    resetCommand.Initialize(stream);
    commandLine.AddCommand(&resetCommand);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Yudetamago...");

    pixels.Begin();
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        pixels.SetPixelColor(i, BLACK_COLOR);
    }
    showNeoPixel();

    pinMode(MODE_PIN,    INPUT_PULLUP);
    pinMode(STOCK_0_PIN, INPUT_PULLUP);
    pinMode(STOCK_1_PIN, INPUT_PULLUP);
    pinMode(STOCK_2_PIN, INPUT_PULLUP);
    pinMode(STOCK_3_PIN, INPUT_PULLUP);
    pinMode(STOCK_4_PIN, INPUT_PULLUP);

    if (!Config::Initialize()) {
        Log::Error("Faild to Config::Initialize().");
        while (1) {
            showError(1);
        }
    }

    if (digitalRead(MODE_PIN) == LOW) {
        Log::Info("Detected Config mode.");

        Config::Read();
        pixels.SetPixelColor(NEO_PIXEL_STOCK_0, CONFIG_COLOR);
        showNeoPixel();

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
            showError(1);
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

    showExistState();
    Serial.println("Started Yudetamago.");
}

void loop() {
    for (int times=0; times<NCMB_ACCESS_INTERVAL; times+=NCMB_BUTTON_INTERVAL) {
        while (commandLine.AnalyzeSerial());

        if (digitalRead(STOCK_0_PIN) == LOW) {
            toggleExistState(0);
        } else if (digitalRead(STOCK_1_PIN) == LOW) {
            toggleExistState(1);
        } else if (digitalRead(STOCK_2_PIN) == LOW) {
            toggleExistState(2);
        } else if (digitalRead(STOCK_3_PIN) == LOW) {
            toggleExistState(3);
        } else if (digitalRead(STOCK_4_PIN) == LOW) {
            toggleExistState(4);
        } else {
            vTaskDelay(NCMB_BUTTON_INTERVAL);
            continue;
        }

        showExistState();

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

    showExistState();
}

#endif // TARGET
