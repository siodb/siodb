// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IODevice.h"

namespace siodb::io {

/** File descriptor input/output */
class FdDevice final : public IODevice {
public:
    /**
     * Initializes object of class FdDevice.
     * @param fd File descriptor.
     * @param autoClose Auto close file on destructor.
     */
    explicit FdDevice(int fd, bool autoClose = false) noexcept;

    /**
     * Deinitializes object of class FdDevice.
     */
    ~FdDevice() override;

    /**
     * Reads data from secure channel.
     * @param data Data.
     * @param size Size of data in bytes.
     * @return Count of read bytes.
     */
    std::size_t read(void* buffer, std::size_t size) override;

    /**
     * Writes data to file descriptor.
     * @param data Data.
     * @param size Size of data in bytes.
     * @return Count of written bytes.
     */
    std::size_t write(const void* buffer, std::size_t size) override;

    /**
     * Skips data.
     * @param size Count of bytes to skip.
     * @return Offset from the beggining of file or -1 in case of error.
     */
    off_t skip(std::size_t size) override;

    /**
     * Closes file descriptor.
     * @return 0 in case of success, other value means error.
     */
    int close() override;

    /**
     * Returns indication whether connection is valid or not.
     * @return true if connection is valid false otherwise.
     */
    bool isValid() const override;

    /**
     * Returns file descriptor.
     * @return File descriptor.
     */
    int getFd() const noexcept
    {
        return m_fd;
    }

    /**
     * Sets automatic file descriptor close indication.
     * @param autoClose automatic file descriptor close indication.
     */
    void setAutoClose(bool autoClose) noexcept
    {
        m_autoClose = autoClose;
    }

    /**
     * Returns automatic file descriptor close indication.
     * @return Automatic file descriptor close indication.
     */
    bool isAutoClose() const noexcept
    {
        return m_autoClose;
    }

private:
    /** File descriptor */
    int m_fd;

    /** Automatic file descriptor close indication */
    bool m_autoClose;
};

}  // namespace siodb::io
