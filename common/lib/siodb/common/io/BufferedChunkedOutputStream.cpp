// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BufferedChunkedOutputStream.h"

// CRT headers
#include <cerrno>

namespace siodb::io {

BufferedChunkedOutputStream::BufferedChunkedOutputStream(
        std::size_t maxChunkSize, OutputStream& out)
    : BufferedOutputStream(validateMaxChunkSize(maxChunkSize), out)
{
}

BufferedChunkedOutputStream::~BufferedChunkedOutputStream()
{
    close();
}

int BufferedChunkedOutputStream::close() noexcept
{
    int res = -1;
    if (m_out) {
        const auto dataSize = m_dataSize;
        const auto written = flush();
        if (static_cast<std::size_t>(written) == dataSize) res = writeChunkSize(0);
        m_out = nullptr;
    } else {
        errno = EIO;
    }
    return res;
}

// ---- internals -----

int BufferedChunkedOutputStream::onFlush(std::size_t dataSize) noexcept
{
    return writeChunkSize(static_cast<std::uint32_t>(dataSize));
}

std::size_t BufferedChunkedOutputStream::validateMaxChunkSize(std::size_t maxChunkSize)
{
    if (maxChunkSize >= kMinMaxChunkSize && maxChunkSize <= kMaxMaxChunkSize) return maxChunkSize;
    throw std::invalid_argument("Invalid max chunk size: " + std::to_string(maxChunkSize));
}

}  // namespace siodb::io
