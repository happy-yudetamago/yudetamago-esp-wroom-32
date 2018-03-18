#pragma once

class CommandLineParser;

class CommandLine {
public:
    CommandLine();

public:
    void InitializeBluetooth();
    boolean AnalyzeBluetooth();
    boolean AnalyzeSerial();
    void Write(char ch);

private:
    size_t writeMessage(const char *message);
    size_t writeChar(char ch);
    void writeError(const char *message);
    void writeInfo(const char *message);
    bool executeSetSsidCommand(const CommandLineParser *parser);
    bool executeSetObjectIdCommand(const CommandLineParser *parser);
    bool executeLogCommand(const CommandLineParser *parser);
    bool executeSetLogLevelCommand(const CommandLineParser *parser);
    bool executeInfoCommand(const CommandLineParser *parser);
    bool executeCommandLine(const char *line);

private:
    String  buf;
    boolean enableBluetooth;
};
