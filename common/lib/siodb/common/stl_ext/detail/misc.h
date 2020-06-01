// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstring>

// STL headers
#include <memory>
#include <type_traits>

namespace stdext::detail {

/**
 * Detects POD type.
 * @tparam T A type.
 */
template<typename T>
using is_trivial_type = ::std::enable_if_t<::std::is_trivial_v<T>, bool>;

/**
 * Detects non-POD type.
 * @tparam T A type.
 */
template<typename T>
using is_non_trivial_type = ::std::enable_if_t<!::std::is_trivial_v<T>, bool>;

/**
 * Detects move constructible type.
 * @tparam T A type.
 */
template<typename T>
using is_move_constructible_type = ::std::enable_if_t<::std::is_move_constructible_v<T>, bool>;

/**
 * Detects non-move contructible type.
 * @tparam T A type.
 */
template<typename T>
using is_not_move_constructible_type = ::std::enable_if_t<!::std::is_move_constructible_v<T>, bool>;

/**
 * Detects trivially constructible type.
 * @tparam T A type.
 */
template<typename T>
using is_trivially_constructible_type =
        ::std::enable_if_t<::std::is_trivially_constructible_v<T>, bool>;

/**
 * Detects non-trivially constructible type.
 * @tparam T A type.
 */
template<typename T>
using is_not_trivially_constructible_type =
        ::std::enable_if_t<!::std::is_trivially_constructible_v<T>, bool>;

/**
 * Detects trivially destructible type.
 * @tparam T A type.
 */
template<typename T>
using is_trivially_destructible_type =
        ::std::enable_if_t<::std::is_trivially_destructible_v<T>, bool>;

/**
 * Detects non-trivially destructible type.
 * @tparam T A type.
 */
template<typename T>
using is_not_trivially_destructible_type =
        ::std::enable_if_t<!::std::is_trivially_destructible_v<T>, bool>;

/**
 * Compares two ranges of the same size for equality.
 * @tparam T Underlying value type.
 * @param first1 Pointer to a first item of the first range.
 * @param last1 Pointer to an item right after the last item of the first range.
 * @param first2 Pointer to a first item of the second range.
 * @return true if elements in the range are equal, false otherwise.
 */
template<class T, is_trivial_type<T> = true>
inline bool are_ranges_equal(const T* first1, const T* last1, const T* first2) noexcept
{
    return ::std::memcmp(first1, first2, (last1 - first1) * sizeof(T)) == 0;
}

/**
 * Compares two ranges of the same size for equality.
 * @tparam T Underlying value type.
 * @param first1 Pointer to a first item of the first range.
 * @param last1 Pointer to an item right after the last item of the first range.
 * @param first2 Pointer to a first item of the second range.
 * @return true if elements in the range are equal, false otherwise.
 */
template<class T, is_non_trivial_type<T> = true>
bool are_ranges_equal(const T* first1, const T* last1, const T* first2) noexcept
{
    for (; first1 != last1; ++first1, ++first2) {
        if (*first1 != *first2) return false;
    }
    return true;
}

/**
 * Compares two ranges of the same size for non-equality.
 * @tparam T Underlying value type.
 * @param first1 Pointer to a first item of the first range.
 * @param last1 Pointer to an item right after the last item of the first range.
 * @param first2 Pointer to a first item of the second range.
 * @return true if elements in the range are not equal, false otherwise.
 */
template<class T, is_trivial_type<T> = true>
inline bool are_ranges_not_equal(const T* first1, const T* last1, const T* first2) noexcept
{
    return ::std::memcmp(first1, first2, (last1 - first1) * sizeof(T)) != 0;
}

/**
 * Compares two ranges of the same size for non-equality.
 * @tparam T Underlying value type.
 * @param first1 Pointer to a first item of the first range.
 * @param last1 Pointer to an item right after the last item of the first range.
 * @param first2 Pointer to a first item of the second range.
 * @return true if elements in the range are not equal, false otherwise.
 */
template<class T, is_non_trivial_type<T> = true>
bool are_ranges_not_equal(const T* first1, const T* last1, const T* first2) noexcept
{
    for (; first1 != last1; ++first1, ++first2) {
        if (*first1 == *first2) return false;
    }
    return true;
}

/**
 * Fakes default initialization of the range elements.
 * @tparam Allocator Allocator type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 * @param allocator Allocator object.
 */
template<class Allocator, is_trivially_constructible_type<typename Allocator::value_type> = true>
inline void default_initialize_range([[maybe_unused]]
                                     typename ::std::allocator_traits<Allocator>::pointer first,
        [[maybe_unused]] typename ::std::allocator_traits<Allocator>::pointer last,
        [[maybe_unused]] Allocator& allocator)
{
}

/**
 * Default initializes range elements.
 * @tparam Allocator Allocator type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 * @param allocator Allocator object.
 */
template<class Allocator,
        is_not_trivially_constructible_type<typename Allocator::value_type> = true>
inline void default_initialize_range(typename ::std::allocator_traits<Allocator>::pointer first,
        typename ::std::allocator_traits<Allocator>::pointer last, Allocator& allocator)
{
    for (; first != last; ++first)
        ::std::allocator_traits<Allocator>::construct(allocator, std::addressof(*first));
}

/**
 * Fakes default initialization of the range elements.
 * @tparam T Underlying value type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 */
template<class T, is_trivially_constructible_type<T> = true>
inline void default_initialize_range([[maybe_unused]] T* first, [[maybe_unused]] T* last)
{
}

/**
 * Default initializes range elements.
 * @tparam T Underlying value type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 */
template<class T, is_not_trivially_constructible_type<T> = true>
void default_initialize_range(T* first, T* last)
{
    for (; first != last; ++first)
        first->T();
}

/**
 * Fakes destroying elements in the range.
 * @tparam Allocator Allocator type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 * @param allocator Allocator object.
 */
template<class Allocator, is_trivially_destructible_type<typename Allocator::value_type> = true>
inline void destroy_range([[maybe_unused]]
                          typename ::std::allocator_traits<Allocator>::pointer first,
        [[maybe_unused]] typename ::std::allocator_traits<Allocator>::pointer last,
        [[maybe_unused]] Allocator& allocator)
{
}

/**
 * Destroys elements in the range using allocator.
 * @tparam Allocator Allocator type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 * @param allocator Allocator object.
 */
template<class Allocator, is_not_trivially_destructible_type<typename Allocator::value_type> = true>
inline void destroy_range([[maybe_unused]]
                          typename ::std::allocator_traits<Allocator>::pointer first,
        [[maybe_unused]] typename ::std::allocator_traits<Allocator>::pointer last,
        Allocator& allocator)
{
    for (; first != last; ++first)
        ::std::allocator_traits<Allocator>::destroy(allocator, std::addressof(*first));
}

/**
 * Fakes destroying elements in the range.
 * @tparam T Underlying value type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 */
template<class T, is_trivially_destructible_type<T> = true>
inline void destroy_range([[maybe_unused]] T* first, [[maybe_unused]] T* last)
{
}

/**
 * Destroys elements in the range directly using destructor.
 * @tparam T Underlying value type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 */
template<class T, is_not_trivially_destructible_type<T> = true>
void destroy_range(T* first, T* last)
{
    for (; first != last; ++first)
        first->~T();
}

}  // namespace stdext::detail
