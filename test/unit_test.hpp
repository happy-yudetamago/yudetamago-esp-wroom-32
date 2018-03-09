
#pragma once

#include <stdio.h>
#include <assert.h>

#define UT_RUN(func) \
    printf("----- %s:%d %s() start -----\n", __FILE__, __LINE__, #func); \
    func(); \
    printf("----- %s:%d %s() end   -----\n", __FILE__, __LINE__, #func); \

#define UT_ASSERT_STRING(expected, actual)                              \
    if (strcmp(expected, actual) != 0) {                                \
        printf("assert fail\n  expected : %s\n  actual: %s\n",          \
               expected, actual);                                       \
        assert(false);                                                  \
    }                                                                   \

#define UT_ASSERT_POINTER(expected, actual)                             \
    if (expected != actual) {                                           \
        printf("assert fail\n  expected : %p\n  actual: %p\n",          \
               expected, actual);                                       \
        assert(false);                                                  \
    }                                                                   \

#define UT_ASSERT_TRUE(actual)  assert(actual)
#define UT_ASSERT_FALSE(actual) assert(!actual)
