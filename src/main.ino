#include <NeoPixelBus.h>
#include <WiFi.h>
#include <sstream>

#include "Log.h"
#include "Config.h"
#include "CommandLine.h"
#include "YudetamagoClient.h"
#include "hardware_defines.h"

/**
 * Enable D_DIAG_NEO_PIXEL_VARIABLE_LED_PER_SEC,
 * if you do diag Neo Pixel LEDs.
 */
// #define D_DIAG_NEO_PIXEL_VARIABLE_LED_PER_SEC

/**
 * Enable D_DIAG_NEO_PIXEL_BLACK_LED,
 * if you do diag Neo Pixel LEDs.
 */
// #define D_DIAG_NEO_PIXEL_BLACK_LED

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixels(NUM_OF_NEO_PIXELS, NEO_PIXEL_PIN);

RgbColor BLACK_COLOR(0, 0, 0);
RgbColor ERROR_COLOR(255, 0, 0);
RgbColor WAITING_COLOR(255, 255, 51);
RgbColor CONFIG_COLOR(255, 255, 255);
RgbColor EXISTS_COLOR(0, 0, 0);
RgbColor NOT_EXSITS_COLOR(255, 0, 0);
const int NCMB_BUTTON_INTERVAL       = (100);
const int NCMB_AFTER_BUTTON_INTERVAL = (1000);
const int NCMB_ACCESS_INTERVAL       = (5 * 60 * 1000);

bool object_exists[OBJECT_ID_SIZE] = {true, true, true, true, true};

CommandLine commandLine;

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
    pixels.Show();
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
    String log = "IP address: ";
    IPAddress ip = WiFi.localIP();
    log += ip[0]; log += ".";
    log += ip[1]; log += ".";
    log += ip[2]; log += ".";
    log += ip[3];
    Log::Info(log.c_str());
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

void singleDiag() {
#ifdef D_DIAG_NEO_PIXEL_VARIABLE_LED_PER_SEC
    const RgbColor colors[] = {
        RgbColor(0,   0,   0),
        RgbColor(255, 0,   0),
        RgbColor(0,   255, 0),
        RgbColor(0,   0,   255),
        RgbColor(255, 255, 0),
        RgbColor(255, 0,   255),
        RgbColor(0,   255, 255),
        RgbColor(255, 255, 255)};
    const int colorsSize = sizeof(colors)/sizeof(colors[0]);
    while (1) {
        for (int c=0; c<colorsSize; c++) {
            RgbColor color = colors[c];
            for (int i=0; i<OBJECT_ID_SIZE; i++) {
                pixels.SetPixelColor(i, color);
            }
            showNeoPixel();
            vTaskDelay(1000);
        }
    }
#endif
#ifdef D_DIAG_NEO_PIXEL_BLACK_LED
    while (1) {
        for (int i=0; i<OBJECT_ID_SIZE; i++) {
            pixels.SetPixelColor(i, BLACK_COLOR);
        }
        showNeoPixel();
        vTaskDelay(200);
    }
#endif
}

void setup() {
    Serial.begin(115200);
    Serial.println("");

    pixels.Begin();
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        pixels.SetPixelColor(i, BLACK_COLOR);
    }
    showNeoPixel();
    commandLine.SetPixels(&pixels);

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
            while (commandLine.AnalyzeSerial());
        }
    }

    if (digitalRead(MODE_PIN) == LOW) {
        Log::Info("Detected Config mode.");

        Config::Read();
        pixels.SetPixelColor(NEO_PIXEL_STOCK_0, CONFIG_COLOR);
        showNeoPixel();

        commandLine.InitializeBluetooth();
        while (1) {
            commandLine.Analyze();
            vTaskDelay(20);
        }
        // can not reach here.
    }
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
    singleDiag();

    showExistState();
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
