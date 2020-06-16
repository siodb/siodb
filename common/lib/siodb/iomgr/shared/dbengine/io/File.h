// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/io/IOError.h>
#include <siodb/common/utils/FdGuard.h>
#include <siodb/common/utils/HelperMacros.h>

// CRT headers
#include <cstdint>

// STL headers
#include <memory>
#include <string>

// System headers
#include <sys/stat.h>
#include <sys/types.h>

namespace siodb::iomgr::dbengine::io {

/** Provides file I/O. */
class File {
protected:
    /**
     * Initializes object of class File. Creates new file.
     * @param path File path.
     * @param extraFlags Additional open flags.
     * @param createMode File creation mode.
     * @param initialSize Initial file size.
     * @throw std::system_error if creating new file fails.
     */
    File(const std::string& path, int extraFlags, int createMode, off_t initialSize = 0);

    /**
     * Initializes object of class File. Opens existing file.
     * @param path File path.
     * @param extraFlags Additional open flags.
     * @throw std::system_error if opening existing file fails.
     */
    File(const std::string& path, int extraFlags);

public:
    DECLARE_NONCOPYABLE(File);

    /** De-initializes object */
    virtual ~File() = default;

    /**
     * Returns file descriptor.
     * @return File descriptor.
     */
    int getFd() const noexcept
    {
        return m_fd.getFd();
    }

    /**
     * Returns last error code due to which last operation has failed.
     * @return Error code.
     */
    int getLastError() const noexcept
    {
        return m_lastError;
    }

    /**
     * Reads specified amount of data from file starting at a given offset.
     * If pread() system call succeeds but reads less then specified, next attempts are taken
     * to read subsequent portion of data until specified number of bytes is read.
     * @param[out] buffer A buffer for data.
     * @param size Desired data size.
     * @param offset Starting offset.
     * @return Number of bytes actually read. Values less than requested indicate error.
     *         In such case, getLastError() will return an error code. If error code is 0,
     *         then end of file is reached.
     */
    virtual std::size_t read(std::uint8_t* buffer, std::size_t size, off_t offset) noexcept = 0;

    /**
     * Reads specified amount of data from file starting at a given offset.
     * If pread() system call succeeds but reads less then specified, next attempts are taken
     * to read subsequent portion of data until specified number of bytes is read.
     * @param[out] buffer A buffer for data.
     * @param size Desired data size.
     * @param offset Starting offset.
     * @throw siodb::FileReadError if read less than requeted.
     */
    void readChecked(std::uint8_t* buffer, std::size_t size, off_t offset);

    /**
     * Writes specified amount of data to file starting at a give offset.
     * If pwrite() system call succeeds but writes less then specified, next attempts are taken
     * to write subsequent portion of data, until specified number of bytes written.
     * @param buffer A buffer with data.
     * @param size Data size.
     * @param offset Starting offset.
     * @return Number of bytes known to be written successfully. Value less than requested
     *         indicates error. In such case, getLastError() will return an error code.
     */
    virtual std::size_t write(
            const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept = 0;

    /**
     * Writes specified amount of data to file starting at a give offset.
     * If pwrite() system call succeeds but writes less then specified, next attempts are taken
     * to write subsequent portion of data, until specified number of bytes written.
     * @param buffer A buffer with data.
     * @param size Data size.
     * @param offset Starting offset.
     * @throw siodb::FileWriteError if not all data written.
     */
    void writeChecked(const std::uint8_t* buffer, std::size_t size, off_t offset);

    /**
     * Returns current file size. This may differ from real on-disk file size.
     * @return File size or -1 on error. In the case of failure,
     *         getLastError() will return an error code.
     */
    virtual off_t getFileSize() noexcept = 0;

    /**
     * Returns current size of the raw on-disk file.
     * @return File size or -1 on error. In the case of failure,
     *         getLastError() will return an error code.
     */
    off_t getRawFileSize() noexcept;

    /**
     * Provides file statistics.
     * @param st Statistics structure.
     * @return true if operation suceeded, false otherwise. In the case of failure,
     *         getLastError() will return an error code.
     */
    virtual bool stat(struct stat& st) noexcept = 0;

    /**
     * Extends file by a length bytes.
     * @param length Number of bytes to allocate.
     * @return true if operation suceeded, false otherwise. In the case of failure,
     *         getLastError() will return an error code.
     */
    virtual bool extend(off_t length) noexcept = 0;

    /**
     * Flushes pending writes to disk.
     * @return true if operation succeeded, false otherwise. In the case of failure,
     *         getLastError() will return an error code.
     */
    bool flush() noexcept;

protected:
    /**
     * Validates given file descriptor.
     * @param fd File descriptor.
     * @param path File path.
     * @return Same fd, if it is valid.
     * @throw std::runtime_error if file descriptor is invlid.
     */
    static int validateFd(int fd, const std::string& path);

protected:
    /** File descriptor */
    FdGuard m_fd;

    /** Last I/O error code */
    int m_lastError;
};

/** Unique pointer shortcut type */
using FilePtr = std::unique_ptr<File>;

}  // namespace siodb::iomgr::dbengine::io
