// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// STL headers
#include <array>

namespace stdext {

/**
 * Bit mask with per bit access and access to an internal storage.
 * @tparam MinCapacity Minimum number of bits that this bitmask is capable to store.
 *                     Actual capacity may be equal or higher and can be received via bit_size().
 */
template<std::size_t MinCapacity>
class fixed_bitmask {
public:
    /** Number of bytes in the storage */
    static constexpr std::size_t static_size =
            (MinCapacity / 8) + (MinCapacity % 8 == 0 ? 0 : 1) + (MinCapacity == 0 ? 1 : 0);

public:
    /**
     * Initializes object of class bitmask.
     * @param bit_size Initial size of bitmask in bits.
     * @param value Value of the initial bits.
     */
    explicit fixed_bitmask(bool value = false) noexcept
    {
        fill(value);
    }

    /**
     * Returns value of a specified bit.
     * @param pos Bit position.
     * @return Value of a specified bit.
     * @throw std::out_of_range if bit index is out of range.
     */
    bool get(std::size_t pos) const
    {
        return (m_data.at(pos / 8) & (1 << (pos % 8))) != 0;
    }

    /**
     * Sets value of a specified bit.
     * @param pos Bit position.
     * @param value Bit value.
     * @throw std::out_of_range if bit index is out of range.
     */
    void set(std::size_t pos, bool value = true)
    {
        const auto byte_pos = pos / 8;
        const auto bit_pos = pos % 8;
        if (value)
            m_data.at(byte_pos) |= (1 << bit_pos);
        else
            m_data.at(byte_pos) &= ~(1 << bit_pos);
    }

    /**
     * Sets value of a specified bit to 0.
     * @param pos Bit position.
     * @throw std::out_of_range if bit index is out of range.
     */
    void reset(std::size_t pos)
    {
        const auto byte_pos = pos / 8;
        const auto bit_pos = pos % 8;
        m_data.at(byte_pos) &= ~(1 << bit_pos);
    }

    /**
     * Returns mutable data storage.
     * @return Data storage.
     */
    std::uint8_t* data() noexcept
    {
        return m_data.data();
    }

    /**
     * Returns constant data storage.
     * @return Data storage.
     */
    const std::uint8_t* data() const noexcept
    {
        return m_data.data();
    }

    /**
     * Returns size of the bitmask in bits.
     * @return Size of the bitmask in bits.
     */
    std::size_t bit_size() const noexcept
    {
        return m_data.size() * 8;
    }

    /**
     * Returns size of the bitmask in bytes.
     * @return Size of the bitmask in bytes.
     */
    std::size_t size() const noexcept
    {
        return m_data.size();
    }

    /**
     * Fills all bits in the bitmask with a specified value.
     * @param value A value.
     */
    void fill(bool value) noexcept
    {
        std::fill(m_data.begin(), m_data.end(), value ? 0xFF : 0);
    }

    /**
     * Swaps this bitmask with another one.
     * @param other Another bitmask.
     */
    void swap(fixed_bitmask& other) noexcept
    {
        m_data.swap(other.m_data);
    }

    /**
     * Equality operator.
     * @param other Another bitmask.
     * @return true if this and other bitmasks are equal, false otherwise.
     */
    bool operator==(const fixed_bitmask& other) const noexcept
    {
        return m_data == other.m_data;
    }

    /**
     * Inquality operator.
     * @param other Another bitmask.
     * @return true if this and other bitmasks are equal, false otherwise.
     */
    bool operator!=(const fixed_bitmask& other) const noexcept
    {
        return m_data != other.m_data;
    }

private:
    /* Bit storage. */
    std::array<std::uint8_t, static_size> m_data;
};

/**
 * Swaps two fixed bitmasks of the same minimal capacity.
 * @tparam MinCapacity Minimum number of bits that a bitmask is capable to store.
 * @param a First bitmask.
 * @param b Second bitmask.
 */
template<std::size_t MinCapacity>
inline void swap(fixed_bitmask<MinCapacity>& a, fixed_bitmask<MinCapacity>& b) noexcept
{
    a.swap(b);
}

}  // namespace stdext
