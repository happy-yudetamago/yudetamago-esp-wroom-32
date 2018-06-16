#include <Adafruit_NeoPixel.h>
#include <WiFi.h>

#include "Log.h"
#include "Config.h"
#include "CommandLine.h"
#include "YudetamagoClient.h"

#define MODE_PIN          14
#define STOCK_0_PIN       14
#define LED_PIN           13
#define NEO_PIXEL_PIN     12
#define NUM_OF_NEO_PIXELS 1
#define NEO_PIXEL_STOCK_0 0

Adafruit_NeoPixel pixels        = Adafruit_NeoPixel(NUM_OF_NEO_PIXELS,
                                                    NEO_PIXEL_PIN,
                                                    NEO_GRB + NEO_KHZ800);

const uint32_t BLACK_COLOR      = Adafruit_NeoPixel::Color(0, 0, 0);
const uint32_t ERROR_COLOR      = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t WAITING_COLOR    = Adafruit_NeoPixel::Color(255, 255, 51);
const uint32_t CONFIG_COLOR     = Adafruit_NeoPixel::Color(255, 255, 255);
const uint32_t EXISTS_COLOR     = Adafruit_NeoPixel::Color(0, 0, 0);
const uint32_t NOT_EXSITS_COLOR = Adafruit_NeoPixel::Color(255, 0, 0);
const int NCMB_BUTTON_INTERVAL       = (100);
const int NCMB_AFTER_BUTTON_INTERVAL = (1000);
const int NCMB_ACCESS_INTERVAL       = (5 * 60 * 1000);
String object_id;
bool exists = true;

static void showError(int times) {
    for (int i=0; times < 0 || i<times; i++) {
        pixels.setPixelColor(NEO_PIXEL_STOCK_0, ERROR_COLOR);
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
        pixels.show();
        pixels.show();
        vTaskDelay(1000);

        pixels.setPixelColor(NEO_PIXEL_STOCK_0, BLACK_COLOR);
        pixels.show();
        pixels.show();
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
    while (WiFi.status() != WL_CONNECTED) {
        pixels.setPixelColor(NEO_PIXEL_STOCK_0, WAITING_COLOR);
        pixels.show();
        pixels.show();
        Log::Debug(".");
        vTaskDelay(500);

        pixels.setPixelColor(NEO_PIXEL_STOCK_0, BLACK_COLOR);
        pixels.show();
        pixels.show();
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
    pixels.setPixelColor(NEO_PIXEL_STOCK_0, exists? EXISTS_COLOR: NOT_EXSITS_COLOR);
    pixels.show();
    pixels.show();
}

void setup() {
    Serial.begin(115200);
    Serial.println("");

    pixels.begin();
    pixels.setBrightness(255);

    pinMode(LED_PIN,  OUTPUT);
    pinMode(MODE_PIN, INPUT);

    if (!Config::Initialize()) {
        Log::Error("Faild to Config::Initialize().");
        showError(-1);
    }

    if (digitalRead(MODE_PIN) == LOW) {
        Config::Read();

        Log::Info("Detected Config mode.");
        pixels.setPixelColor(NEO_PIXEL_STOCK_0, CONFIG_COLOR);
        pixels.show();
        pixels.show();

        CommandLine cmd;
        cmd.SetPixels(&pixels);
        cmd.InitializeBluetooth();
        while (1) {
            cmd.AnalyzeBluetooth();
            vTaskDelay(20);
        }
        // can not reach here.
    }
    Log::Info("Detected Normal mode.");
    if (!Config::Read()) {
        Log::Error("Faild to read objectId.");
        showError(-1);
    }
    Config::GetObjectId(0, object_id);
    String log = "objectId : ";
    log += object_id;
    Log::Info(log.c_str());
    reconnectWifi();

    String error;
    if (!YudetamagoClient::GetExistance(object_id.c_str(), exists, error) ) {
        Log::Error(error.c_str());
        showError(10);
        return;
    }
    if (exists) {
        Log::Info("Detected initial status: exists");
    } else {
        Log::Info("Detected initial status: not exists");
    }
    showExistState();
}

void loop() {
    static CommandLine cmd;
    cmd.SetPixels(&pixels);
    for (int times=0; times<NCMB_ACCESS_INTERVAL; times+=NCMB_BUTTON_INTERVAL) {
        while (cmd.AnalyzeSerial()) {
        }
        if (digitalRead(STOCK_0_PIN) != LOW) {
            vTaskDelay(NCMB_BUTTON_INTERVAL);
            continue;
        }

        // button pressed
        exists = !exists;
        if (exists) {
            Log::Debug("Detected button pressed: exist");
        } else {
            Log::Debug("Detected button pressed: not exist");
        }
        String error;
        if (!YudetamagoClient::SetExistance(object_id.c_str(), exists, error)) {
            Log::Error(error.c_str());
            showError(10);
            return;
        }
        if (exists) {
            Log::Info("SetExistance: exist");
        } else {
            Log::Info("SetExistance: not exist");
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

    String error;
    bool existsPrev = exists;
    if (!YudetamagoClient::GetExistance(object_id.c_str(), exists, error) ) {
        Log::Error(error.c_str());
        showError(10);
        return;
    }
    if (exists == existsPrev) {
        Log::Debug("status not change:");
        return;
    }
    if (exists) {
        Log::Info("Detected status change: exists");
    } else {
        Log::Info("Detected status change: not exists");
    }
    showExistState();
}
