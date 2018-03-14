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

bool CommandLine::executeLogLevelCommand(const CommandLineParser *parser)
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
    if (strcmp(parser.GetName(), "log") == 0) {
        return executeLogCommand(&parser);
    }
    if (strcmp(parser.GetName(), "loglevel") == 0) {
        return executeLogLevelCommand(&parser);
    }
    if (strcmp(parser.GetName(), "info") == 0) {
        return executeInfoCommand(&parser);
    }
    writeError("parser: unknown command");
    return true;
}

