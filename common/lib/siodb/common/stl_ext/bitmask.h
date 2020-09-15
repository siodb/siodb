// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// STL headers
#include <vector>

namespace stdext {

/** Bit mask with per bit access and access to an internal storage. */
class bitmask {
public:
    /** Initializes object of class bitmask. */
    bitmask() noexcept = default;

    /**
     * Initializes object of class bitmask.
     * @param bit_size Initial size of bitmask in bits.
     * @param value Value of the initial bits.
     */
    explicit bitmask(std::size_t bit_size, bool value = false)
        : m_data(required_size(bit_size), value ? 0xFF : 0)
    {
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
     * Resizes bitmask to a minimum number of bytes that can hold specified number of bits.
     * @param bit_size New size in bits.
     * @param value Value for a new bits, false by default.
     */
    void resize(std::size_t bit_size, bool value = false)
    {
        m_data.resize(required_size(bit_size), value ? 0xFF : 0);
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
    void swap(bitmask& other) noexcept
    {
        m_data.swap(other.m_data);
    }

private:
    /**
     * Returns minimal number bytes that can hold specified number of bits.
     * @param n Number of bits.
     * @return Number of bytes.
     */
    static constexpr std::size_t required_size(std::size_t n) noexcept
    {
        return (n / 8) + ((n % 8 != 0) ? 1 : 0);
    }

private:
    /* Bit storage. */
    std::vector<std::uint8_t> m_data;
};

/**
  * Swaps two bitmasks.
  * @param a First bitmask.
  * @param b Second bitmask.
  */
inline void swap(bitmask& a, bitmask& b) noexcept
{
    a.swap(b);
}

}  // namespace stdext
