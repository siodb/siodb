// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "FdDevice.h"

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

FdDevice::FdDevice(int fd, bool autoClose) noexcept
    : m_fd(fd)
    , m_autoClose(autoClose)
{
}

FdDevice::FdDevice(FdDevice&& src) noexcept
    : m_fd(src.m_fd)
    , m_autoClose(src.m_autoClose)
{
    src.m_fd = -1;
    src.m_autoClose = false;
}

FdDevice::~FdDevice()
{
    if (m_autoClose && isValid()) close();
}

FdDevice& FdDevice::operator=(FdDevice&& src) noexcept
{
    if (!isValid()) close();
    m_fd = src.m_fd;
    m_autoClose = src.m_autoClose;
    src.m_fd = -1;
    src.m_autoClose = false;
    return *this;
}

bool FdDevice::isValid() const
{
    return m_fd >= 0;
}

std::ptrdiff_t FdDevice::read(void* buffer, std::size_t size)
{
    return ::read(m_fd, buffer, size);
}

std::ptrdiff_t FdDevice::write(const void* buffer, std::size_t size)
{
    return ::write(m_fd, buffer, size);
}

off_t FdDevice::skip(std::size_t size)
{
    return ::lseek(m_fd, size, SEEK_CUR);
}

int FdDevice::close()
{
    if (isValid()) return doClose();
    throw std::runtime_error("Invalid file descriptor");
}

void FdDevice::swap(FdDevice& other) noexcept
{
    if (SIODB_LIKELY(&other != this)) {
        std::swap(m_fd, other.m_fd);
        std::swap(m_autoClose, other.m_autoClose);
    }
}

int FdDevice::doClose() noexcept
{
    const auto result = closeFileIgnoreSignal(m_fd);
    m_fd = -1;
    return result;
}

}  // namespace siodb::io
