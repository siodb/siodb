// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DynamicMemoryOutputStream.h"

// Common project headers
#include "../crt_ext/compiler_defs.h"

// CRT headers
#include <cstdint>
#include <cstring>

// STL headers
#include <algorithm>

namespace siodb ::io {

DynamicMemoryOutputStream::DynamicMemoryOutputStream(
        std::size_t initialSize, std::size_t growthStep)
    : m_buffer(initialSize)
    , m_growthStep(growthStep)
    , m_dataSize(0)
    , m_current(m_buffer.data())
    , m_remaining(m_buffer.size())
{
}

bool DynamicMemoryOutputStream::isValid() const noexcept
{
    return m_current != nullptr;
}

int DynamicMemoryOutputStream::close() noexcept
{
    m_current = nullptr;
    m_remaining = 0;
    return 0;
}

std::ptrdiff_t DynamicMemoryOutputStream::write(const void* buffer, std::size_t size) noexcept
{
    if (SIODB_LIKELY(isValid())) {
        if (size == 0) return 0;
        if (size > m_remaining && m_growthStep > 0) {
            auto n = size - m_remaining;
            const auto r = n % m_growthStep;
            if (r > 0) n += m_growthStep - r;
            try {
                const auto dataSize = this->size();
                m_buffer.resize(n);
                m_current = m_buffer.data() + dataSize;
                m_remaining = m_buffer.size() - dataSize;
            } catch (std::bad_alloc& ex) {
                // Skip this, write data only to the available extent
                errno = ENOMEM;
            }
        }

        if (SIODB_UNLIKELY(size == 0)) return 0;

        const auto n = std::min(size, m_remaining);
        if (SIODB_LIKELY(n > 0)) {
            std::memcpy(m_current, buffer, n);
            m_current = static_cast<std::uint8_t*>(m_current) + n;
            m_remaining -= n;
            return n;
        } else
            close();
    }
    errno = EIO;
    return -1;
}

}  // namespace siodb::io
