#include "Log.h"

static int log_size             = 0;
static int log_next_write_index = 0;
static int log_level            = Log::LOG_LEVEL_INFO;

static std::string logs[Log::LOG_CAPACITY];

static void add(const std::string& message)
{
    logs[log_next_write_index] = message;

    ++log_next_write_index;
    log_next_write_index = log_next_write_index % Log::LOG_CAPACITY;

    if (log_size < Log::LOG_CAPACITY) {
        ++log_size;
    }
}

void Log::SetLevel(int level)
{
    log_level = level;
}

int Log::Size()
{
    return log_size;
}

bool Log::GetLog(int index, std::string& log)
{
    if (index < 0 || log_size <= index) {
        return 0;
    }

    if (log_next_write_index == log_size) {
        // log_size  != LOG_CAPACITY
        // log_start == 0
        log = logs[index];
        return true;
    }

    // log_size  == LOG_CAPACITY
    // log_start == log_next_write_index
    int i = (log_next_write_index + index) % LOG_CAPACITY;
    log = logs[i];
    return true;
}

void Log::Fatal(const char *message)
{
    if (log_level > LOG_LEVEL_FATAL) {
        return;
    }
    add(std::string("[FATAL] ") + message);
}

void Log::Error(const char *message)
{
    if (log_level > LOG_LEVEL_ERROR) {
        return;
    }
    add(std::string("[ERROR] ") + message);
}

void Log::Warn(const char *message)
{
    if (log_level > LOG_LEVEL_WARN) {
        return;
    }
    add(std::string("[WARN ] ") + message);
}

void Log::Info(const char *message)
{
    if (log_level > LOG_LEVEL_INFO) {
        return;
    }
    add(std::string("[INFO ] ") + message);
}

void Log::Debug(const char *message)
{
    if (log_level > LOG_LEVEL_DEBUG) {
        return;
    }
    add(std::string("[DEBUG] ") + message);
}

void Log::Trace(const char *message)
{
    if (log_level > LOG_LEVEL_TRACE) {
        return;
    }
    add(std::string("[TRACE] ") + message);
}

void Log::Clean()
{
    log_size = 0;
    log_next_write_index = 0;
}
