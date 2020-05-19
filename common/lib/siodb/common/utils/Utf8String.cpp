// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Utf8String.h"

#include <utf8cpp/utf8.h>

namespace siodb::utils {

int utf8_strcmp(const char* s1, std::size_t s1Len, const char* s2, std::size_t s2Len)
{
    const auto s1End = s1 + s1Len;
    const auto s2End = s2 + s2Len;
    while (true) {
        if (s1 == s1End) return (s2 == s2End) ? 0 : -1;
        if (s2 == s2End) return 1;
        const auto symb1 = utf8::next(s1, s1End);
        const auto symb2 = utf8::next(s2, s2End);
        if (symb1 < symb2)
            return -1;
        else if (symb1 > symb2)
            return 1;
    }
}

}  // namespace siodb::utils
