// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BufferedOutputStream.h"

// CRT headers
#include <cerrno>

namespace siodb::io {

BufferedOutputStream::BufferedOutputStream(std::size_t bufferSize, OutputStream& out)
    : OutputStreamWrapperStream(out)
    , m_buffer(bufferSize)
    , m_dataSize(0)
{
}

BufferedOutputStream::~BufferedOutputStream()
{
    close();
}

int BufferedOutputStream::close() noexcept
{
    int res = -1;
    if (m_out) {
        res = flush();
        m_out = nullptr;
    } else {
        errno = EIO;
    }
    return res;
}

std::ptrdiff_t BufferedOutputStream::write(const void* buffer, std::size_t size) noexcept
{
    if (!isValid()) {
        errno = EIO;
        return -1;
    }
    const auto bufferSize = m_buffer.size();
    auto remaining = size;
    const auto freeBufferSize = bufferSize - m_dataSize;
    if (freeBufferSize > 0) {
        const auto n = std::min(freeBufferSize, remaining);
        std::memcpy(m_buffer.data() + m_dataSize, buffer, n);
        m_dataSize += n;
        if (remaining == n) return size;
        remaining -= n;
        buffer = static_cast<const std::uint8_t*>(buffer) + n;
    }

    if (flush() < 0) return size - remaining;

    while (remaining > bufferSize) {
        if (onFlush(bufferSize) < 0) return size - remaining;
        const auto n = m_out->write(buffer, bufferSize);
        if (n < 0) return size - remaining;
        remaining -= n;
        buffer = static_cast<const std::uint8_t*>(buffer) + n;
    }

    if (remaining > 0) {
        std::memcpy(m_buffer.data(), buffer, remaining);
        m_dataSize = remaining;
        remaining = 0;
    }

    if (size > remaining) errno = EIO;
    return size - remaining;
}

std::ptrdiff_t BufferedOutputStream::flush() noexcept
{
    if (!isValid()) {
        errno = EIO;
        return -1;
    }
    if (m_dataSize == 0) return 0;
    if (onFlush(m_dataSize) < 0) return -1;
    const auto n = m_out->write(m_buffer.data(), m_dataSize);
    if (n >= 0) {
        if (static_cast<std::size_t>(n) == m_dataSize)
            m_dataSize = 0;
        else {
            const auto newDataSize = m_buffer.size() - n;
            std::memmove(m_buffer.data(), m_buffer.data() + n, newDataSize);
            m_dataSize = newDataSize;
        }
    }
    return n;
}

// ---- internals -----

int BufferedOutputStream::onFlush([[maybe_unused]] std::size_t dataSize) noexcept
{
    return 0;
}

}  // namespace siodb::io
