// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "File.h"

namespace siodb::iomgr::dbengine::io {

/** Provides encrypted binary file I/O. */
class NormalFile : public File {
public:
    DECLARE_NONCOPYABLE(NormalFile);

    /**
     * Initializes object of class NormalFile. Creates new file.
     * @param path File path.
     * @param extraFlags Additional open flags.
     * @param createMode File creation mode.
     * @param initialSize Initial file size.
     * @throw std::system_error if creating new file fails.
     */
    NormalFile(const std::string& path, int extraFlags, int createMode, off_t initialSize = 0);

    /**
     * Initializes object of class NormalFile. Open existing file.
     * @param path File path.
     * @param extraFlags Additional create flags.
     * @throw std::system_error if opening existing file fails.
     */
    NormalFile(const std::string& path, int extraFlags);

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
    std::size_t read(std::uint8_t* buffer, std::size_t size, off_t offset) noexcept override;

    /**
     * Writes specified amount of data to file starting at a give offset.
     * If write() system call succeeds but writes less then specified, next attempts are taken
     * to write subsequent portion of data, until specified number of bytes written.
     * @param buffer A buffer with data.
     * @param size Data size.
     * @param offset Starting offset.
     * @return Number of bytes known to be written successfully. Value less than requested
     *         indicates error. In such case, getLastError() will return an error code.
     */
    std::size_t write(const std::uint8_t* buffer, std::size_t size, off_t offset) noexcept override;

    /**
     * Returns current file size.
     * @return File size or -1 on error. In the case of failure,
     *         getLastError() will return an error code.
     */
    off_t getFileSize() noexcept override;

    /**
     * Provides file statistics.
     * @param st Statistics structure.
     * @return true if operation suceeded, false otherwise.
     */
    bool stat(struct stat& st) noexcept override;

    /**
     * Extends file by a length bytes.
     * @param length Number of bytes to allocate.
     * @return true if operation suceeded, false otherwise. In the case of failure,
     *         getLastError() will return an error code.
     */
    bool extend(off_t length) noexcept override;
};

}  // namespace siodb::iomgr::dbengine::io
