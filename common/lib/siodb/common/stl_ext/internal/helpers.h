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
 * Throws std::out_of_range error with specified prefix text.
 * @param prefix Prefix text.
 * @param n Actual value.
 * @param limit Limit value.
 * @throw std::out_of_range
 */
[[noreturn]] void throwOutOfRangeError(const char* prefix, ::std::size_t n, ::std::size_t limit);

/**
 * Throws std::length_error with specified prefix text.
 * @param prefix Prefix text.
 * @param n Actual value.
 * @param limit Limit value.
 * @throw std::length_error
 */
[[noreturn]] void throwLengthError(const char* prefix, ::std::size_t n, ::std::size_t limit);

/**
 * Detects POD type.
 * @tparam T A type.
 */
template<typename T>
using IsTrivialType = ::std::enable_if_t<::std::is_trivial_v<T>, bool>;

/**
 * Detects non-POD type.
 * @tparam T A type.
 */
template<typename T>
using IsNonTrivialType = ::std::enable_if_t<!::std::is_trivial_v<T>, bool>;

/**
 * Detects move constructible type.
 * @tparam T A type.
 */
template<typename T>
using IsMoveConstructibleType = ::std::enable_if_t<::std::is_move_constructible_v<T>, bool>;

/**
 * Detects non-move contructible type.
 * @tparam T A type.
 */
template<typename T>
using IsNonMoveConstructibleType = ::std::enable_if_t<!::std::is_move_constructible_v<T>, bool>;

/**
 * Detects trivially constructible type.
 * @tparam T A type.
 */
template<typename T>
using IsTriviallyConstructibleType =
        ::std::enable_if_t<::std::is_trivially_constructible_v<T>, bool>;

/**
 * Detects non-trivially constructible type.
 * @tparam T A type.
 */
template<typename T>
using IsNonTriviallyConstructibleType =
        ::std::enable_if_t<!::std::is_trivially_constructible_v<T>, bool>;

/**
 * Detects trivially destructible type.
 * @tparam T A type.
 */
template<typename T>
using IsTriviallyDestructibleType = ::std::enable_if_t<::std::is_trivially_destructible_v<T>, bool>;

/**
 * Detects non-trivially destructible type.
 * @tparam T A type.
 */
template<typename T>
using IsNonTriviallyDestructibleType =
        ::std::enable_if_t<!::std::is_trivially_destructible_v<T>, bool>;

/**
 * Compares two ranges of the same size for equality.
 * @tparam T Underlying value type.
 * @param first1 Pointer to a first item of the first range.
 * @param last1 Pointer to an item right after the last item of the first range.
 * @param first2 Pointer to a first item of the second range.
 * @return true if elements in the range are equal, false otherwise.
 */
template<class T, IsTrivialType<T> = true>
inline bool areRangesEqual(const T* first1, const T* last1, const T* first2) noexcept
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
template<class T, IsNonTrivialType<T> = true>
bool areRangesEqual(const T* first1, const T* last1, const T* first2) noexcept
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
template<class T, IsTrivialType<T> = true>
inline bool areRangesNotEqual(const T* first1, const T* last1, const T* first2) noexcept
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
template<class T, IsNonTrivialType<T> = true>
bool areRangesNotEqual(const T* first1, const T* last1, const T* first2) noexcept
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
template<class Allocator, IsTriviallyConstructibleType<typename Allocator::value_type> = true>
inline void defaultInitializeRange([[maybe_unused]]
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
template<class Allocator, IsNonTriviallyConstructibleType<typename Allocator::value_type> = true>
inline void defaultInitializeRange(typename ::std::allocator_traits<Allocator>::pointer first,
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
template<class T, IsTriviallyConstructibleType<T> = true>
inline void defaultInitializeRange([[maybe_unused]] T* first, [[maybe_unused]] T* last)
{
}

/**
 * Default initializes range elements.
 * @tparam T Underlying value type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 */
template<class T, IsNonTriviallyConstructibleType<T> = true>
void defaultInitializeRange(T* first, T* last)
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
template<class Allocator, IsTriviallyDestructibleType<typename Allocator::value_type> = true>
inline void destroyRange([[maybe_unused]]
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
template<class Allocator, IsNonTriviallyDestructibleType<typename Allocator::value_type> = true>
inline void destroyRange([[maybe_unused]]
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
template<class T, IsTriviallyDestructibleType<T> = true>
inline void destroyRange([[maybe_unused]] T* first, [[maybe_unused]] T* last)
{
}

/**
 * Destroys elements in the range directly using destructor.
 * @tparam T Underlying value type.
 * @param first Pointer to a first item of the range.
 * @param last Pointer to an item right after the last item of the range.
 */
template<class T, IsNonTriviallyDestructibleType<T> = true>
void destroyRange(T* first, T* last)
{
    for (; first != last; ++first)
        first->~T();
}

}  // namespace stdext::detail
