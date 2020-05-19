// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "string_ext.h"

// Project headers
#include "../crt_ext/string_ext.h"

// STL headers
#include <stdexcept>

namespace stdext {

unsigned int stou(const std::string& str, size_t* pos, int base)
{
    const unsigned long result = std::stoul(str, pos, base);
    if (result > std::numeric_limits<unsigned int>::max()) {
        throw std::out_of_range("stdext::stou");
    }
    return static_cast<unsigned int>(result);
}

std::string to_string(int value, int base)
{
    const char* format = nullptr;
    switch (base) {
        case 8: {
            format = "%od";
            break;
        }
        case 10: {
            format = "%d";
            break;
        }
        case 16: {
            format = "%xd";
            break;
        }
        default: throw std::invalid_argument("Invalid base");
    }
    char buffer[16];
    const int n = sprintf(buffer, format, value);
    return std::string(buffer, n);
}

std::string to_string(long value, int base)
{
    const char* format = nullptr;
    switch (base) {
        case 8: {
            format = "%old";
            break;
        }
        case 10: {
            format = "%ld";
            break;
        }
        case 16: {
            format = "%xld";
            break;
        }
        default: throw std::invalid_argument("Invalid base");
    }
    char buffer[32];
    const int n = sprintf(buffer, format, value);
    return std::string(buffer, n);
}

std::string to_string(long long value, int base)
{
    const char* format = nullptr;
    switch (base) {
        case 8: {
            format = "%olld";
            break;
        }
        case 10: {
            format = "%lld";
            break;
        }
        case 16: {
            format = "%xlld";
            break;
        }
        default: throw std::invalid_argument("Invalid base");
    }
    char buffer[32];
    const int n = sprintf(buffer, format, value);
    return std::string(buffer, n);
}

std::string to_string(unsigned value, int base)
{
    const char* format = nullptr;
    switch (base) {
        case 8: {
            format = "%ou";
            break;
        }
        case 10: {
            format = "%u";
            break;
        }
        case 16: {
            format = "%xu";
            break;
        }
        default: throw std::invalid_argument("Invalid base");
    }
    char buffer[16];
    const int n = sprintf(buffer, format, value);
    return std::string(buffer, n);
}

std::string to_string(unsigned long value, int base)
{
    const char* format = nullptr;
    switch (base) {
        case 8: {
            format = "%olu";
            break;
        }
        case 10: {
            format = "%lu";
            break;
        }
        case 16: {
            format = "%xlu";
            break;
        }
        default: throw std::invalid_argument("Invalid base");
    }
    char buffer[32];
    const int n = sprintf(buffer, format, value);
    return std::string(buffer, n);
}

std::string to_string(unsigned long long value, int base)
{
    const char* format = nullptr;
    switch (base) {
        case 8: {
            format = "%ollu";
            break;
        }
        case 10: {
            format = "%llu";
            break;
        }
        case 16: {
            format = "%xllu";
            break;
        }
        default: throw std::invalid_argument("Invalid base");
    }
    char buffer[32];
    const int n = sprintf(buffer, format, value);
    return std::string(buffer, n);
}

std::string to_string(const int128_t& value)
{
    char buffer[41];
    _i128toa(&value, buffer, 10);
    return std::string(buffer);
}

std::string to_string(const uint128_t& value)
{
    char buffer[40];
    _u128toa(&value, buffer, 10);
    return std::string(buffer);
}

std::string to_string(const int128_t& value, int base)
{
    if (!(base == 8 || base == 10 || base == 16)) throw std::invalid_argument("Invalid base");
    char buffer[44];
    _i128toa(&value, buffer, base);
    return std::string(buffer);
}

std::string to_string(const uint128_t& value, int base)
{
    if (!(base == 8 || base == 10 || base == 16)) throw std::invalid_argument("Invalid base");
    char buffer[44];
    _u128toa(&value, buffer, base);
    return std::string(buffer);
}

}  // namespace stdext
