#include <BluetoothSerial.h>

#include "CommandLine.h"
#include "Log.h"
#include "Config.h"
#include "CommandLineParser.hpp"

static BluetoothSerial SerialBT;

CommandLine::CommandLine() : enableBluetooth(false)
{
}

void CommandLine::InitializeBluetooth()
{
    enableBluetooth = true;
    SerialBT.begin("Yudetamago config"); // Bluetooth device name
}

boolean CommandLine::AnalyzeBluetooth()
{
    if (!enableBluetooth) {
        return false;
    }
    if (!SerialBT.available()) {
        return false;
    }

    char ch = SerialBT.read();
    Write(ch);
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
    writeChar(ch);

    switch (ch) {
    case '\n':
        writeMessage("$ ");
    case '\r':
        if (buf.length() != 0) {
            executeCommandLine(buf.c_str());
            buf = String();
        }
        break;
    default:
        buf.concat(ch);
        break;
    }
}

size_t CommandLine::writeMessage(const char *message)
{
    Serial.print(message);
    int length = strlen(message);
    if (enableBluetooth) {
        size_t result = SerialBT.write((uint8_t*)message, length);
        return result;
    }
    return length;
}

size_t CommandLine::writeChar(char ch)
{
    Serial.write(ch);
    if (enableBluetooth) {
        return SerialBT.write(ch);
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
    return true;
}
