// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "InputStream.h"

// Common project headers
#include "../crt_ext/compiler_defs.h"

// CRT headers
#include <cstdint>

// STL headers
#include <algorithm>

namespace siodb::io {

std::ptrdiff_t InputStream::skip(std::size_t size) noexcept
{
    std::uint8_t buffer[4096];
    auto remaining = size;
    while (remaining > 0) {
        const auto bytesToSkip = std::min(remaining, sizeof(buffer));
        const auto n = read(buffer, bytesToSkip);
        if (SIODB_UNLIKELY(n < 1)) break;
        remaining -= n;
    }
    return size - remaining;
}

}  // namespace siodb::io
