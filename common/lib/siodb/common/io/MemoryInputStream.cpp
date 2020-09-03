// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "MemoryInputStream.h"

// Common project headers
#include "../crt_ext/compiler_defs.h"

// CRT headers
#include <cstring>

// STL headers
#include <algorithm>

namespace siodb::io {

MemoryInputStream::MemoryInputStream(const void* buffer, std::size_t size)
    : m_buffer(static_cast<const std::uint8_t*>(buffer))
    , m_remaining(size)
{
}

bool MemoryInputStream::isValid() const noexcept
{
    return m_buffer;
}

int MemoryInputStream::close()
{
    m_buffer = nullptr;
    m_remaining = 0;
    return 0;
}

std::ptrdiff_t MemoryInputStream::read(void* buffer, std::size_t size)
{
    if (SIODB_LIKELY(isValid())) {
        const auto n = std::min(size, m_remaining);
        if (SIODB_LIKELY(n > 0)) {
            std::memcpy(buffer, m_buffer, n);
            m_buffer += n;
            m_remaining -= n;
        }
        return n;
    }
    return -1;
}

std::ptrdiff_t MemoryInputStream::skip(std::size_t size)
{
    if (SIODB_LIKELY(isValid())) {
        const auto n = std::min(size, m_remaining);
        m_remaining -= n;
        return n;
    }
    return -1;
}

}  // namespace siodb::io
