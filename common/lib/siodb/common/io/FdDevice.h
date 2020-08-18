// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IODevice.h"
#include "../utils/HelperMacros.h"

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
     * Initializes object of class FdDevice from another object.
     * @param src Source object.
     */
    FdDevice(FdDevice&& src) noexcept;

    /** De-initializes object of class FdDevice. */
    ~FdDevice();

    DECLARE_NONCOPYABLE(FdDevice);

    /**
     * Move assignment operator.
     * @param src Source object.
     * @return This object.
     */
    FdDevice& operator=(FdDevice&& src) noexcept;

    /**
     * Returns file descriptor.
     * @return File descriptor.
     */
    int getFd() const noexcept
    {
        return m_fd;
    }

    /**
     * Returns automatic file descriptor close flag.
     * @return Automatic file descriptor close flag.
     */
    bool isAutoClose() const noexcept
    {
        return m_autoClose;
    }

    /**
     * Sets automatic file descriptor close flag.
     * @param autoClose automatic file descriptor close flag.
     */
    void setAutoClose(bool autoClose = true) noexcept
    {
        m_autoClose = autoClose;
    }

    /**
     * Returns indication that file descriptor is valid.
     * @return true device if file descriptor is valid, false otherwise.
     */
    bool isValid() const override;

    /**
     * Reads data from file.
     * @param buffer Data buffer.
     * @param size Data size in bytes.
     * @return Number of read bytes. Negative value indicates error.
     */
    std::ptrdiff_t read(void* buffer, std::size_t size) override;

    /**
     * Writes data to file.
     * @param buffer Data buffer.
     * @param size Size of data in bytes.
     * @return Number of written bytes. Negative value indicates error.
     */
    std::ptrdiff_t write(const void* buffer, std::size_t size) override;

    /**
     * Skips data.
     * @param size Number of bytes to skip.
     * @return Offset from the beginning of file. Negative value indicates error.
     */
    off_t skip(std::size_t size) override;

    /**
     * Closes file descriptor.
     * @return Zero on success, nonzero otherwise.
     */
    int close() override;

    /**
     * Swaps content with other object.
     * @param other Other object.
     */
    void swap(FdDevice& other) noexcept;

private:
    /**
     * Closes file descriptor.
     * @return Zero on success, nonzero otherwise.
     */
    int doClose() noexcept;

private:
    /** File descriptor */
    int m_fd;

    /** Automatic file descriptor close flag */
    bool m_autoClose;
};

/**
 * Swaps content of two device objects.
 * @param a First object.
 * @param b Second object.
 */
inline void swap(FdDevice& a, FdDevice& b) noexcept
{
    a.swap(b);
}

}  // namespace siodb::io
