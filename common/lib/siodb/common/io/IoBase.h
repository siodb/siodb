// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstddef>

// System headers
#include <sys/types.h>

namespace siodb::io {

/**
 * A base class for input/output operations.
 */
class IoBase {
public:
    /**
     * Deinitializes object of class FdIo.
     */
    virtual ~IoBase() = default;

    /**
     * Reads data from input.
     * @param data Data.
     * @param size Size of data in bytes.
     * @return Count of read bytes.
     */
    virtual std::size_t read(void* buffer, std::size_t size) = 0;

    /**
     * Writes data to output.
     * @param data Data.
     * @param size Size of data in bytes.
     * @return Count of written bytes.
     */
    virtual std::size_t write(const void* buffer, std::size_t size) = 0;

    /**
     * Skips data.
     * @param size Count of bytes to skip.
     * @return Offset from the beggining of input. Negative value indicates error.
     */
    virtual off_t skip(std::size_t size) = 0;

    /**
     * Closes IO.
     * @return 0 in case of success.
     */
    virtual int close() = 0;

    /**
     * Returns indication whether IO is valid.
     * @return true means valid IO, false otherwise.
     */
    virtual bool isValid() const = 0;
};

}  // namespace siodb::io
