// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "NormalFile.h"

// Common project headers
#include <siodb/common/io/FileIO.h>

// CRT headers
#include <cerrno>
#include <cstring>

// STL headers
#include <sstream>

namespace siodb::iomgr::dbengine::io {

NormalFile::NormalFile(const std::string& path, int extraFlags, int createMode, off_t initialSize)
    : File(path, extraFlags, createMode, initialSize)
{
}

NormalFile::NormalFile(const std::string& path, int extraFlags)
    : File(path, extraFlags)
{
}

std::size_t NormalFile::read(std::uint8_t* buffer, std::size_t size, off_t offset) noexcept
{
    const auto res = ::preadExact(m_fd.getFD(), buffer, size, offset, kIgnoreSignals);
    if (res != size) m_lastError = errno;
    return res;
}

std::size_t NormalFile::write(const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept
{
    const auto res = ::pwriteExact(m_fd.getFD(), buffer, size, offset, kIgnoreSignals);
    if (res != size) m_lastError = errno;
    return res;
}

off_t NormalFile::getFileSize() noexcept
{
    return getRawFileSize();
}

bool NormalFile::stat(struct stat& st) noexcept
{
    if (::fstat(m_fd.getFD(), &st) == 0) return true;
    m_lastError = errno;
    return false;
}

bool NormalFile::extend(off_t length) noexcept
{
    struct stat st;
    if (::fstat(m_fd.getFD(), &st) == 0
            && ::posixFileAllocateExact(m_fd.getFD(), st.st_size, length) == 0)
        return true;
    m_lastError = errno;
    return false;
}

}  // namespace siodb::iomgr::dbengine::io
