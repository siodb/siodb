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

namespace stdext {

/**
 * Memory buffer with elements of trivial type.
 * @tparam Element Buffer element type.
 */
template<class Element>
class buffer {
    static_assert(std::is_trivial_v<Element>,
            "Template argument T must be trivial type in the class stdext::buffer");

public:
    using value_type = Element;
    using reference = Element&;
    using const_reference = const Element&;
    using pointer = Element*;
    using const_pointer = const Element*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

public:
    using iterator = pointer;
    using const_iterator = const_pointer;

public:
    /**
     * Initializes object of class buffer.
     * @param size Buffer size.
     */
    explicit buffer(size_type size = 0)
        : m_data(size > 0 ? new value_type[size] : nullptr)
        , m_end(m_data.get() + size)
    {
    }

    /**
     * Initializes object of class buffer.
     * @param size Buffer size.
     */
    template<typename Init =
                     std::conditional<std::is_fundamental_v<Element>, value_type, const_reference>>
    explicit buffer(size_type size, Init init)
        : m_data(size > 0 ? new value_type[size] : nullptr)
        , m_end(m_data.get() + size)
    {
        if (SIODB_LIKELY(m_data.get() != nullptr)) {
            if constexpr (std::is_fundamental_v<Init> && sizeof(Init) == 1)
                std::memset(m_data.get(), init, size);
            else
                std::uninitialized_fill(m_data.get(), m_end, init);
        }
    }

    /**
     * Initializes object of class buffer.
     * @param data Pointer to first value.
     * @param size Number of values.
     */
    buffer(const_pointer data, size_type size)
        : m_data(size > 0 ? new value_type[size] : nullptr)
        , m_end(m_data.get() + size)
    {
        if (SIODB_LIKELY(m_data.get() != nullptr))
            std::memcpy(m_data.get(), data, sizeof(value_type) * size);
    }

    /**
     * Initializes object of class buffer.
     * @param data Pointer to first value.
     * @param size Number of values.
     * @param attach Attach data flag. Value false causes copying data,
     *               value true - attaching data as buffer data.
     */
    buffer(pointer data, size_type size, bool attach = false)
        : m_data(attach ? data : (size > 0 ? new value_type[size] : nullptr))
        , m_end(m_data.get() + size)
    {
        if (SIODB_UNLIKELY(attach)) return;
        if (SIODB_LIKELY(m_data.get() != nullptr))
            std::memcpy(m_data.get(), data, sizeof(value_type) * size);
    }

    /**
     * Initializes object of class buffer.
     * @param first Pointer to first value.
     * @param last Pointer to value after last.
     */
    buffer(const Element* first, const Element* last)
        : m_data(static_cast<std::size_t>(last - first) > 0
                         ? new value_type[static_cast<std::size_t>(last - first)]
                         : nullptr)
        , m_end(m_data.get() + static_cast<std::size_t>(last - first))
    {
        if (SIODB_LIKELY(m_data.get() != nullptr))
            std::memcpy(m_data.get(), first, sizeof(value_type) * size());
    }

    /**
     * Initializes object of class buffer.
     * @param size Initializer list.
     */
    buffer(std::initializer_list<Element> list)
        : m_data(list.size() > 0 ? new value_type[list.size()] : nullptr)
        , m_end(m_data.get() + list.size())
    {
        if (SIODB_LIKELY(m_data.get() != nullptr))
            std::uninitialized_copy(list.begin(), list.end(), m_data.get());
    }

    /**
     * Initializes object of class buffer.
     * @param src Source buffer.
     */
    buffer(const buffer& src)
        : m_data(src.m_data ? new value_type[src.size()] : nullptr)
        , m_end(m_data.get() + src.size())
    {
        if (SIODB_LIKELY(m_data.get() != nullptr))
            std::memcpy(m_data.get(), src.m_data.get(), sizeof(value_type) * src.size());
    }

    /**
     * Initializes object of class buffer.
     * @param src Source buffer.
     */
    buffer(buffer&& src) noexcept
        : m_data(std::move(src.m_data))
        , m_end(src.m_end)
    {
        src.m_end = nullptr;
    }

    /**
     * Copy assignment operator.
     * @param src Source buffer.
     * @return This object.
     */
    buffer& operator=(const buffer& src)
    {
        if (SIODB_LIKELY(&src != this)) {
            const auto srcSize = src.size();
            if (size() != srcSize) {
                m_data.reset(src.m_data ? new value_type[srcSize] : nullptr);
                m_end = m_data.get() + srcSize;
            }
            if (src.m_data)
                std::memcpy(m_data.get(), src.m_data.get(), sizeof(value_type) * srcSize);
        }
        return *this;
    }

    /**
     * Move assignment operator.
     * @param src Source buffer.
     * @return This object.
     */
    buffer& operator=(buffer&& src)
    {
        if (SIODB_LIKELY(&src != this)) {
            m_data = std::move(src.m_data);
            m_end = src.m_end;
            src.m_end = nullptr;
        }
        return *this;
    }

    /**
     * Equality operator.
     * @param other Other buffer.
     * @return true if buffers are equal, false otherwise.
     */
    bool operator==(const buffer& other) const noexcept
    {
        if (SIODB_LIKELY(&other != this)) {
            if (size() != other.size()) return false;
            if (!m_data) return true;
            return std::memcmp(m_data.get(), other.m_data.get(), sizeof(value_type) * size()) == 0;
        }
        return true;
    }

    /**
     * Non-equality operator.
     * @param other Other buffer.
     * @return true if buffers are equal, false otherwise.
     */
    bool operator!=(const buffer& other) const noexcept
    {
        return !(*this == other);
    }

    /**
     * Less operator.
     * @param other Other buffer.
     * @return true if this buffer is "less" than other, false otherwise.
     */
    bool operator<(const buffer& other) const noexcept
    {
        if (SIODB_LIKELY(&other != this)) {
            const auto thisSize = size();
            const auto otherSize = other.size();
            if (thisSize == otherSize) {
                return m_data ? std::memcmp(m_data.get(), other.m_data.get(),
                                        sizeof(value_type) * thisSize)
                                        < 0
                              : false;
            } else
                return thisSize < otherSize;
        }
        return false;
    }

    /**
     * Greater operator.
     * @param other Other buffer.
     * @return true if this buffer is "greater" than other, false otherwise.
     */
    bool operator>(const buffer& other) const noexcept
    {
        return other < *this;
    }

    /**
     * Less or equal operator.
     * @param other Other buffer.
     * @return true if this buffer is "less" than other or equal to other, false otherwise.
     */
    bool operator<=(const buffer& other) const noexcept
    {
        return !(other < *this);
    }

    /**
     * Greater or equal operator.
     * @param other Other buffer.
     * @return true if this buffer is "greater" than other or equal to other, false otherwise.
     */
    bool operator>=(const buffer& other) const noexcept
    {
        return !(*this < other);
    }

    /**
     * Returns mutable begin iterator.
     * @return Mutable iterartor.
     */
    iterator begin() noexcept
    {
        return m_data.get();
    }

    /**
     * Returns read-only begin iterator.
     * @return Read-only iterartor.
     */
    const_iterator begin() const noexcept
    {
        return m_data.get();
    }

    /**
     * Returns read-only begin iterator.
     * @return Read-only iterartor.
     */
    const_iterator cbegin() const noexcept
    {
        return m_data.get();
    }

    /**
     * Return mutable end iterator.
     * @return Mutable iterartor.
     */
    iterator end() noexcept
    {
        return m_end;
    }

    /**
     * Returns read-only end iterator.
     * @return Read-only iterartor.
     */
    const_iterator end() const noexcept
    {
        return m_end;
    }

    /**
     * Returns read-only end iterator.
     * @return Read-only iterartor.
     */
    const_iterator cend() const noexcept
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
    size_type size() const noexcept
    {
        return static_cast<size_type>(m_end - data());
    }

    /**
     * Returns buffer size.
     * @return Buffer data pointer.
     */
    pointer data() noexcept
    {
        return m_data.get();
    }

    /**
     * Returns buffer size.
     * @return Buffer data pointer.
     */
    pointer data() const noexcept
    {
        return m_data.get();
    }

    /**
     * Returns mutable first value from the buffer.
     * @return Mutable value from the buffer.
     */
    reference front() noexcept
    {
        return *m_data.get();
    }

    /**
     * Returns read-only first value from the buffer.
     * @return Read-only value from the buffer.
     */
    const_reference front() const noexcept
    {
        return *m_data.get();
    }

    /**
     * Returns mutable last value from the buffer.
     * @return Mutable value from the buffer.
     */
    reference back() noexcept
    {
        return m_end[-1];
    }

    /**
     * Returns read-only last value from the buffer.
     * @return Read-only value from the buffer.
     */
    const_reference back() const noexcept
    {
        return m_end[-1];
    }

    /**
     * Returns n-th mutable value from the buffer. Validates index.
     * @param n An index.
     * @return Mutable value from the buffer.
     * @throw std::out_of_range if n is greater or equal to buffer size.
     */
    reference at(size_type n) noexcept
    {
        validate_index(n);
        return m_data.get()[n];
    }

    /**
     * Returns n-th read-only value from the buffer. Validates index.
     * @param n An index.
     * @return Read-only value from the buffer.
     * @throw std::out_of_range if n is greater or equal to buffer size.
     */
    const_reference at(size_type n) const
    {
        validate_index(n);
        return m_data.get()[n];
    }

    /**
     * Returns n-th mutable value from the buffer. Doesn't validate index.
     * @param n An index.
     * @return Mutable value from the buffer.
     */
    reference operator[](size_type n) noexcept
    {
        return m_data.get()[n];
    }

    /**
     * Returns n-th read-only value from the buffer. Doesn't validate index.
     * @param n An index.
     * @return Read-only value from the buffer.
     */
    const_reference operator[](size_type n) const noexcept
    {
        return m_data.get()[n];
    }

    /** Clears buffer to zero bytes. */
    void clear() noexcept
    {
        m_data.reset();
        m_end = nullptr;
    }

    /**
     * Resizes buffer. Leaves extra bytes uninitialized.
     * @param new_size New buffer size.
     */
    void resize(size_type new_size)
    {
        const auto cur_size = size();
        if (new_size == cur_size) return;
        if (new_size > 0) {
            auto new_data = new value_type[new_size];
            if (m_data) {
                std::memcpy(new_data, m_data.get(),
                        sizeof(value_type) * (new_size < cur_size ? new_size : cur_size));
            }
            m_data.reset(new_data);
            m_end = new_data + new_size;
        } else
            clear();
    }

    /**
     * Resizes buffer, fills extra bytes with given value.
     * @param new_size New buffer size.
     * @param value Fill value.
     */
    template<typename Init =
                     std::conditional<std::is_fundamental_v<Element>, value_type, const_reference>>
    void resize(size_type new_size, Init value)
    {
        const auto cur_size = size();
        if (new_size == cur_size) return;
        if (new_size > 0) {
            auto new_data = new value_type[new_size];
            const auto new_end = new_data + new_size;
            if (m_data) {
                std::memcpy(new_data, m_data.get(),
                        sizeof(value_type) * (new_size < cur_size ? new_size : cur_size));
            }
            if (new_size > cur_size) {
                if constexpr (std::is_fundamental_v<Init> && sizeof(Init) == 1)
                    std::memset(new_data + cur_size, value, new_size - cur_size);
                else
                    std::uninitialized_fill(new_data + cur_size, new_end, value);
            }
            m_data.reset(new_data);
            m_end = new_end;
        } else
            clear();
    }

    /**
     * Fills buffer with given value.
     * @param value A value.
     */
    template<typename Init =
                     std::conditional<std::is_fundamental_v<Element>, value_type, const_reference>>
    void fill(Init value) noexcept
    {
        if (m_data) {
            if constexpr (std::is_fundamental_v<Init> && sizeof(Init) == 1)
                std::memset(m_data.get(), value, size());
            else
                std::uninitialized_fill(data(), m_end, value);
        }
    }

    /**
     * Swaps contents of this buffer with other buffer.
     * @param other Other buffer.
     */
    void swap(buffer& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) {
            m_data.swap(other.m_data);
            std::swap(m_end, other.m_end);
        }
    }

private:
    /** 
     * Validates buffer index.
     * @param n An index.
     * @throw std::out_of_range if n is greater or equal to buffer size.
     */
    void validate_index(std::size_t n) const
    {
        if (n >= size()) throw std::out_of_range("buffer index is out of range");
    }

private:
    /** Buffer data */
    std::unique_ptr<value_type[]> m_data;
    /** End of buffer */
    pointer m_end;
};

/**
 * Swaps two buffers.
 * @tparam Element Buffer element type.
 * @param a First buffer.
 * @param b Second buffer.
 */
template<class Element>
inline void swap(buffer<Element>& a, buffer<Element>& b) noexcept
{
    a.swap(b);
}

}  // namespace stdext
