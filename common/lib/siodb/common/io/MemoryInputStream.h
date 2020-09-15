// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "InputStream.h"

namespace siodb::io {

class MemoryInputStream : public InputStream {
public:
    /**
     * Initializes object of class MemoryInputStream.
     * @param buffer Memory buffer address.
     * @param size Data size.
     */
    MemoryInputStream(const void* buffer, std::size_t size) noexcept
        : m_current(buffer)
        , m_remaining(size)
    {
    }

    DECLARE_NONCOPYABLE(MemoryInputStream);

    /**
     * Returns number of remaining unread bytes in the buffer.
     * @return Number of unread bytes in the buffer.
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
     * Reads data from stream.
     * @param buffer Data buffer.
     * @param size Size of data in bytes.
     * @return Number of read bytes. Negative value indicates error.
     */
    std::ptrdiff_t read(void* buffer, std::size_t size) noexcept override;

    /**
     * Skips data.
     * @param size Number of bytes to skip.
     * @return Number of bytes skipped. Negative value indicates error.
     */
    std::ptrdiff_t skip(std::size_t size) noexcept override;

private:
    /** Current buffer address */
    const void* m_current;

    /** Remaining size */
    std::size_t m_remaining;
};

}  // namespace siodb::io
