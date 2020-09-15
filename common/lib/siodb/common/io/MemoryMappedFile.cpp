// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "MemoryMappedFile.h"

// CRT headers
#include <cerrno>

// STL headers
#include <system_error>

// System headers
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace siodb::io {

MemoryMappedFile::MemoryMappedFile(
        const char* path, int openFlags, int mappingFlags, off_t offset, std::size_t length)
    : m_fdGuard(::open(path, openFlags | FD_CLOEXEC))
    , m_fd(m_fdGuard.getFD())
    , m_length(m_fdGuard.isValidFd() ? ((length > 0) ? length : getFileLength(m_fd)) : 0)
    , m_mappingAddr(m_length > 0 ? ::mmap(nullptr, m_length, deduceMemoryProtectionMode(openFlags),
                            MAP_SHARED | mappingFlags, m_fd, offset)
                                 : nullptr)
{
    checkInitialized();
}

MemoryMappedFile::MemoryMappedFile(
        int fd, bool fdOwner, int prot, int mappingFlags, off_t offset, std::size_t length)
    : m_fdGuard(fdOwner ? fd : -1)
    , m_fd(fd)
    , m_length((fd < 0) ? 0 : ((length > 0) ? length : getFileLength(fd)))
    , m_mappingAddr(m_length > 0
                            ? ::mmap(nullptr, m_length, prot, MAP_SHARED | mappingFlags, fd, offset)
                            : nullptr)
{
    checkInitialized();
}

MemoryMappedFile::~MemoryMappedFile()
{
    if (m_mappingAddr) ::munmap(m_mappingAddr, m_length);
}

int MemoryMappedFile::deduceMemoryProtectionMode(int openFlags) noexcept
{
    if ((openFlags & O_RDWR) == O_RDWR) return PROT_READ | PROT_WRITE;
    return ((openFlags & O_WRONLY) == O_WRONLY) ? PROT_WRITE : PROT_READ;
}

off_t MemoryMappedFile::getFileLength(int fd)
{
    struct stat st;
    if (::fstat(fd, &st) < 0)
        throw std::system_error(std::error_code(errno, std::system_category()));
    return st.st_size;
}

void MemoryMappedFile::checkInitialized() const
{
    if (!m_mappingAddr && (m_fdGuard.getFD() < 0 || m_length > 0))
        throw std::system_error(std::error_code(errno, std::system_category()));
}

}  // namespace siodb::io
