// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstddef>
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** Base class for all LOB streams. */
class LobStream {
protected:
    /**
     * Initializes object of class LobStream.
     * @param size Stream size.
     */
    explicit LobStream(std::uint32_t size) noexcept
        : m_size(size)
        , m_pos(0)
    {
    }

public:
    /** De-initializes object of class LobStream */
    virtual ~LobStream() = default;

    /**
     * Creates copy of this stream.
     * @return Copy of this stream object.
     * @throw std::logic_error if stream cannot be cloned.
     */
    virtual LobStream* clone() const = 0;

    /**
     * Returns stream size.
     * @return Stream size.
     */
    std::uint32_t getSize() const noexcept
    {
        return m_size;
    }

    /**
     * Returns current stream position.
     * @return Stream position.
     */
    std::uint32_t getPos() const noexcept
    {
        return m_pos;
    }

    /**
     * Returns number of remaining bytes in the stream.
     * @return Number of remaining bytes in the stream.
     */
    std::uint32_t getRemainingSize() const noexcept
    {
        return m_size - m_pos;
    }

    /**
     * Reads data from stream up to given data size.
     * @param buffer Destinaton buffer.
     * @param bufferSize Buffer size.
     * @return -1 if error occurred, 0 if EOF reached, otherwise data size read actually.
     */
    virtual std::ptrdiff_t read(void* buffer, std::size_t bufferSize) = 0;

    /**
     * Rewinds stream to beginning.
     * @return true if stream was rewind, false if stream doesn't support rewinding.
     */
    virtual bool rewind() = 0;

protected:
    /**
     * Trivial rewind implementation.
     */
    void trivialRewind() noexcept
    {
        m_pos = 0;
    }

    /** Stream size */
    std::uint32_t m_size;

    /** Stream position */
    std::uint32_t m_pos;
};

}  // namespace siodb::iomgr::dbengine
