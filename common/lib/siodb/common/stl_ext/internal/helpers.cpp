// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "helpers.h"

// CRT headers
#include <cstdio>
#include <cstring>

// STL headers
#include <memory>

namespace stdext::detail {

[[noreturn]] void throwOutOfRangeError(const char* prefix, std::size_t n, std::size_t limit)
{
    const auto bufferSize = std::strlen(prefix) + 128;
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    std::snprintf(buffer.get(), bufferSize, "%s: got %zu but limit is %zu", prefix, n, limit);
    throw std::out_of_range(buffer.get());
}

[[noreturn]] void throwLengthError(const char* prefix, std::size_t n, std::size_t limit)
{
    const auto bufferSize = std::strlen(prefix) + 128;
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    std::snprintf(buffer.get(), bufferSize, "%s: got %zu but limit is %zu", prefix, n, limit);
    throw std::out_of_range(buffer.get());
}

}  // namespace stdext::detail
