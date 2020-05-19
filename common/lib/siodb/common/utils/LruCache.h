// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

//---------------------------------------------------------------------------//
// Siodb LRU cache implementation.
// Based on the Boost.Compute LRU cache implementation.
// Original copyright notice:
//---------------------------------------------------------------------------//
// Copyright (c) 2013 Kyle Lutz <kyle.r.lutz@gmail.com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
// See http://boostorg.github.com/compute for more information.
//---------------------------------------------------------------------------//

// Project headers
#include "../stl_ext/utility_ext.h"

// STL headers
#include <list>
#include <optional>

namespace siodb::utils {

/** Thrown when LRU cache is full */
class LruCacheFullError : public std::runtime_error {
public:
    LruCacheFullError()
        : std::runtime_error("LRU cache full")
    {
    }
};

/** 
 * A cache which evicts the least recently used item when it is full.
 * @tparam Key Key type.
 * @tparam T Mapped type.
 * @tparam Map Map type used internally.
 * @tparam List List type used internally.
 * @note Uses STL-like naming convention.
 */
template<class Key, class T, class Map, class List = std::list<Key>>
class basic_lru_cache {
protected:
    /** Internal map type */
    using map_type = Map;

    /** Internal list type */
    using list_type = List;

    /** Map's mapped type */
    using map_mapped_type = std::pair<T, typename list_type::iterator>;

public:
    /** Key type */
    using key_type = typename map_type::key_type;

    /** Value type */
    using value_type = typename map_type::value_type;

    /** Mapped type */
    using mapped_type = T;

public:
    /**
     * Initializes object of class basic_lru_cache.
     * @param capacity Cache capacity (maximum allowed size).
     */
    explicit basic_lru_cache(std::size_t capacity) noexcept
        : m_capacity(capacity)
    {
    }

    /**
     * Initializes object of class basic_lru_cache.
     * @param src Source object.
     */
    explicit basic_lru_cache(basic_lru_cache&& src) noexcept
        : m_capacity(src.m_capacity)
    {
        swap(src);
    }

    /**
     * De-initializes object of class basic_lru_cache.
     */
    virtual ~basic_lru_cache()
    {
        // Clear explicitly to force on_evict() on each element
        clear();
    }

    /** Not copy-constructible. */
    basic_lru_cache(const basic_lru_cache&) = delete;

    /** Not copy-assignable. */
    basic_lru_cache& operator=(const basic_lru_cache&) = delete;

    /**
     * Moves content of a source object into this object, discards current content.
     * @param src Source object.
     */
    basic_lru_cache& operator=(basic_lru_cache&& src) noexcept
    {
        basic_lru_cache tmp(src.m_capacity);
        swap(tmp);
        swap(src);
        return *this;
    }

    /**
     * Returns current number of elements in cache.
     * @return Current number of elements in cache.
     */
    std::size_t size() const noexcept
    {
        return m_map.size();
    }

    /**
     * Returns cache capacity.
     * @return Cache capacity.
     */
    std::size_t capacity() const noexcept
    {
        return m_capacity;
    }

    /**
     * Returns indication that cache is empty.
     * @return true if cache is empty, false otherwise.
     */
    bool empty() const noexcept
    {
        return m_map.empty();
    }

    /**
     * Returns indication that elements in the cache.
     * @param key A key.
     * @return true if cache is in the cache, false otherwise.
     */
    bool contains(const key_type& key) const noexcept
    {
        return m_map.count(key) > 0;
    }

    /**
     * Returns first key.
     * @return First key.
     */
    template<typename ReturnType = key_type,
            typename = std::enable_if_t<!std::is_fundamental_v<ReturnType>>>
    const ReturnType& first_key() const
    {
        checkHasData();
        return m_map.begin()->first;
    }

    /**
     * Returns first key.
     * @return First key.
     */
    template<typename ReturnType = key_type,
            typename = std::enable_if_t<std::is_fundamental_v<ReturnType>>>
    ReturnType first_key() const
    {
        checkHasData();
        return m_map.begin()->first;
    }

    /**
     * Returns last key.
     * @return Last key.
     */
    template<typename ReturnType = key_type,
            typename = std::enable_if_t<!std::is_fundamental_v<ReturnType>>>
    const ReturnType& last_key() const
    {
        checkHasData();
        return m_map.rbegin()->first;
    }

    /**
     * Returns last key.
     * @return Last key.
     */
    template<typename ReturnType = key_type,
            typename = std::enable_if_t<std::is_fundamental_v<ReturnType>>>
    ReturnType last_key() const
    {
        checkHasData();
        return m_map.rbegin()->first;
    }

    /**
     * Ensures cache capacity is not less than given size.
     * Evicts extra elements if new cache capacity is less tha current.
     * @param capacity New cache capacity.
     */
    void extend(std::size_t capacity)
    {
        if (m_capacity < capacity) m_capacity = capacity;
    }

    /**
     * Gets cached element by key.
     * @param key A key.
     * @return std::optional with element, if element found, std::nullopt otherwise.
     */
    std::optional<mapped_type> get(const key_type& key) const
    {
        const auto it = m_map.find(key);
        if (it == m_map.cend()) return std::nullopt;
        if (it->second.second != m_list.begin()) {
            // Move item to the front of the most recently used list
            m_list.push_front(key_type());
            std::swap(*m_list.begin(), *it->second.second);
            m_list.erase(it->second.second);
            // Update iterator in map
            stdext::as_mutable(it->second).second = m_list.begin();
        }
        return it->second.first;
    }

    /**
     * Inserts element into the cache.
     * @param key A key.
     * @param value A value.
     * @param replace Replace value is exists.
     * @return true if element was new to cache, false otherwise.
     */
    bool emplace(const key_type& key, const mapped_type& value, bool replace = false)
    {
        const auto it = m_map.find(key);
        if (it == m_map.end()) {
            if (m_map.size() >= m_capacity) evict();
            m_list.push_front(key);
            const auto lit = m_list.begin();
            m_map.emplace(*lit, std::make_pair(value, lit));
            return true;
        } else if (replace) {
            it->second.first = value;
        }
        return false;
    }

    /**
     * Inserts element into the cache.
     * @param key A key.
     * @param value A value.
     * @param replace Replace value is exists.
     * @return true if element was new to cache, false otherwise.
     */
    bool emplace(const key_type& key, mapped_type&& value, bool replace = false)
    {
        const auto it = m_map.find(key);
        if (it == m_map.end()) {
            if (m_map.size() >= m_capacity) evict();
            m_list.push_front(key);
            const auto lit = m_list.begin();
            m_map.emplace(*lit, std::make_pair(std::move(value), lit));
            return true;
        } else if (replace) {
            it->second.first = std::move(value);
        }
        return false;
    }

    /**
     * Inserts element into the cache.
     * @param key A key.
     * @param value A value.
     * @param replace Replace value is exists.
     * @return true if element was new to cache, false otherwise.
     */
    bool emplace(key_type&& key, mapped_type&& value, bool replace = false)
    {
        const auto it = m_map.find(key);
        if (it == m_map.end()) {
            if (m_map.size() >= m_capacity) evict();
            m_list.push_front(std::move(key));
            const auto lit = m_list.begin();
            m_map.emplace(*lit, std::make_pair(std::move(value), lit));
            return true;
        } else if (replace) {
            it->second.first = std::move(value);
        }
        return false;
    }

    /**
     * Removes key and associated value from cache.
     * @param key A key.
     * @return true if key removed, false if key not found.
     */
    bool erase(const key_type& key)
    {
        const auto it = m_map.find(key);
        if (it == m_map.end()) return false;
        m_list.erase(it->second.second);
        m_map.erase(it);
        return true;
    }

    /** Clears cache */
    void clear() noexcept
    {
        for (auto& e : m_map) {
            try {
                on_evict(e.first, e.second.first, true);
            } catch (...) {
                // ignore all exceptions
            }
        }
        m_list.clear();
        m_map.clear();
    }

    /**
     * Swaps content with other cache object.
     * @param other Other cache object.
     */
    void swap(basic_lru_cache& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) {
            std::swap(m_capacity, other.m_capacity);
            m_map.swap(other.m_map);
            m_list.swap(other.m_list);
        }
    }

    /**
     * Cache mutable iterator.
     * Some ideas are taken from here: https://stackoverflow.com/a/38103394/1540501
     */
    class iterator {
    public:
        using iterator_category = typename map_type::iterator::iterator_category;
        using difference_type = typename map_type::iterator::difference_type;
        using value_type = std::pair<const key_type&, mapped_type&>;
        using pointer = value_type*;
        using reference = value_type&;

    public:
        /**
         * Initializes object of class iterator.
         */
        iterator() noexcept
            : m_map(nullptr)
        {
        }

        /**
         * Initializes object of class iterator.
         * @param map Map object.
         * @param iter Map iterator object.
         */
        iterator(map_type* map, typename map_type::iterator iter) noexcept
            : m_map(map)
            , m_iter(std::move(iter))
        {
        }

        /**
         * Initializes object of class iterator.
         * @param src Source iterator.
         */
        iterator(const iterator& src) noexcept
            : m_map(src.m_map)
            , m_iter(src.m_iter)
        {
        }

        /**
         * Initializes object of class iterator.
         * @param src Source iterator.
         */
        iterator(iterator&& src) noexcept
            : m_map(src.m_map)
            , m_iter(src.m_iter)
        {
            src.m_map = nullptr;
        }

        /**
         * Moves iterator one step forward (prefix increment).
         * @return This object.
         */
        iterator& operator++() noexcept
        {
            ++m_iter;
            return *this;
        }

        /**
         * Moves iterator one step forward (postfix increment).
         * @return New iterator.
         */
        iterator operator++(int) noexcept
        {
            auto retval = *this;
            ++(*this);
            return retval;
        }

        /**
         * Moves iterator one step back (prefix decrement).
         * @return This object.
         */
        iterator& operator--() noexcept
        {
            --m_iter;
            return *this;
        }

        /**
         * Moves iterator one step back (postfix decrement).
         * @return New iterator.
         */
        iterator operator--(int) noexcept
        {
            auto retval = *this;
            --(*this);
            return retval;
        }

        /**
         * Moves iterator N steps forward.
         * @param n Number of steps.
         * @return This object.
         */
        iterator& operator+=(difference_type n) noexcept
        {
            m_iter += n;
            return *this;
        }

        /**
         * Moves iterator N steps back.
         * @param n Number of steps.
         * @return This object.
         */
        iterator& operator-=(difference_type n) noexcept
        {
            m_iter -= n;
            return *this;
        }

        /**
         * Creates new iterator with position at N steps forward.
         * @param n Number of steps.
         * @return This object.
         */
        iterator operator+(difference_type n) noexcept
        {
            return iterator(m_map, m_iter + n);
        }

        /**
         * Creates new iterator with position at N steps back.
         * @param n Number of steps.
         * @return This object.
         */
        iterator operator-(difference_type n) noexcept
        {
            return iterator(m_map, m_iter - n);
        }

        /**
         * Compares this and other iterator for equality.
         * @return true if iterators are equal, false otherwise.
         */
        bool operator==(const iterator& other) const noexcept
        {
            return m_map == other.m_map && m_iter == other.m_iter;
        }

        /**
         * Compares this and other iterator for inequality.
         * @return true if iterators are equal, false otherwise.
         */
        bool operator!=(const iterator& other) const noexcept
        {
            return m_map != other.m_map || m_iter != other.m_iter;
        }

        /**
         * Returns current value.
         * @return Current value.
         */
        reference operator*() const noexcept
        {
            return *build_current_value();
        }

        /**
         * Returns pointer to current value.
         * @return Pointer to current value.
         */
        pointer operator->() const noexcept
        {
            return build_current_value();
        }

    private:
        /**
         * Arranges current value and returns pointer to it.
         * @return pointer to the current value.
         */
        pointer build_current_value() const noexcept
        {
            m_value.first = &m_iter->first;
            m_value.second = &m_iter->second.first;
            return reinterpret_cast<value_type*>(&m_value);
        }

    private:
        /** Referred cache map */
        map_type* m_map;

        /** Map iterator */
        typename map_type::iterator m_iter;

        /** Value buffer */
        mutable std::pair<const void*, void*> m_value;
    };

    /**
     * Cache read-only iterator.
     * Some ideas are taken from here: https://stackoverflow.com/a/38103394/1540501
     */
    class const_iterator {
    public:
        using iterator_category = typename map_type::const_iterator::iterator_category;
        using difference_type = typename map_type::const_iterator::difference_type;
        using value_type = std::pair<const key_type&, const mapped_type&>;
        using pointer = value_type*;
        using reference = value_type&;

    public:
        /**
         * Initializes object of class iterator.
         */
        const_iterator() noexcept
            : m_map(nullptr)
        {
        }

        /**
         * Initializes object of class iterator.
         * @param map Map object.
         * @param iter Map iterator object.
         */
        const_iterator(map_type* map, typename map_type::const_iterator iter) noexcept
            : m_map(map)
            , m_iter(std::move(iter))
        {
        }

        /**
         * Initializes object of class iterator.
         * @param src Source iterator.
         */
        const_iterator(const iterator& src) noexcept
            : m_map(src.m_map)
            , m_iter(src.m_iter)
        {
        }

        /**
         * Initializes object of class iterator.
         * @param src Source iterator.
         */
        const_iterator(const_iterator&& src) noexcept
            : m_map(src.m_map)
            , m_iter(src.m_iter)
        {
            src.m_map = nullptr;
        }

        /**
         * Moves iterator one step forward (prefix increment).
         * @return This object.
         */
        const_iterator& operator++() noexcept
        {
            ++m_iter;
            return *this;
        }

        /**
         * Moves iterator one step forward (postfix increment).
         * @return New iterator.
         */
        const_iterator operator++(int) noexcept
        {
            auto retval = *this;
            ++(*this);
            return retval;
        }

        /**
         * Moves iterator one step back (prefix decrement).
         * @return This object.
         */
        const_iterator& operator--() noexcept
        {
            --m_iter;
            return *this;
        }

        /**
         * Moves iterator one step back (postfix decrement).
         * @return New iterator.
         */
        const_iterator operator--(int) noexcept
        {
            auto retval = *this;
            --(*this);
            return retval;
        }

        /**
         * Moves iterator N steps forward.
         * @param n Number of steps.
         * @return This object.
         */
        const_iterator& operator+=(difference_type n) noexcept
        {
            m_iter += n;
            return *this;
        }

        /**
         * Moves iterator N steps back.
         * @param n Number of steps.
         * @return This object.
         */
        const_iterator& operator-=(difference_type n) noexcept
        {
            m_iter -= n;
            return *this;
        }

        /**
         * Creates new iterator with position at N steps forward.
         * @param n Number of steps.
         * @return This object.
         */
        const_iterator operator+(difference_type n) noexcept
        {
            return const_iterator(m_map, m_iter + n);
        }

        /**
         * Creates new iterator with position at N steps back.
         * @param n Number of steps.
         * @return This object.
         */
        const_iterator operator-(difference_type n) noexcept
        {
            return const_iterator(m_map, m_iter - n);
        }

        /**
         * Compares this and other iterator for equality.
         * @return true if iterators are equal, false otherwise.
         */
        bool operator==(const const_iterator& other) const noexcept
        {
            return m_map == other.m_map && m_iter == other.m_iter;
        }

        /**
         * Compares this and other iterator for inequality.
         * @return true if iterators are equal, false otherwise.
         */
        bool operator!=(const const_iterator& other) const noexcept
        {
            return m_map != other.m_map || m_iter != other.m_iter;
        }

        /**
         * Returns current value.
         * @return Current value.
         */
        reference operator*() const noexcept
        {
            return *build_current_value();
        }

        /**
         * Returns pointer to current value.
         * @return Pointer to current value.
         */
        pointer operator->() const noexcept
        {
            return build_current_value();
        }

    private:
        /**
         * Arranges current value and returns pointer to it.
         * @return pointer to the current value.
         */
        pointer build_current_value() const noexcept
        {
            m_value.first = &m_iter->first;
            m_value.second = &m_iter->second.first;
            return reinterpret_cast<value_type*>(&m_value);
        }

    private:
        /** Referred cache map */
        map_type* m_map;

        /** Map iterator */
        typename map_type::const_iterator m_iter;

        /** Value buffer */
        mutable std::pair<const void*, const void*> m_value;
    };

    /**
     * Cache mutable reverse iterator.
     * Some ideas are taken from here: https://stackoverflow.com/a/38103394/1540501
     */
    class reverse_iterator {
    public:
        using iterator_category = typename map_type::reverse_iterator::iterator_category;
        using difference_type = typename map_type::reverse_iterator::difference_type;
        using value_type = std::pair<const key_type&, mapped_type&>;
        using pointer = value_type*;
        using reference = value_type&;

    public:
        /**
         * Initializes object of class reverse_iterator.
         */
        reverse_iterator() noexcept
            : m_map(nullptr)
        {
        }

        /**
         * Initializes object of class reverse_iterator.
         * @param map Map object.
         * @param iter Map reverse_iterator object.
         */
        reverse_iterator(map_type* map, typename map_type::reverse_iterator iter) noexcept
            : m_map(map)
            , m_iter(std::move(iter))
        {
        }

        /**
         * Initializes object of class reverse_iterator.
         * @param src Source reverse_iterator.
         */
        reverse_iterator(const reverse_iterator& src) noexcept
            : m_map(src.m_map)
            , m_iter(src.m_iter)
        {
        }

        /**
         * Initializes object of class reverse_iterator.
         * @param src Source reverse_iterator.
         */
        reverse_iterator(reverse_iterator&& src) noexcept
            : m_map(src.m_map)
            , m_iter(src.m_iter)
        {
            src.m_map = nullptr;
        }

        /**
         * Moves reverse_iterator one step forward (prefix increment).
         * @return This object.
         */
        reverse_iterator& operator++() noexcept
        {
            ++m_iter;
            return *this;
        }

        /**
         * Moves reverse_iterator one step forward (postfix increment).
         * @return New reverse_iterator.
         */
        reverse_iterator operator++(int) noexcept
        {
            auto retval = *this;
            ++(*this);
            return retval;
        }

        /**
         * Moves reverse_iterator one step back (prefix decrement).
         * @return This object.
         */
        reverse_iterator& operator--() noexcept
        {
            --m_iter;
            return *this;
        }

        /**
         * Moves reverse_iterator one step back (postfix decrement).
         * @return New reverse_iterator.
         */
        reverse_iterator operator--(int) noexcept
        {
            auto retval = *this;
            --(*this);
            return retval;
        }

        /**
         * Moves reverse_iterator N steps forward.
         * @param n Number of steps.
         * @return This object.
         */
        reverse_iterator& operator+=(difference_type n) noexcept
        {
            m_iter += n;
            return *this;
        }

        /**
         * Moves reverse_iterator N steps back.
         * @param n Number of steps.
         * @return This object.
         */
        reverse_iterator& operator-=(difference_type n) noexcept
        {
            m_iter -= n;
            return *this;
        }

        /**
         * Creates new reverse_iterator with position at N steps forward.
         * @param n Number of steps.
         * @return This object.
         */
        reverse_iterator operator+(difference_type n) noexcept
        {
            return reverse_iterator(m_map, m_iter + n);
        }

        /**
         * Creates new reverse_iterator with position at N steps back.
         * @param n Number of steps.
         * @return This object.
         */
        reverse_iterator operator-(difference_type n) noexcept
        {
            return reverse_iterator(m_map, m_iter - n);
        }

        /**
         * Compares this and other reverse_iterator for equality.
         * @return true if iterators are equal, false otherwise.
         */
        bool operator==(const reverse_iterator& other) const noexcept
        {
            return m_map == other.m_map && m_iter == other.m_iter;
        }

        /**
         * Compares this and other reverse_iterator for inequality.
         * @return true if iterators are equal, false otherwise.
         */
        bool operator!=(const reverse_iterator& other) const noexcept
        {
            return m_map != other.m_map || m_iter != other.m_iter;
        }

        /**
         * Returns current value.
         * @return Current value.
         */
        reference operator*() const noexcept
        {
            return *build_current_value();
        }

        /**
         * Returns pointer to current value.
         * @return Pointer to current value.
         */
        pointer operator->() const noexcept
        {
            return build_current_value();
        }

    private:
        /**
         * Arranges current value and returns pointer to it.
         * @return pointer to the current value.
         */
        pointer build_current_value() const noexcept
        {
            m_value.first = &m_iter->first;
            m_value.second = &m_iter->second.first;
            return reinterpret_cast<value_type*>(&m_value);
        }

    private:
        /** Referred cache map */
        map_type* m_map;

        /** Map reverse_iterator */
        typename map_type::reverse_iterator m_iter;

        /** Value buffer */
        mutable std::pair<const void*, void*> m_value;
    };

    /**
     * Cache read-only reverse iterator.
     * Some ideas are taken from here: https://stackoverflow.com/a/38103394/1540501
     */
    class const_reverse_iterator {
    public:
        using iterator_category = typename map_type::const_reverse_iterator::iterator_category;
        using difference_type = typename map_type::const_reverse_iterator::difference_type;
        using value_type = std::pair<const key_type&, const mapped_type&>;
        using pointer = value_type*;
        using reference = value_type&;

    public:
        /**
         * Initializes object of class const_reverse_iterator.
         */
        const_reverse_iterator() noexcept
            : m_map(nullptr)
        {
        }

        /**
         * Initializes object of class const_reverse_iterator.
         * @param map Map object.
         * @param iter Map const_reverse_iterator object.
         */
        const_reverse_iterator(
                map_type* map, typename map_type::const_reverse_iterator iter) noexcept
            : m_map(map)
            , m_iter(std::move(iter))
        {
        }

        /**
         * Initializes object of class const_reverse_iterator.
         * @param src Source const_reverse_iterator.
         */
        const_reverse_iterator(const const_reverse_iterator& src) noexcept
            : m_map(src.m_map)
            , m_iter(src.m_iter)
        {
        }

        /**
         * Initializes object of class const_reverse_iterator.
         * @param src Source const_reverse_iterator.
         */
        const_reverse_iterator(const_reverse_iterator&& src) noexcept
            : m_map(src.m_map)
            , m_iter(src.m_iter)
        {
            src.m_map = nullptr;
        }

        /**
         * Moves const_reverse_iterator one step forward (prefix increment).
         * @return This object.
         */
        const_reverse_iterator& operator++() noexcept
        {
            ++m_iter;
            return *this;
        }

        /**
         * Moves const_reverse_iterator one step forward (postfix increment).
         * @return New const_reverse_iterator.
         */
        const_reverse_iterator operator++(int) noexcept
        {
            auto retval = *this;
            ++(*this);
            return retval;
        }

        /**
         * Moves const_reverse_iterator one step back (prefix decrement).
         * @return This object.
         */
        const_reverse_iterator& operator--() noexcept
        {
            --m_iter;
            return *this;
        }

        /**
         * Moves const_reverse_iterator one step back (postfix decrement).
         * @return New const_reverse_iterator.
         */
        const_reverse_iterator operator--(int) noexcept
        {
            auto retval = *this;
            --(*this);
            return retval;
        }

        /**
         * Moves const_reverse_iterator N steps forward.
         * @param n Number of steps.
         * @return This object.
         */
        const_reverse_iterator& operator+=(difference_type n) noexcept
        {
            m_iter += n;
            return *this;
        }

        /**
         * Moves const_reverse_iterator N steps back.
         * @param n Number of steps.
         * @return This object.
         */
        const_reverse_iterator& operator-=(difference_type n) noexcept
        {
            m_iter -= n;
            return *this;
        }

        /**
         * Creates new const_reverse_iterator with position at N steps forward.
         * @param n Number of steps.
         * @return This object.
         */
        const_reverse_iterator operator+(difference_type n) noexcept
        {
            return const_reverse_iterator(m_map, m_iter + n);
        }

        /**
         * Creates new const_reverse_iterator with position at N steps back.
         * @param n Number of steps.
         * @return This object.
         */
        const_reverse_iterator operator-(difference_type n) noexcept
        {
            return const_reverse_iterator(m_map, m_iter - n);
        }

        /**
         * Compares this and other const_reverse_iterator for equality.
         * @return true if iterators are equal, false otherwise.
         */
        bool operator==(const const_reverse_iterator& other) const noexcept
        {
            return m_map == other.m_map && m_iter == other.m_iter;
        }

        /**
         * Compares this and other const_reverse_iterator for inequality.
         * @return true if iterators are equal, false otherwise.
         */
        bool operator!=(const const_reverse_iterator& other) const noexcept
        {
            return m_map != other.m_map || m_iter != other.m_iter;
        }

        /**
         * Returns reference to current value.
         * @return Reference to current value.
         */
        reference operator*() const noexcept
        {
            return *build_current_value();
        }

        /**
         * Returns pointer to current value.
         * @return Pointer to current value.
         */
        pointer operator->() const noexcept
        {
            return build_current_value();
        }

    private:
        /**
         * Arranges current value and returns pointer to it.
         * @return pointer to the current value.
         */
        pointer build_current_value() const noexcept
        {
            m_value.first = &m_iter->first;
            m_value.second = &m_iter->second.first;
            return reinterpret_cast<value_type*>(&m_value);
        }

    private:
        /** Referred cache map */
        map_type* m_map;

        /** Map const_reverse_iterator */
        typename map_type::const_reverse_iterator m_iter;

        /** Value buffer */
        mutable std::pair<const void*, const void*> m_value;
    };

    /**
     * Returns begin mutable access iterator.
     * @return Iterator.
     */
    auto begin() noexcept
    {
        return iterator(&m_map, m_map.begin());
    }

    /**
     * Returns begin read-only iterator.
     * @return Iterator.
     */
    auto begin() const noexcept
    {
        return const_iterator(&m_map, m_map.cbegin());
    }

    /**
     * Returns begin read-only iterator.
     * @return Iterator.
     */
    auto cbegin() const noexcept
    {
        return const_iterator(&m_map, m_map.cbegin());
    }

    /**
     * Returns end mutable access iterator.
     * @return Iterator.
     */
    auto end() noexcept
    {
        return iterator(&m_map, m_map.end());
    }

    /**
     * Returns end read-only iterator.
     * @return Iterator.
     */
    auto end() const noexcept
    {
        return const_iterator(&m_map, m_map.cend());
    }

    /**
     * Returns end read-only iterator.
     * @return Iterator.
     */
    auto cend() const noexcept
    {
        return const_iterator(&m_map, m_map.cend());
    }

    /**
     * Returns begin mutable access reverse iterator.
     * @return Iterator.
     */
    auto rbegin() noexcept
    {
        return reverse_iterator(&m_map, m_map.rbegin());
    }

    /**
     * Returns begin read-only reverse iterator.
     * @return Iterator.
     */
    auto rbegin() const noexcept
    {
        return const_reverse_iterator(&m_map, m_map.crbegin());
    }

    /**
     * Returns begin read-only iterator.
     * @return Iterator.
     */
    auto crbegin() const noexcept
    {
        return const_reverse_iterator(&m_map, m_map.crbegin());
    }

    /**
     * Returns end mutable access reverse iterator.
     * @return Iterator.
     */
    auto rend() noexcept
    {
        return reverse_iterator(&m_map, m_map.rend());
    }

    /**
     * Returns end read-only iterator.
     * @return Iterator.
     */
    auto rend() const noexcept
    {
        return const_reverse_iterator(&m_map, m_map.crend());
    }

    /**
     * Returns end read-only iterator.
     * @return Iterator.
     */
    auto crend() const noexcept
    {
        return const_reverse_iterator(&m_map, m_map.crend());
    }

    /**
     * Finds element with a given key.
     * @return Iterator to an element, if it is found, or end iterator othwerwise.
     */
    auto find(const key_type& key) noexcept
    {
        return iterator(&m_map, m_map.find(key));
    }

    /**
     * Finds element with a given key.
     * @return Iterator to an element, if it is found, or end iterator othwerwise.
     */
    auto find(const key_type& key) const noexcept
    {
        return const_iterator(&m_map, m_map.find(key));
    }

protected:
    /**
     * Returns internal map.
     * @return Internal map.
     */
    const auto& map_internal() const noexcept
    {
        return m_map;
    }

    /**
     * Returns internal list.
     * @return Internal list.
     */
    const auto& list_internal() const noexcept
    {
        return m_list;
    }

    /**
     * Returns indication if item can be evicted. By default all items can be evicted.
     * @param key A key.
     * @param value A value.
     * @return true if item can be evicted, false otherwise.
     */
    virtual bool can_evict([[maybe_unused]] const key_type& key,
            [[maybe_unused]] const mapped_type& value) const noexcept
    {
        return true;
    }

    /**
     * Called before item gets evicted from the cache.
     * @param key A key.
     * @param value A value.
     */
    virtual void on_evict([[maybe_unused]] const key_type& key, [[maybe_unused]] mapped_type& value,
            [[maybe_unused]] bool clearingCache) const
    {
    }

    /**
     * Called if evict() could not find any discardable element
     * to give last chance for making some additional cleanup.
     * By default no additional action taken.
     * @return true if some changes were made so that next eviction attempt
     *         must be made, false otherwise.
     */
    virtual bool on_last_chance_cleanup()
    {
        return false;
    }

    /**
     * Evicts most outdated element from cache which is allowed to be evicted.
     * Can be overridden.
     */
    virtual void evict()
    {
        while (true) {
            for (auto lrit = m_list.rbegin(); lrit != m_list.rend(); ++lrit) {
                const auto mit = m_map.find(*lrit);
                if (mit == m_map.end()) throw std::runtime_error("LRU cache corrupted");
                if (can_evict(mit->first, mit->second.first)) {
                    on_evict(mit->first, mit->second.first, false);
                    m_list.erase(mit->second.second);
                    m_map.erase(mit);
                    return;
                }
            }
            if (!on_last_chance_cleanup()) throw LruCacheFullError();
        }
    }

    /** Checks that there is some data */
    void checkHasData() const
    {
        if (m_map.empty()) throw std::logic_error("LRU cache is empty");
    }

private:
    /** Cache capacity */
    std::size_t m_capacity;

    /** Key-value map */
    map_type m_map;

    /** LRU order list */
    mutable list_type m_list;
};

/**
 * Swaps two basic_lru_cache objects.
 * @param a First object.
 * @param b Second object.
 */
template<class Key, class Value, class List, class Map>
inline void swap(basic_lru_cache<Key, Value, List, Map>& a,
        basic_lru_cache<Key, Value, List, Map>& b) noexcept
{
    a.swap(b);
}

}  // namespace siodb::utils
