// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ChunkedInputStream.h"

// Common project headers
#include "../crt_ext/compiler_defs.h"
#include "../utils/Base128VariantEncoding.h"

// STL headers
#include <limits>

namespace siodb::io {

std::ptrdiff_t ChunkedInputStream::read(void* buffer, std::size_t size) noexcept
{
    if (!isValid()) return -1;

    if (!m_hasChunkSize) {
        const int res = readChunkSize();
        if (SIODB_UNLIKELY(res < 1)) {
            if (res < 0) m_in = nullptr;
            return res;
        }
    }

    if (SIODB_UNLIKELY(m_eof)) return -1;

    std::size_t remaining = size;
    while (remaining > 0) {
        const auto bytesToRead = std::min(remaining, m_chunkSize - m_pos);
        if (bytesToRead == 0) break;

        const auto n = m_in->read(buffer, bytesToRead);
        if (SIODB_UNLIKELY(n < 1)) {
            m_in = nullptr;
            return -1;
        }

        buffer = static_cast<std::uint8_t*>(buffer) + n;
        remaining -= n;
        m_pos += n;

        if (m_pos == m_chunkSize) {
            m_hasChunkSize = false;
            break;
        }
    }
    return size - remaining;
}

std::ptrdiff_t ChunkedInputStream::skip(std::size_t size) noexcept
{
    if (!isValid()) return -1;

    if (!m_hasChunkSize) {
        const int res = readChunkSize();
        if (SIODB_UNLIKELY(res < 1)) {
            if (res < 0) {
                errno = EIO;
                m_in = nullptr;
            }
            return res;
        }
    }

    if (SIODB_UNLIKELY(m_eof)) return -1;

    std::size_t remaining = size;
    while (remaining > 0) {
        const auto bytesToSkip = std::min(remaining, m_chunkSize - m_pos);
        if (bytesToSkip == 0) break;

        const auto n = m_in->skip(bytesToSkip);
        if (SIODB_UNLIKELY(n < 1)) {
            m_in = nullptr;
            return -1;
        }

        remaining -= n;
        m_pos += n;

        if (m_pos == m_chunkSize) {
            const int res = readChunkSize();
            if (SIODB_UNLIKELY(res < 1)) {
                if (res < 0) m_in = nullptr;
                return res;
            }
        }
    }
    return size - remaining;
}

// ----- internals -----

int ChunkedInputStream::readChunkSize() noexcept
{
    std::uint8_t buffer[kMaxSerializedInt64Size];
    int i = 0;
    for (; i < kMaxSerializedInt64Size; ++i) {
        const auto n = m_in->read(buffer + i, 1);
        if (SIODB_UNLIKELY(n < 0)) {
            m_in = nullptr;
            return -1;
        }
        if (SIODB_UNLIKELY(n == 0)) {
            if (i > 0) {
                m_in = nullptr;
                errno = EIO;
                return -1;
            }
            m_chunkSize = 0;
            m_pos = 0;
            m_hasChunkSize = true;
            m_eof = true;
            return 0;
        }
        if ((buffer[i] & 0x80) == 0) {
            ++i;
            break;
        }
    }

    if (SIODB_UNLIKELY(i == kMaxSerializedInt64Size)) {
        m_in = nullptr;
        errno = EIO;
        return -1;
    }

    std::uint64_t chunkSize = 0;
    if (SIODB_UNLIKELY(::decodeVarUInt64(buffer, i, &chunkSize) < 1)) {
        m_in = nullptr;
        errno = EIO;
        return -1;
    }

    m_chunkSize = chunkSize;
    m_pos = 0;
    m_hasChunkSize = true;
    m_eof = (chunkSize == 0);
    return 1;
}

}  // namespace siodb::io
