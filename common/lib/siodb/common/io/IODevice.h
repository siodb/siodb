// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstddef>

// System headers
#include <sys/types.h>

namespace siodb::io {

/** Common interface for the IO device classes. */
class IODevice {
public:
    /** De-initializes object of class IODevice. */
    virtual ~IODevice() = default;

    /**
     * Returns indication that device is valid.
     * @return true device if device is valid, false otherwise.
     */
    virtual bool isValid() const = 0;

    /**
     * Reads data from device.
     * @param buffer Data buffer.
     * @param size Size of data in bytes.
     * @return Number of read bytes. Negative value indicates error.
     */
    virtual std::ptrdiff_t read(void* buffer, std::size_t size) = 0;

    /**
     * Writes data to device.
     * @param buffer Data buffer.
     * @param size Data size in bytes.
     * @return Number of written bytes. Negative value indicates error.
     */
    virtual std::ptrdiff_t write(const void* buffer, std::size_t size) = 0;

    /**
     * Skips data.
     * @param size Number of bytes to skip.
     * @return Offset from the beginning of device. Negative value indicates error.
     */
    virtual off_t skip(std::size_t size) = 0;

    /**
     * Closes device.
     * @return Zero on success, nonzero otherwise.
     */
    virtual int close() = 0;
};

}  // namespace siodb::io
