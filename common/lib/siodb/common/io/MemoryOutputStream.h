// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "OutputStream.h"

namespace siodb::io {

class MemoryOutputStream : public OutputStream {
public:
    /**
     * Initializes object of class MemoryOutputStream.
     * @param buffer Memory buffer address.
     * @param size Data size.
     */
    MemoryOutputStream(void* buffer, std::size_t size) noexcept
        : m_current(buffer)
        , m_remaining(size)
    {
    }

    DECLARE_NONCOPYABLE(MemoryOutputStream);

    /**
     * Returns number of remaining unused bytes in the buffer.
     * @return Number of unused bytes in the buffer.
     */
    std::size_t getRemaining() const noexcept
    {
        return m_remaining;
    }

    /**
     * Returns indication that stream is valid.
     * @return true stream if stream is valid, false otherwise.
     */
    bool isValid() const noexcept override;

    /**
     * Closes stream.
     * @return Zero on success, nonzero otherwise.
     */
    int close() noexcept override;

    /**
     * Writes data to stream.
     * @param buffer Data buffer.
     * @param size Size of data in bytes.
     * @return Number of written bytes. Negative value indicates error.
     */
    std::ptrdiff_t write(const void* buffer, std::size_t size) noexcept override;

private:
    /** Current buffer address */
    void* m_current;

    /** Remaining size */
    std::size_t m_remaining;
};

}  // namespace siodb::io
