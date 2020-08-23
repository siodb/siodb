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

namespace {
constexpr std::size_t kUndefinedChunkSize = std::numeric_limits<std::size_t>::max();
}  // namespace

ChunkedInputStream::ChunkedInputStream(InputStream& stream)
    : m_stream(&stream)
    , m_pos(0)
    , m_chunkSize(kUndefinedChunkSize)
{
}

bool ChunkedInputStream::isValid() const
{
    return m_stream && m_stream->isValid();
}

int ChunkedInputStream::close()
{
    if (m_stream) {
        m_stream = nullptr;
        return 0;
    }
    return -1;
}

std::ptrdiff_t ChunkedInputStream::read(void* buffer, std::size_t size)
{
    if (!isValid()) return -1;

    if (SIODB_UNLIKELY(m_chunkSize == kUndefinedChunkSize)) {
        const int res = readChunkSize();
        if (SIODB_UNLIKELY(res < 1)) {
            if (res < 0) m_stream = nullptr;
            return res;
        }
    }

    std::size_t remaining = size;
    while (remaining > 0) {
        const auto bytesToRead = std::min(remaining, m_chunkSize - m_pos);
        if (bytesToRead == 0) break;

        const auto n = m_stream->read(buffer, bytesToRead);
        if (SIODB_UNLIKELY(n < 1)) {
            m_stream = nullptr;
            return -1;
        }

        buffer = static_cast<std::uint8_t*>(buffer) + n;
        remaining -= n;
        m_pos += n;

        if (m_pos == m_chunkSize) {
            const int res = readChunkSize();
            if (SIODB_UNLIKELY(res < 1)) {
                if (res < 0) m_stream = nullptr;
                return res;
            }
        }
    }
    return size - remaining;
}

off_t ChunkedInputStream::skip(std::size_t size)
{
    if (!isValid()) return -1;

    if (SIODB_UNLIKELY(m_chunkSize == kUndefinedChunkSize)) {
        const int res = readChunkSize();
        if (SIODB_UNLIKELY(res < 1)) {
            if (res < 0) m_stream = nullptr;
            return res;
        }
    }

    std::size_t remaining = size;
    while (remaining > 0) {
        const auto bytesToSkip = std::min(remaining, m_chunkSize - m_pos);
        if (bytesToSkip == 0) break;

        const auto n = m_stream->skip(bytesToSkip);
        if (SIODB_UNLIKELY(n < 1)) {
            m_stream = nullptr;
            return -1;
        }

        remaining -= n;
        m_pos += n;

        if (m_pos == m_chunkSize) {
            const int res = readChunkSize();
            if (SIODB_UNLIKELY(res < 1)) {
                if (res < 0) m_stream = nullptr;
                return res;
            }
        }
    }
    return size - remaining;
}

// ----- internals -----

int ChunkedInputStream::readChunkSize()
{
    std::uint8_t buffer[5];
    int i = 0;
    for (; i < 5; ++i) {
        const auto n = m_stream->read(buffer + i, 1);
        if (SIODB_UNLIKELY(n < 0)) {
            m_stream = nullptr;
            return -1;
        }
        if (SIODB_UNLIKELY(n == 0)) {
            if (i > 0) {
                m_stream = nullptr;
                return -1;
            }
            m_chunkSize = 0;
            m_pos = 0;
            return 0;
        }
        if ((buffer[i] & 0x80) == 0) break;
    }

    if (SIODB_UNLIKELY(i == 5)) {
        m_stream = nullptr;
        return -1;
    }

    std::uint32_t chunkSize = 0;
    if (SIODB_UNLIKELY(::decodeVarUInt32(buffer, i, &chunkSize) < 1)) {
        m_stream = nullptr;
        return -1;
    }

    m_chunkSize = chunkSize;
    m_pos = 0;
    return 1;
}

}  // namespace siodb::io
