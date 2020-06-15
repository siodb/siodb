// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headrs
#include <cstddef>

// Project headers
#include "../utils/FdGuard.h"

// System headers
#include <sys/mman.h>  // intentionally included to provide mmap constants
#include <sys/types.h>

namespace siodb::io {

/**
 * Manages mapping of a file region into memory.
 */
class MemoryMappedFile {
public:
    /**
     * Initializes object of class MemoryMappedFile.
     * @param path File path.
     * @param openFlags File opening flags.
     * @param mappingFlags Additional memory mapping flags.
     * @param offset Offset in file.
     * @param length Mapping length. Zero value indicates that entire file should be mapped.
     */
    MemoryMappedFile(const char* path, int openFlags, int mappingFlags, off_t offset,
            std::size_t length = 0);

    /**
     * Initializes object of class MemoryMappedFile.
     * @param fd File descriptor.
     * @param fdOwner Indicates that file descriptor should be owned by this object.
     * @param prot Memory protection mode.
     * @param mappingFlags Additional memory mapping flags.
     * @param offset Offset in file.
     * @param length Mapping length. Zero value indicates that entire file should be mapped.
     */
    MemoryMappedFile(
            int fd, bool fdOwner, int prot, int mappingFlags, off_t offset, std::size_t length = 0);

    /** Cleans up object of class MemoryMappedFile. */
    ~MemoryMappedFile();

    /** Copy construction disabled. */
    MemoryMappedFile(const MemoryMappedFile& src) = delete;

    /** Copy assignment disabled. */
    MemoryMappedFile& operator=(const MemoryMappedFile& src) = delete;

    /**
     * Returns mapping address.
     * @return Mapping address.
     */
    void* getMappingAddress() const noexcept
    {
        return m_mappingAddr;
    }

    /**
     * Returns mapping length.
     * @return Mapping legth.
     */
    std::size_t getMappingLength() const noexcept
    {
        return m_length;
    }

    /**
     * Deduces memory protection mode from file open flags.
     * @param int openFlags File open flags.
     * @return Protection mode.
     */
    static int deduceMemoryProtectionMode(int openFlags) noexcept;

private:
    /**
     * Returns length of the opened file.
     * @param fd File descriptor.
     * @return File length.
     * @throw std::runtime_error if file length could not be obtained.
     */
    static off_t getFileLength(int fd);

    /**
     * Checks that object was properly initialized.
     * @throw std::system_error if initialization error occurred
     */
    void checkInitialized() const;

private:
    /** File descriptor */
    const FdGuard m_fd;

    /** Mapping length */
    const std::size_t m_length;

    /** Mapping address */
    void* const m_mappingAddr;
};

}  // namespace siodb::io
