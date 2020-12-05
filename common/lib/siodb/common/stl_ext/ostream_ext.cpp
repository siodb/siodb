// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ostream_ext.h"

// Project headers
#include "../crt_ext/detail/string_ext_detail.h"

// STL headers
#include <algorithm>
#include <iterator>

namespace stdext {

std::ostream& operator<<(std::ostream& os, const stdext::int128_t& value)
{
    const auto flags = os.flags();
    const bool uppercase = (flags & std::ios_base::uppercase) != 0;

    int base = 10;
    int baseLength = 0;
    const char* baseStr = nullptr;
    if (flags & std::ios_base::oct) {
        base = 8;
        baseLength = 1;
        baseStr = "0";
    } else if (flags & std::ios_base::hex) {
        base = 16;
        baseLength = 2;
        baseStr = uppercase ? "0X" : "0x";
    }

    char buffer[128 / 3 + 2];
    const auto p = _i128toa_impl(
            &value, buffer, sizeof(buffer), base, uppercase, (flags & std::ios_base::showpos) != 0);
    const auto length = (buffer + sizeof(buffer) - 1) - p;
    const auto lengthWithBase = length + baseLength;
    const auto fillLength = static_cast<std::streamsize>(lengthWithBase) >= os.width()
                                    ? 0
                                    : os.width() - lengthWithBase;
    if (flags & std::ios_base::internal) {
        if ((flags & std::ios_base::showbase) && baseStr) {
            os << baseStr;
            if (!os) return os;
        }
        if (fillLength > 0) {
            std::fill_n(std::ostream_iterator<char>(os), fillLength, os.fill());
            if (!os) return os;
        }
        os << p;
        if (!os) return os;
    } else if (flags & std::ios_base::right) {
        if ((flags & std::ios_base::showbase) && baseStr) {
            os << baseStr;
            if (!os) return os;
        }
        os << p;
        if (!os) return os;
        if (fillLength > 0) {
            std::fill_n(std::ostream_iterator<char>(os), fillLength, os.fill());
            if (!os) return os;
        }
    } else {
        if (fillLength > 0) {
            std::fill_n(std::ostream_iterator<char>(os), fillLength, os.fill());
            if (!os) return os;
        }
        if ((flags & std::ios_base::showbase) && baseStr) {
            os << baseStr;
            if (!os) return os;
        }
        os << buffer;
        if (!os) return os;
    }

    if (flags & std::ios_base::unitbuf) os.flush();
    return os;
}

std::ostream& operator<<(std::ostream& os, const stdext::uint128_t& value)
{
    const auto flags = os.flags();
    const bool uppercase = (flags & std::ios_base::uppercase) != 0;

    int base = 10;
    int baseLength = 0;
    const char* baseStr = nullptr;
    if (flags & std::ios_base::oct) {
        base = 8;
        baseLength = 1;
        baseStr = "0";
    } else if (flags & std::ios_base::hex) {
        base = 16;
        baseLength = 2;
        baseStr = uppercase ? "0X" : "0x";
    }

    char buffer[128 / 3 + 2];
    const auto p = _u128toa_impl(
            &value, buffer, sizeof(buffer), base, uppercase, (flags & std::ios_base::showpos) != 0);
    const auto length = (buffer + sizeof(buffer) - 1) - p;
    const auto lengthWithBase = length + baseLength;
    const auto fillLength = static_cast<std::streamsize>(lengthWithBase) >= os.width()
                                    ? 0
                                    : os.width() - lengthWithBase;
    if (flags & std::ios_base::internal) {
        if ((flags & std::ios_base::showbase) && baseStr) {
            os << baseStr;
            if (!os) return os;
        }
        if (fillLength > 0) {
            std::fill_n(std::ostream_iterator<char>(os), fillLength, os.fill());
            if (!os) return os;
        }
        os << p;
        if (!os) return os;
    } else if (flags & std::ios_base::right) {
        if ((flags & std::ios_base::showbase) && baseStr) {
            os << baseStr;
            if (!os) return os;
        }
        os << p;
        if (!os) return os;
        if (fillLength > 0) {
            std::fill_n(std::ostream_iterator<char>(os), fillLength, os.fill());
            if (!os) return os;
        }
    } else {
        if (fillLength > 0) {
            std::fill_n(std::ostream_iterator<char>(os), fillLength, os.fill());
            if (!os) return os;
        }
        if ((flags & std::ios_base::showbase) && baseStr) {
            os << baseStr;
            if (!os) return os;
        }
        os << buffer;
        if (!os) return os;
    }

    if (flags & std::ios_base::unitbuf) os.flush();
    return os;
}

}  // namespace stdext
