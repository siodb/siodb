// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <type_traits>

namespace siodb::utils {

/**
 * Comparison operator support class.
 * @tparam T Data type.
 * @see https://stackoverflow.com/a/9857292/1540501
 */
template<class T>
struct ComparisonOperations {
    friend inline int compare(const T& a, const T& b) noexcept
    {
        return a.compareTo(b);
    }

    friend inline bool operator<(const T& a, const T& b) noexcept
    {
        return compare(a, b) < 0;
    }

    friend inline bool operator<=(const T& a, const T& b) noexcept
    {
        return compare(a, b) <= 0;
    }

    friend inline bool operator==(const T& a, const T& b) noexcept
    {
        return compare(a, b) == 0;
    }

    friend inline bool operator>=(const T& a, const T& b) noexcept
    {
        return compare(a, b) >= 0;
    }

    friend inline bool operator>(const T& a, const T& b) noexcept
    {
        return compare(a, b) > 0;
    }

    friend inline bool operator!=(const T& a, const T& b) noexcept
    {
        return compare(a, b) != 0;
    }
};

/**
 * 3-way comparison function for the fundamental types.
 * @tparam T Data type.
 * @param left Left operand.
 * @param right Right operand.
 * @return -1 if @ref left less than @ref right.
 *          0 if @ref left and @ref right are equal.
 *          1 if @ref left is greater than @ref right.
 * @see https://stackoverflow.com/a/9857292/1540501
 */
template<class T>
inline typename std::enable_if<std::is_fundamental<T>::value, int>::type compare3way(
        const T left, const T right)
{
    return (left == right) ? 0 : ((left < right) ? -1 : 1);
}

/**
 * 3-way comparison function for the non-fundamental types.
 * @tparam T Data type.
 * @param left Left operand.
 * @param right Right operand.
 * @return -1 if @ref left less than @ref right.
 *          0 if @ref left and @ref right are equal.
 *          1 if @ref left is greater than @ref right.
 * @see https://stackoverflow.com/a/9857292/1540501
 */
template<class T>
inline typename std::enable_if<!std::is_fundamental<T>::value, int>::type compare3way(
        const T& left, const T& right)
{
    return (left == right) ? 0 : ((left < right) ? -1 : 1);
}

}  // namespace siodb::utils
