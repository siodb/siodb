// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "InputOutputStream.h"

namespace siodb::io {

/** File descriptor input/output */
class FDStream final : public InputOutputStream {
public:
    /**
     * Initializes object of class FDStream.
     * @param fd File descriptor.
     * @param autoClose Auto close file on destructor.
     */
    explicit FDStream(int fd, bool autoClose = false) noexcept;

    /**
     * Initializes object of class FDStream from another object.
     * @param src Source object.
     */
    FDStream(FDStream&& src) noexcept;

    /** De-initializes object of class FDStream. */
    ~FDStream();

    /**
     * Move assignment operator.
     * @param src Source object.
     * @return This object.
     */
    FDStream& operator=(FDStream&& src) noexcept;

    /**
     * Returns file descriptor.
     * @return File descriptor.
     */
    int getFD() const noexcept
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
     * @return true stream if file descriptor is valid, false otherwise.
     */
    bool isValid() const noexcept override;

    /**
     * Reads data from file.
     * @param buffer Data buffer.
     * @param size Data size in bytes.
     * @return Number of read bytes. Negative value indicates error.
     */
    std::ptrdiff_t read(void* buffer, std::size_t size) noexcept override;

    /**
     * Writes data to file.
     * @param buffer Data buffer.
     * @param size Size of data in bytes.
     * @return Number of written bytes. Negative value indicates error.
     */
    std::ptrdiff_t write(const void* buffer, std::size_t size) noexcept override;

    /**
     * Skips data.
     * @param size Number of bytes to skip.
     * @return Number of bytes skipped. Negative value indicates error.
     */
    std::ptrdiff_t skip(std::size_t size) noexcept override;

    /**
     * Closes file descriptor.
     * @return Zero on success, nonzero otherwise.
     */
    int close() noexcept override;

    /**
     * Swaps content with other object.
     * @param other Other object.
     */
    void swap(FDStream& other) noexcept;

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
 * Swaps content of two stream objects.
 * @param a First object.
 * @param b Second object.
 */
inline void swap(FDStream& a, FDStream& b) noexcept
{
    a.swap(b);
}

}  // namespace siodb::io
