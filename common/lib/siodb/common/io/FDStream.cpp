// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "FDStream.h"

// Project headers
#include "FileIO.h"

// Common project headers
#include "../crt_ext/compiler_defs.h"

// STL headers
#include <stdexcept>
#include <utility>

// System headers
#include <fcntl.h>
#include <unistd.h>

namespace siodb::io {

FDStream::FDStream(int fd, bool autoClose) noexcept
    : m_fd(fd)
    , m_autoClose(autoClose)
{
}

FDStream::FDStream(FDStream&& src) noexcept
    : m_fd(src.m_fd)
    , m_autoClose(src.m_autoClose)
{
    src.m_fd = -1;
    src.m_autoClose = false;
}

FDStream::~FDStream()
{
    if (m_autoClose && isValid()) close();
}

FDStream& FDStream::operator=(FDStream&& src) noexcept
{
    if (isValid()) close();
    m_fd = src.m_fd;
    m_autoClose = src.m_autoClose;
    src.m_fd = -1;
    src.m_autoClose = false;
    return *this;
}

bool FDStream::isValid() const noexcept
{
    return m_fd >= 0;
}

std::ptrdiff_t FDStream::read(void* buffer, std::size_t size)
{
    // IMPORTANT: Use exactly read() system call
    return ::read(m_fd, buffer, size);
}

std::ptrdiff_t FDStream::write(const void* buffer, std::size_t size)
{
    // IMPORTANT: Use exactly writeExact()
    return ::writeExact(m_fd, buffer, size, kIgnoreSignals);
}

std::ptrdiff_t FDStream::skip(std::size_t size)
{
    if (::lseek(m_fd, size, SEEK_CUR) >= 0) return size;
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

int FDStream::close()
{
    if (isValid()) return doClose();
    return -1;
}

void FDStream::swap(FDStream& other) noexcept
{
    if (SIODB_LIKELY(&other != this)) {
        std::swap(m_fd, other.m_fd);
        std::swap(m_autoClose, other.m_autoClose);
    }
}

// ----- internals -----

int FDStream::doClose() noexcept
{
    const auto result = closeFileIgnoreSignal(m_fd);
    m_fd = -1;
    return result;
}

}  // namespace siodb::io
