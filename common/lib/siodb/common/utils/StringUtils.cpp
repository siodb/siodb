// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "StringUtils.h"

// CRT headers
#include <cstring>

namespace siodb::utils {

std::string createString(std::size_t length, char fill)
{
    std::string s;
    if (length > 0) {
        s.resize(length);
        std::memset(s.data(), fill, length);
    }
    return s;
}

}  // namespace siodb::utils
