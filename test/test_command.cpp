#include "unit_test.hpp"

#include "string.h"
#include "CommandLineParser.hpp"

void test_parse()
{
    const char *line = "set_ssid  xxx";
    CommandLineParser parser(line);
    UT_ASSERT_TRUE(parser.Parse());
    UT_ASSERT_STRING("set_ssid", parser.GetName());
    const char *arg = parser.GetFirstArg();
    UT_ASSERT_STRING("xxx", arg);
    arg = parser.NextArg(arg);
    UT_ASSERT_POINTER((void*)0, arg);
}

int main(int argc, char **argv)
{
    UT_RUN(test_parse);
    return 0;
}
