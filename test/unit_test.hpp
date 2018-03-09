
#pragma once

#include <stdio.h>

#define UT_RUN(func) \
    printf("----- %s:%d %s() start -----\n", __FILE__, __LINE__, #func); \
    func(); \
    printf("----- %s:%d %s() end   -----\n", __FILE__, __LINE__, #func); \

