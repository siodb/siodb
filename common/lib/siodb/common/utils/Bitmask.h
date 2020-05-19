// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// STL headers
#include <vector>

namespace siodb::utils {

/** Bit mask with per bit access and access to an internal storage. */
class Bitmask {
public:
    /** Initializes object of class Bitmask. */
    Bitmask() noexcept = default;

    /**
     * Initializes object of class Bitmask.
     * @param bitSize Initial size of bitmask in bits.
     * @param value Value of the initial bits.
     */
    explicit Bitmask(std::size_t bitSize, bool value = false)
        : m_bytes(getByteSize(bitSize), value ? 0xFF : 0)
    {
    }

    /**
     * Returns value of a specified bit.
     * @param bit Bit index
     * @return Value of a specified bit.
     * @throw std::out_of_range if bit index is out of range.
     */
    bool getBit(std::size_t bit) const
    {
        return (m_bytes.at(bit / 8) & (1 << (bit % 8))) != 0;
    }

    /**
     * Sets value of a specified bit.
     * @param bit Bit index
     * @param value Bit value.
     * @throw std::out_of_range if bit index is out of range.
     */
    void setBit(std::size_t bit, bool value)
    {
        const auto byteIdx = bit / 8;
        const auto bitIdx = bit % 8;
        if (value) {
            m_bytes.at(byteIdx) |= (1 << bitIdx);
        } else {
            m_bytes.at(byteIdx) &= ~(1 << bitIdx);
        }
    }

    /**
     * Returns mutable data storage.
     * @return Data storage.
     */
    std::uint8_t* getData() noexcept
    {
        return m_bytes.data();
    }

    /**
     * Returns constant data storage.
     * @return Data storage.
     */
    const std::uint8_t* getData() const noexcept
    {
        return m_bytes.data();
    }

    /**
     * Returns size of bitmask in bits.
     * @return Size of bitmask in bits.
     */
    std::size_t getBitSize() const noexcept
    {
        return m_bytes.size() * 8;
    }

    /**
     * Returns size of bitmask in bytes.
     * @return Size if bitmask in bytes.
     */
    std::size_t getByteSize() const noexcept
    {
        return m_bytes.size();
    }

    /**
     * Resizes bitmask to a number of bytes that can hold specified number of bits.
     * @param bitSize New size in bits.
     * @param value Value for a new bits if any.
     */
    void resize(std::size_t bitSize, bool value = false)
    {
        m_bytes.resize(getByteSize(bitSize), value ? 0xFF : 0);
    }

    /**
     * Fills all bits in the bitmask with a specified value.
     * @param value A value.
     */
    void fill(bool value) noexcept
    {
        std::fill(m_bytes.begin(), m_bytes.end(), value ? 0xFF : 0);
    }

private:
    /**
     * Returns minimal size in bytes that can hold specified amount of bits.
     * @param bitSize Size in bits.
     * @return Size in bytes.
     */
    static constexpr std::size_t getByteSize(std::size_t bitSize)
    {
        return (bitSize / 8) + ((bitSize % 8 != 0) ? 1 : 0);
    }

private:
    /* Bit storage. */
    std::vector<std::uint8_t> m_bytes;
};

}  // namespace siodb::utils
