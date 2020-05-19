// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "FdIo.h"

// Project headers
#include "FileIO.h"

// STL headers
#include <stdexcept>

// System headers
#include <fcntl.h>
#include <unistd.h>

namespace siodb::io {

FdIo::FdIo(int fd, bool autoClose) noexcept
    : m_fd(fd)
    , m_autoClose(autoClose)
{
}

FdIo::~FdIo()
{
    if (m_autoClose && isValid()) close();
}

std::size_t FdIo::read(void* buffer, std::size_t size)
{
    return ::read(m_fd, buffer, size);
}

std::size_t FdIo::write(const void* buffer, std::size_t size)
{
    return ::write(m_fd, buffer, size);
}

off_t FdIo::skip(std::size_t size)
{
    return ::lseek(m_fd, size, SEEK_CUR);
}

int FdIo::close()
{
    if (!isValid()) throw std::runtime_error("Invalid file descriptor");

    const auto result = closeFileIgnoreSignal(m_fd);
    m_fd = -1;
    return result;
}

bool FdIo::isValid() const
{
    return m_fd > 0;
}

}  // namespace siodb::io