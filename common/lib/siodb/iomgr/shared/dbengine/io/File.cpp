// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "File.h"

// Project headers
#include <siodb/common/io/FileIO.h>

// CRT headers
#include <cerrno>
#include <cstring>

// STL headers
#include <sstream>
#include <system_error>

// System headers
#include <unistd.h>

namespace siodb::iomgr::dbengine::io {

File::File(const std::string& path, int extraFlags, int createMode, off_t initialSize)
    : m_fd(validateFd(
            ::open(path.c_str(),
                    ((extraFlags & O_TMPFILE) ? 0 : O_CREAT) | O_RDWR | O_CLOEXEC | extraFlags,
                    createMode),
            path))
    , m_lastError(0)
{
    if (initialSize > 0 && ::posixFileAllocateExact(m_fd.getFD(), 0, initialSize)) {
        const int errorCode = errno;
        std::ostringstream err;
        err << "Can't allocate " << (initialSize / 1024) << " KiB of disk space for the file "
            << path;
        throw std::system_error(errorCode, std::generic_category(), err.str());
    }
}

File::File(const std::string& path, int extraFlags)
    : m_fd(validateFd(::open(path.c_str(), O_RDWR | O_CLOEXEC | extraFlags), path))
    , m_lastError(0)
{
}

void File::readChecked(std::uint8_t* buffer, std::size_t size, off_t offset)
{
    const auto n = read(buffer, size, offset);
    if (n != size) throw FileReadError(m_lastError, std::strerror(m_lastError), n);
}

void File::writeChecked(const std::uint8_t* buffer, std::size_t size, off_t offset)
{
    const auto n = write(buffer, size, offset);
    if (n != size) throw FileWriteError(m_lastError, std::strerror(m_lastError), n);
}

off_t File::getRawFileSize() noexcept
{
    struct stat st;
    return stat(st) ? st.st_size : -1;
}

bool File::flush() noexcept
{
    if (::fdatasync(m_fd.getFD()) == 0) return true;
    m_lastError = errno;
    return false;
}

// ----- internals -----

int File::validateFd(int fd, const std::string& path)
{
    if (fd >= 0) return fd;
    const int errorCode = errno;
    std::ostringstream err;
    err << "Can't open file " << path;
    throw std::system_error(errorCode, std::generic_category(), err.str());
}

}  // namespace siodb::iomgr::dbengine::io
