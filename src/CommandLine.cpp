#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

#include "CommandLine.h"
#include "Version.h"
#include "Log.h"
#include "Config.h"
#include "CommandLineParser.h"

bool deviceConnected = false;

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
            Serial.write(rxValue[i]);
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
    BLEServer *pServer = BLEDevice::createServer();
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

boolean CommandLine::AnalyzeBluetooth()
{
    if (!enableBluetooth) {
        return false;
    }
    // std::string message = pCharacteristic->getValue();
    // if (message.length() <= 0) {
    //     return false;
    // }

    // for (std::string::iterator ite = message.begin(); ite != message.end(); ite++) {
    //     Write(*ite);
    // }
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

void CommandLine::SetPixels(Adafruit_NeoPixel* pixels)
{
    this->pixels = pixels;
}

size_t CommandLine::writeMessage(const char *message)
{
    Serial.print(message);
    int length = strlen(message);
    if (enableBluetooth) {
        pCharacteristic->setValue((uint8_t*)message, length);
    }
    return length;
}

size_t CommandLine::writeChar(char ch)
{
    Serial.write(ch);
    if (enableBluetooth) {
        pCharacteristic->setValue((uint8_t*)&ch, 1);
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
    if (Config::WriteWifiConfig(ssid, pass) &&
        Config::ReadWifiConfig(ssid, pass)) {
        writeInfo("Successed to write wifi config.");
    } else {
        writeError("Failed to write wifi config.");
    }
    return true;
}

bool CommandLine::executeSetObjectIdCommand(const CommandLineParser *parser)
{
    String info;
    const char *parsedObjectId = parser->GetFirstArg();
    if (parsedObjectId == 0) {
        writeError("set_objectid: ObjectID not found.");
        return false;
    }

    if (parser->NextArg(parsedObjectId) != 0) {
        writeError("set_objectid: too many arguments.");
        return false;
    }

    String objectId(parsedObjectId);
    if (Config::WriteObjectId(objectId) &&
        Config::ReadObjectId(objectId)) {
        writeInfo("Successed to write objectId.");
    } else {
        writeError("Failed to write objectId.");
    }
    return true;
}

bool CommandLine::executeLogCommand(const CommandLineParser *parser)
{
    for (int i=0; i<Log::LOG_CAPACITY; i++) {
        String log;
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
    pixels->setPixelColor(ledIndex, red, green, blue);

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
    pixels->show();
    pixels->show();
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
    if (Config::ReadWifiConfig(ssid, pass)) {
        writeMessage("ssid=");
        writeMessage(ssid.c_str());
        writeMessage("\npass=");
        writeMessage(pass.c_str());
        writeMessage("\n");
    } else {
        writeError("ReadWifiConfig: fails");
    }

    String objectId;
    if (Config::ReadObjectId(objectId)) {
        writeMessage("objectId=");
        writeMessage(objectId.c_str());
        writeMessage("\n");
    } else {
        writeError("ReadObjectId: fails");
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

