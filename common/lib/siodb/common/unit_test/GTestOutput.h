// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <iostream>
#include <sstream>

// Following code is based on the ideas taken from here:
// https://stackoverflow.com/a/29155677/1540501

#if 0

// Unavailable because gtest declares ColoredPrintf() as as static,
// so it is finally not visible to linker.

namespace testing::internal {

enum GTestColor {
    COLOR_DEFAULT,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
};

void ColoredPrintf(GTestColor color, const char* fmt, ...);

}  // namespace testing::internal

#define TEST_PRINTF(fmt, ...)                                                                \
    do {                                                                                     \
        testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] ");   \
        testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, fmt, __VA_ARGS__); \
    } while (false)

#define TEST_PUTS(s)                                                                       \
    do {                                                                                   \
        testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); \
        testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, "%s\n", s);      \
    } while (false)

class TestCout : public std::ostringstream {
public:
    ~TestCout()
    {
        try {
            TEST_PRINTF("%s", str().c_str());
        } catch (...) {
            // Intentionally skip all exception here
        }
    }
};

#define TEST_COUT TestCout()

#else

#define TEST_PUTS(s)                                      \
    do {                                                  \
        std::cout << "[          ] " << (s) << std::endl; \
    } while (false)

#define TEST_COUT std::cout

#endif

#ifdef _DEBUG
#define DEBUG_TEST_PRINTF(fmt, ...) TEST_PRINTF(fmt, __VA_ARGS__)
#define DEBUG_TEST_PUTS(s) TEST_PUTS(s)
#define DEBUG_TEST_COUT TEST_COUT
#else
#define DEBUG_TEST_PRINTF(fmt, ...) \
    do {                            \
    } while (false)
#define DEBUG_TEST_PUTS(s) \
    do {                   \
    } while (false)
#define DEBUG_TEST_COUT std::ostringstream()
#endif  // _DEBUG
