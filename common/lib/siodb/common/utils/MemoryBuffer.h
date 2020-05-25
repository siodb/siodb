// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../config/CompilerDefs.h"

// CRT headers
#include <cstring>

// STL headers
#include <memory>
#include <type_traits>

namespace siodb::utils {

/**
 * Memory buffer with elements of trivial type.
 * @tparam T Buffer element type.
 */
template<class T>
class MemoryBuffer {
    static_assert(std::is_trivial_v<T>,
            "Template argument T must be trivial type in the class siodb::utils::MemoryBuffer");

public:
    /**
     * Initializes object of class MemoryBuffer
     * @param size Buffer size.
     */
    explicit MemoryBuffer(std::size_t size = 0)
        : m_size(size)
        , m_data(m_size > 0 ? new T[m_size] : nullptr)
        , m_end(m_data.get() + m_size)
    {
    }

    /**
     * Initializes object of class MemoryBuffer
     * @param size Buffer size.
     */
    explicit MemoryBuffer(std::size_t size, const T& init)
        : m_size(size)
        , m_data(m_size > 0 ? new T[m_size] : nullptr)
        , m_end(m_data.get() + m_size)
    {
        if (m_data) {
            if constexpr (sizeof(T) == 1) {
                std::memset(m_data.get(), init, m_size);
            } else {
                for (auto p = m_data.get(); p != m_end; ++p)
                    *p = init;
            }
        }
    }

    /**
     * Initializes object of class MemoryBuffer
     * @param data Pointer to first value.
     * @param size Number of values.
     */
    MemoryBuffer(const T* data, std::size_t size)
        : m_size(size)
        , m_data(m_size > 0 ? new T[m_size] : nullptr)
        , m_end(m_data.get() + m_size)
    {
        if (m_data) std::memcpy(m_data.get(), data, sizeof(T) * m_size);
    }

    /**
     * Initializes object of class MemoryBuffer
     * @param first Pointer to first value.
     * @param last Pointer to value after last.
     */
    MemoryBuffer(const T* first, const T* last)
        : m_size(static_cast<std::size_t>(last - first))
        , m_data(m_size > 0 ? new T[m_size] : nullptr)
        , m_end(m_data.get() + m_size)
    {
        if (m_data) std::memcpy(m_data.get(), first, sizeof(T) * m_size);
    }

    /**
     * Initializes object of class MemoryBuffer
     * @param size Initializer list.
     */
    MemoryBuffer(std::initializer_list<T> list)
        : m_size(list.size())
        , m_data(m_size > 0 ? new T[m_size] : nullptr)
        , m_end(m_data.get() + m_size)
    {
        if (m_data) std::uninitialized_copy(list.begin(), list.end(), m_data.get());
    }

    /**
     * Initializes object of class MemoryBuffer.
     * @param src Source buffer.
     */
    MemoryBuffer(const MemoryBuffer& src)
        : m_size(src.m_size)
        , m_data(src.m_data ? new T[m_size] : nullptr)
        , m_end(m_data.get() + m_size)
    {
        if (m_data) std::memcpy(m_data.get(), src.m_data.get(), sizeof(T) * src.m_size);
    }

    /**
     * Initializes object of class MemoryBuffer.
     * @param src Source buffer.
     */
    MemoryBuffer(MemoryBuffer&& src) noexcept
        : m_size(src.m_size)
        , m_data(std::move(src.m_data))
        , m_end(src.m_end)
    {
        src.m_size = 0;
        src.m_end = nullptr;
    }

    /**
     * Copy assignment operator.
     * @param src Source buffer.
     * @return This object.
     */
    MemoryBuffer& operator=(const MemoryBuffer& src)
    {
        if (SIODB_LIKELY(&src != this)) {
            if (m_size != src.m_size) {
                m_data.reset(src.m_data ? new T[src.m_size] : nullptr);
                m_size = src.m_size;
                m_end = m_data.get() + m_size;
            }
            if (src.m_data) std::memcpy(m_data.get(), src.m_data.get(), sizeof(T) * src.m_size);
        }
        return *this;
    }

    /**
     * Move assignment operator.
     * @param src Source buffer.
     * @return This object.
     */
    MemoryBuffer& operator=(MemoryBuffer&& src)
    {
        if (SIODB_LIKELY(&src != this)) {
            m_size = src.m_size;
            m_data = std::move(src.m_data);
            m_end = src.m_end;
            src.m_size = 0;
            src.m_end = nullptr;
        }
        return *this;
    }

    /**
     * Equality operator.
     * @param other Other buffer.
     * @return true if buffers are equal, false otherwise.
     */
    bool operator==(const MemoryBuffer& other) const noexcept
    {
        if (SIODB_LIKELY(&other != this)) {
            if (m_size != other.m_size) return false;
            if (!m_data) return true;
            return std::memcmp(m_data.get(), other.m_data.get(), sizeof(T) * m_size) == 0;
        }
        return true;
    }

    /**
     * Non-equality operator.
     * @param other Other buffer.
     * @return true if buffers are equal, false otherwise.
     */
    bool operator!=(const MemoryBuffer& other) const noexcept
    {
        return !(*this == other);
    }

    /**
     * Less operator.
     * @param other Other buffer.
     * @return true if this buffer is "less" than other, false otherwise.
     */
    bool operator<(const MemoryBuffer& other) const noexcept
    {
        if (SIODB_LIKELY(&other != this)) {
            if (m_size == other.m_size) {
                return m_data ? std::memcmp(m_data.get(), other.m_data.get(), sizeof(T) * m_size)
                                        < 0
                              : false;
            } else
                return m_size < other.m_size;
        }
        return false;
    }

    /**
     * Greater operator.
     * @param other Other buffer.
     * @return true if this buffer is "greater" than other, false otherwise.
     */
    bool operator>(const MemoryBuffer& other) const noexcept
    {
        return other < *this;
    }

    /**
     * Less or equal operator.
     * @param other Other buffer.
     * @return true if this buffer is "less" than other or equal to other, false otherwise.
     */
    bool operator<=(const MemoryBuffer& other) const noexcept
    {
        return !(other < *this);
    }

    /**
     * Greater or equal operator.
     * @param other Other buffer.
     * @return true if this buffer is "greater" than other or equal to other, false otherwise.
     */
    bool operator>=(const MemoryBuffer& other) const noexcept
    {
        return !(*this < other);
    }

    /**
     * Returns mutable begin iterator.
     * @return Mutable iterartor.
     */
    T* begin() noexcept
    {
        return m_data.get();
    }

    /**
     * Returns read-only begin iterator.
     * @return Read-only iterartor.
     */
    const T* begin() const noexcept
    {
        return m_data.get();
    }

    /**
     * Returns read-only begin iterator.
     * @return Read-only iterartor.
     */
    const T* cbegin() const noexcept
    {
        return m_data.get();
    }

    /**
     * Return mutable end iterator.
     * @return Mutable iterartor.
     */
    T* end() noexcept
    {
        return m_end;
    }

    /**
     * Returns read-only end iterator.
     * @return Read-only iterartor.
     */
    const T* end() const noexcept
    {
        return m_end;
    }

    /**
     * Returns read-only end iterator.
     * @return Read-only iterartor.
     */
    const T* cend() const noexcept
    {
        return m_end;
    }

    /**
     * Returns indication that buffer has zero size.
     * @return true is buffer has zero size, false otherwise.
     */
    bool empty() const noexcept
    {
        return !m_data;
    }

    /**
     * Returns buffer size.
     * @return Buffer size.
     */
    std::size_t size() const noexcept
    {
        return m_size;
    }

    /**
     * Returns buffer size.
     * @return Buffer data pointer.
     */
    T* data() noexcept
    {
        return m_data.get();
    }

    /**
     * Returns buffer size.
     * @return Buffer data pointer.
     */
    const T* data() const noexcept
    {
        return m_data.get();
    }

    /**
     * Returns mutable first value from the buffer.
     * @return Mutable value from the buffer.
     */
    T& front() noexcept
    {
        return *m_data.get();
    }

    /**
     * Returns read-only first value from the buffer.
     * @return Read-only value from the buffer.
     */
    const T& front() const noexcept
    {
        return *m_data.get();
    }

    /**
     * Returns mutable last value from the buffer.
     * @return Mutable value from the buffer.
     */
    T& back() noexcept
    {
        return m_end[-1];
    }

    /**
     * Returns read-only last value from the buffer.
     * @return Read-only value from the buffer.
     */
    const T& back() const noexcept
    {
        return m_end[-1];
    }

    /**
     * Returns n-th mutable value from the buffer.
     * @param n An index.
     * @return Read-only value from the buffer.
     */
    T& operator[](std::size_t n) noexcept
    {
        return m_data.get()[n];
    }

    /**
     * Returns n-th read-only value from the buffer.
     * @param n An index.
     * @return Read-only value from the buffer.
     */
    const T& operator[](std::size_t n) const noexcept
    {
        return m_data.get()[n];
    }

    /** Clears buffer to zero bytes. */
    void clear() noexcept
    {
        m_size = 0;
        m_data.reset();
        m_end = nullptr;
    }

    /**
     * Resizes buffer. Leaves extra bytes uninitialized.
     * @param newSize New buffer size.
     */
    void resize(std::size_t newSize)
    {
        if (newSize == m_size) return;
        if (newSize > 0) {
            T* newData = new T[newSize];
            if (m_data) {
                std::memcpy(
                        newData, m_data.get(), sizeof(T) * (newSize < m_size ? newSize : m_size));
            }
            m_data.reset(newData);
            m_end = newData + newSize;
        } else {
            m_data.reset();
            m_end = nullptr;
        }
        m_size = newSize;
    }

    /**
     * Resizes buffer, fills extra bytes with given value.
     * @param newSize New buffer size.
     * @param value Fill value.
     */
    void resize(std::size_t newSize, const T& value)
    {
        if (newSize == m_size) return;
        if (newSize > 0) {
            T* newData = new T[newSize];
            const auto newEnd = newData + newSize;
            if (m_data) {
                std::memcpy(
                        newData, m_data.get(), sizeof(T) * (newSize < m_size ? newSize : m_size));
                if (newSize > m_size) {
                    if constexpr (sizeof(T) == 1)
                        std::memset(newData + m_size, value, newSize - m_size);
                    else
                        std::uninitialized_fill(newData + m_size, newEnd, value);
                }
            }
            m_data.reset(newData);
            m_end = newEnd;
        } else {
            m_data.reset();
            m_end = nullptr;
        }
        m_size = newSize;
    }

    /**
     * Fills buffer with given value.
     * @param value A value.
     */
    void fill(const T& value) noexcept
    {
        if (m_data) {
            if constexpr (sizeof(T) == 1)
                std::memset(m_data.get(), value, m_size);
            else
                std::uninitialized_fill(m_data.get(), end, value);
        }
    }

    /**
     * Swaps contents of this memory buffer with other.
     * @param other Other memory buffer.
     */
    void swap(MemoryBuffer& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) {
            std::swap(m_size, other.m_size);
            m_data.swap(other.m_data);
            std::swap(m_end, other.m_end);
        }
    }

private:
    /** Buffer size */
    std::size_t m_size;
    /** Buffer data */
    std::unique_ptr<T[]> m_data;
    /** End of buffer */
    T* m_end;
};

/**
 * Swaps two memory buffers.
 * @param a First memory buffer.
 * @param b Second memory buffer.
 */
template<class T>
inline void swap(MemoryBuffer<T>& a, MemoryBuffer<T>& b) noexcept
{
    a.swap(b);
}

}  // namespace siodb::utils
