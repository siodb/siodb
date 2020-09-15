// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "OutputStream.h"

// Common project headers
#include "../stl_ext/buffer.h"

// CRT headers
#include <cstdint>

namespace siodb ::io {

class DynamicMemoryOutputStream : public OutputStream {
public:
    /** Default buffer initial size */
    static constexpr std::size_t kDefaultInitialSize = 4096;
    /** Default buffer grow step */
    static constexpr std::size_t kDefaultGrowStep = 4096;

public:
    /**
     * Initializes object of class DynamicMemoryOutputStream.
     * @param initialSize Initial size of the buffer.
     * @param growStep Data size.
     */
    explicit DynamicMemoryOutputStream(
            std::size_t initialSize = kDefaultInitialSize, std::size_t growStep = kDefaultGrowStep);

    DECLARE_NONCOPYABLE(DynamicMemoryOutputStream);

    /**
     * Returns pointer to the current data buffer.
     * @return Pointer to the current data buffer.
     */
    void* data() noexcept
    {
        return m_buffer.data();
    }

    /**
     * Returns pointer to the current data buffer.
     * @return Pointer to the current data buffer.
     */
    const void* data() const noexcept
    {
        return m_buffer.data();
    }

    /**
     * Returns current data size.
     * @return Current data size.
     */
    std::size_t size() const noexcept
    {
        return m_current - m_buffer.data();
    }

    /**
     * Returns current buffer capacity.
     * @return Current buffer capacity.
     */
    std::size_t capacity() const noexcept
    {
        return m_buffer.size();
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
    /** Extensible buffer */
    stdext::buffer<std::uint8_t> m_buffer;

    /** Buffer grow step */
    const std::size_t m_growStep;

    /** Current data size */
    std::size_t m_dataSize;

    /** Current buffer address */
    std::uint8_t* m_current;

    /** Remaining size */
    std::size_t m_remaining;
};

}  // namespace siodb::io
