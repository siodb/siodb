// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "FdDevice.h"

// Project headers
#include "FileIO.h"

// STL headers
#include <stdexcept>

// System headers
#include <fcntl.h>
#include <unistd.h>

namespace siodb::io {

FdDevice::FdDevice(int fd, bool autoClose) noexcept
    : m_fd(fd)
    , m_autoClose(autoClose)
{
}

FdDevice::~FdDevice()
{
    if (m_autoClose && isValid()) close();
}

std::size_t FdDevice::read(void* buffer, std::size_t size)
{
    return ::read(m_fd, buffer, size);
}

std::size_t FdDevice::write(const void* buffer, std::size_t size)
{
    return ::write(m_fd, buffer, size);
}

off_t FdDevice::skip(std::size_t size)
{
    return ::lseek(m_fd, size, SEEK_CUR);
}

int FdDevice::close()
{
    if (!isValid()) throw std::runtime_error("Invalid file descriptor");

    const auto result = closeFileIgnoreSignal(m_fd);
    m_fd = -1;
    return result;
}

bool FdDevice::isValid() const
{
    return m_fd > 0;
}

}  // namespace siodb::io
