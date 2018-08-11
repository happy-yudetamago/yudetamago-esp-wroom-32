#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

#include "log/Log.h"

#include "CommandLine.h"
#include "Version.h"
#include "Config.h"
#include "CommandLineParser.h"
#include "hardware_defines.h"

bool deviceConnected = false;
bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
public:
    MyCallbacks(CommandLine* commandLine) : commandLine(commandLine) {
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();

        Serial.println("********");
        for (int i=0; i<rxValue.length(); i++) {
            commandLine->Write(rxValue[i]);
        }
    }

private:
    CommandLine* commandLine;
};

CommandLine::CommandLine() : enableBluetooth(false), pixels(0)
{
}

void CommandLine::InitializeBluetooth()
{
    enableBluetooth = true;
    BLEDevice::init("Yudetamago config");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY);

    pCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE
        );

    pCharacteristic->setCallbacks(new MyCallbacks(this));

    // Start the service
    pService->start();

    // Start advertising
    pServer->getAdvertising()->start();
}

boolean CommandLine::Analyze()
{
    boolean bSerialResult = AnalyzeSerial();
    boolean bStreamResult = AnalyzeBluetooth();
    return bSerialResult || bStreamResult;
}

boolean CommandLine::AnalyzeBluetooth()
{
    if (!enableBluetooth) {
        return false;
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    return true;
}

boolean CommandLine::AnalyzeSerial()
{
    if (!Serial.available()) {
        return false;
    }

    char ch = Serial.read();
    Write(ch);
    return true;
}

void CommandLine::Write(char ch)
{
    if (ch == '\r') {
        return;
    }
    writeChar(ch);

    switch (ch) {
    case '\n':
        if (buf.length() != 0) {
            executeCommandLine(buf.c_str());
            buf = String();
        }
        writeMessage("$ ");
        break;
    default:
        buf.concat(ch);
        break;
    }
}

void CommandLine::SetPixels(NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>* pixels)
{
    this->pixels = pixels;
}

size_t CommandLine::writeMessage(const char *message)
{
    Serial.print(message);
    int length = strlen(message);
    if (enableBluetooth) {
        pCharacteristic->setValue((uint8_t*)message, length);
        pCharacteristic->notify();
        delay(10); // bluetooth stack will go into congestion, if too many packets are sent
    }
    return length;
}

size_t CommandLine::writeChar(char ch)
{
    Serial.write(ch);
    if (enableBluetooth) {
        pCharacteristic->setValue((uint8_t*)&ch, 1);
        pCharacteristic->notify();
        delay(10); // bluetooth stack will go into congestion, if too many packets are sent
        return 1;
    }
    return 1;
}

void CommandLine::writeError(const char *message)
{
    Log::Error(message);
    if (enableBluetooth) {
        writeMessage(message);
        writeMessage("\n");
    }
}

void CommandLine::writeInfo(const char *message)
{
    Log::Info(message);
    if (enableBluetooth) {
        writeMessage(message);
        writeMessage("\n");
    }
}

bool CommandLine::executeSetSsidCommand(const CommandLineParser *parser)
{
    const char *parsedSsid = parser->GetFirstArg();
    if (parsedSsid == 0) {
        writeError("set_ssid: SSID not found.");
        return false;
    }

    const char *parsedPass = parser->NextArg(parsedSsid);
    if (parsedPass == 0) {
        writeError("set_ssid: password not found.");
        return false;
    }

    if (parser->NextArg(parsedPass) != 0) {
        writeError("set_ssid: too many arguments.");
        return false;
    }

    String ssid(parsedSsid);
    String pass(parsedPass);
    Config::SetWifiConfig(ssid, pass);
    if (Config::Write()) {
        writeInfo("Successed to write wifi config.");
    } else {
        writeError("Failed to write wifi config.");
    }
    return true;
}

bool CommandLine::executeSetObjectIdCommand(const CommandLineParser *parser)
{
    // validate args
    const char *parsedObjectId = parser->GetFirstArg();
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        if (parsedObjectId == 0) {
            writeError("set_objectid: too few arguments.");
            return false;
        }
        parsedObjectId = parser->NextArg(parsedObjectId);
    }
    if (parsedObjectId != 0) {
        writeError("set_objectid: too many arguments.");
        return false;
    }

    // write data
    parsedObjectId = parser->GetFirstArg();
    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        String objectId(parsedObjectId);
        Config::SetObjectId(i, objectId);
        parsedObjectId = parser->NextArg(parsedObjectId);
    }
    if (Config::Write()) {
        writeInfo("Successed to write objectId.");
    } else {
        writeError("Failed to write objectId.");
    }
    return true;
}

bool CommandLine::executeLogCommand(const CommandLineParser *parser)
{
    for (int i=0; i<Log::LOG_CAPACITY; i++) {
        std::string log;
        if (!Log::GetLog(i, log)) {
            return true;
        }
        writeMessage(log.c_str());
        writeMessage("\n");
    }
    return true;
}

bool CommandLine::executeSetLedCommand(const CommandLineParser *parser)
{
    if (!pixels) {
        writeError("set_led: pixels is null.");
        return false;
    }

    String info;
    const char *parsedLedIndex = parser->GetFirstArg();
    if (parsedLedIndex == 0) {
        writeError("set_led: led index not found.");
        return false;
    }
    int ledIndex = atoi(parsedLedIndex);
    if (ledIndex < 0) {
        writeError("set_led: led index region error.");
        return false;
    }

    const char *parsedRedColor = parser->NextArg(parsedLedIndex);
    if (parsedRedColor == 0) {
        writeError("set_led: red color not found.");
        return false;
    }
    uint8_t red = atoi(parsedRedColor);

    const char *parsedGreenColor = parser->NextArg(parsedRedColor);
    if (parsedGreenColor == 0) {
        writeError("set_led: green color not found.");
        return false;
    }
    uint8_t green = atoi(parsedGreenColor);

    const char *parsedBlueColor = parser->NextArg(parsedGreenColor);
    if (parsedBlueColor == 0) {
        writeError("set_led: blue color not found.");
        return false;
    }
    uint8_t blue = atoi(parsedBlueColor);

    printf("set_led: index=%d, red=%d, green=%d, blue=%d\n", ledIndex, red, green, blue);
    pixels->SetPixelColor(ledIndex, RgbColor(red, green, blue));

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
    pixels->Show();
    return true;
}

void CommandLine::writeButtonState(const char *name, int pin)
{
    String message = name;
    if (digitalRead(pin) == LOW) {
        message += ": pressed";
    } else {
        message += ": not pressed";
    }
    writeInfo(message.c_str());
}

bool CommandLine::executeGetButtonStateCommand(const CommandLineParser *parser)
{
    const char *arg = parser->GetFirstArg();
    if (arg != 0) {
        writeError("get_button_state: too many arguments.");
        return false;
    }

    writeButtonState("button #0", STOCK_0_PIN);
    writeButtonState("button #1", STOCK_1_PIN);
    writeButtonState("button #2", STOCK_2_PIN);
    writeButtonState("button #3", STOCK_3_PIN);
    writeButtonState("button #4", STOCK_4_PIN);
    return true;
}

bool CommandLine::executeSetLogLevelCommand(const CommandLineParser *parser)
{
    const char *level = parser->GetFirstArg();
    if (level == 0) {
        writeError("loglevel: no log level");
        return false;
    }
    if (strcmp(level, "TRACE") == 0) {
        writeInfo("loglevel: set TRACE");
        Log::SetLevel(Log::LOG_LEVEL_TRACE);
        return true;
    }
    if (strcmp(level, "DEBUG") == 0) {
        writeInfo("loglevel: set DEBUG");
        Log::SetLevel(Log::LOG_LEVEL_DEBUG);
        return true;
    }
    if (strcmp(level, "INFO") == 0) {
        writeInfo("loglevel: set INFO");
        Log::SetLevel(Log::LOG_LEVEL_INFO);
        return true;
    }
    if (strcmp(level, "WARN") == 0) {
        writeInfo("loglevel: set WARN");
        Log::SetLevel(Log::LOG_LEVEL_WARN);
        return true;
    }
    if (strcmp(level, "ERROR") == 0) {
        writeInfo("loglevel: set ERROR");
        Log::SetLevel(Log::LOG_LEVEL_ERROR);
        return true;
    }
    if (strcmp(level, "FATAL") == 0) {
        writeInfo("loglevel: set FATAL");
        Log::SetLevel(Log::LOG_LEVEL_FATAL);
        return true;
    }
    writeError("loglevel: unknown level");
    return false;
}

bool CommandLine::executeInfoCommand(const CommandLineParser *parser)
{
    writeMessage("version=");
    writeMessage(VERSION);
    writeMessage("\n");

    String ssid;
    String pass;
    Config::GetWifiConfig(ssid, pass);
    writeMessage("ssid=");
    writeMessage(ssid.c_str());
    writeMessage("\npass=");
    writeMessage(pass.c_str());
    writeMessage("\n");

    for (int i=0; i<OBJECT_ID_SIZE; i++) {
        String objectId;
        Config::GetObjectId(i, objectId);
        writeMessage("objectId=");
        writeMessage(objectId.c_str());
        writeMessage("\n");
    }
    return true;
}

bool CommandLine::executeCommandLine(const char *line)
{
    CommandLineParser parser(line);
    if (!parser.Parse()) {
        return false;
    }

    if (strcmp(parser.GetName(), "set_ssid") == 0) {
        return executeSetSsidCommand(&parser);
    }
    if (strcmp(parser.GetName(), "set_objectid") == 0) {
        return executeSetObjectIdCommand(&parser);
    }
    if (strcmp(parser.GetName(), "set_led") == 0) {
        return executeSetLedCommand(&parser);
    }
    if (strcmp(parser.GetName(), "get_button_state") == 0) {
        return executeGetButtonStateCommand(&parser);
    }
    if (strcmp(parser.GetName(), "log") == 0) {
        return executeLogCommand(&parser);
    }
    if (strcmp(parser.GetName(), "set_loglevel") == 0) {
        return executeSetLogLevelCommand(&parser);
    }
    if (strcmp(parser.GetName(), "info") == 0) {
        return executeInfoCommand(&parser);
    }
    writeError("parser: unknown command");
    return true;
}

