#include "unit_test.hpp"

#include "string.h"
#include "Log.h"
#include "Log.cpp"

void test_trace()
{
    Log::Trace("abc");
    UT_ASSERT_INT(0, Log::Size());

    std::string log;
    UT_ASSERT_FALSE(Log::GetLog(0, log));
    Log::Clean();
}

void test_debug()
{
    Log::Debug("abc");
    UT_ASSERT_INT(0, Log::Size());

    std::string log;
    UT_ASSERT_FALSE(Log::GetLog(0, log));
    Log::Clean();
}

void test_info()
{
    Log::Info("abc");
    std::string log;
    UT_ASSERT_TRUE(Log::GetLog(0, log));
    UT_ASSERT_STRING("[INFO ] abc", log.c_str());
    UT_ASSERT_INT(1, Log::Size());
    UT_ASSERT_FALSE(Log::GetLog(1, log));
    Log::Clean();
}

void test_error()
{
    Log::Error("abc");
    std::string log;
    UT_ASSERT_TRUE(Log::GetLog(0, log));
    UT_ASSERT_STRING("[ERROR] abc", log.c_str());
    UT_ASSERT_INT(1, Log::Size());
    UT_ASSERT_FALSE(Log::GetLog(1, log));
    Log::Clean();
}

void test_warn()
{
    Log::Warn("abc");
    std::string log;
    UT_ASSERT_TRUE(Log::GetLog(0, log));
    UT_ASSERT_STRING("[WARN ] abc", log.c_str());
    UT_ASSERT_INT(1, Log::Size());
    UT_ASSERT_FALSE(Log::GetLog(1, log));
    Log::Clean();
}

void test_fatal()
{
    Log::Fatal("abc");
    std::string log;
    UT_ASSERT_TRUE(Log::GetLog(0, log));
    UT_ASSERT_STRING("[FATAL] abc", log.c_str());
    UT_ASSERT_INT(1, Log::Size());
    UT_ASSERT_FALSE(Log::GetLog(1, log));
    Log::Clean();
}

void test_set_level()
{
    Log::SetLevel(Log::LOG_LEVEL_TRACE);
    Log::Trace("abc"); UT_ASSERT_INT(1, Log::Size());
    Log::Debug("abc"); UT_ASSERT_INT(2, Log::Size());
    Log::Info("abc");  UT_ASSERT_INT(3, Log::Size());
    Log::Warn("abc");  UT_ASSERT_INT(4, Log::Size());
    Log::Error("abc"); UT_ASSERT_INT(5, Log::Size());
    Log::Fatal("abc"); UT_ASSERT_INT(6, Log::Size());
    Log::Clean();

    Log::SetLevel(Log::LOG_LEVEL_DEBUG);
    Log::Trace("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Debug("abc"); UT_ASSERT_INT(1, Log::Size());
    Log::Info("abc");  UT_ASSERT_INT(2, Log::Size());
    Log::Warn("abc");  UT_ASSERT_INT(3, Log::Size());
    Log::Error("abc"); UT_ASSERT_INT(4, Log::Size());
    Log::Fatal("abc"); UT_ASSERT_INT(5, Log::Size());
    Log::Clean();

    Log::SetLevel(Log::LOG_LEVEL_INFO);
    Log::Trace("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Debug("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Info("abc");  UT_ASSERT_INT(1, Log::Size());
    Log::Warn("abc");  UT_ASSERT_INT(2, Log::Size());
    Log::Error("abc"); UT_ASSERT_INT(3, Log::Size());
    Log::Fatal("abc"); UT_ASSERT_INT(4, Log::Size());
    Log::Clean();

    Log::SetLevel(Log::LOG_LEVEL_WARN);
    Log::Trace("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Debug("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Info("abc");  UT_ASSERT_INT(0, Log::Size());
    Log::Warn("abc");  UT_ASSERT_INT(1, Log::Size());
    Log::Error("abc"); UT_ASSERT_INT(2, Log::Size());
    Log::Fatal("abc"); UT_ASSERT_INT(3, Log::Size());
    Log::Clean();

    Log::SetLevel(Log::LOG_LEVEL_ERROR);
    Log::Trace("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Debug("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Info("abc");  UT_ASSERT_INT(0, Log::Size());
    Log::Warn("abc");  UT_ASSERT_INT(0, Log::Size());
    Log::Error("abc"); UT_ASSERT_INT(1, Log::Size());
    Log::Fatal("abc"); UT_ASSERT_INT(2, Log::Size());
    Log::Clean();

    Log::SetLevel(Log::LOG_LEVEL_FATAL);
    Log::Trace("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Debug("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Info("abc");  UT_ASSERT_INT(0, Log::Size());
    Log::Warn("abc");  UT_ASSERT_INT(0, Log::Size());
    Log::Error("abc"); UT_ASSERT_INT(0, Log::Size());
    Log::Fatal("abc"); UT_ASSERT_INT(1, Log::Size());
    Log::Clean();
}

void test_full_log()
{
    for (int i=0; i<Log::LOG_CAPACITY; i++) {
        Log::Fatal("abc"); UT_ASSERT_INT(i+1, Log::Size());
    }
    for (int i=0; i<Log::LOG_CAPACITY; i++) {
        Log::Fatal("abc"); UT_ASSERT_INT(Log::LOG_CAPACITY, Log::Size());
    }
}

int main(int argc, char **argv)
{
    UT_RUN(test_trace);
    UT_RUN(test_debug);
    UT_RUN(test_info);
    UT_RUN(test_error);
    UT_RUN(test_warn);
    UT_RUN(test_fatal);
    UT_RUN(test_set_level);
    UT_RUN(test_full_log);
    return 0;
}
