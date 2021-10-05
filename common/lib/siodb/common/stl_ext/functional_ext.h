// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Parts of the following content are derived from:
// https://raw.githubusercontent.com/HowardHinnant/hash_append/master/n3876.h
//-------------------------------- n3876.h -------------------------------------
//
// This software is in the public domain.  The only restriction on its use is
// that no one can remove it from the public domain by claiming ownership of it,
// including the original authors.
//
// There is no warranty of correctness on the software contained herein.  Use
// at your own risk.
//
//------------------------------------------------------------------------------

// This is an implementation of the N3876 proposal found at:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3876.pdf

// Project headers
#include "type_traits_ext.h"

// STL headers
#include <functional>

namespace stdext {

/**
 * Hash combination for the no parameters case.
 * @param[inout] seed Hash seed and result.
 */
inline void hash_combine([[maybe_unused]] std::size_t& seed) noexcept
{
}

/**
 * Hash combination implementation for a single value which takes into account size of std::size_t.
 * @see https://github.com/HowardHinnant/hash_append/issues/7
 * @see https://github.com/HowardHinnant/hash_append/issues/7#issuecomment-629460500
 * @tparam T Value type.
 * @param[inout] seed Hash seed and result.
 * @param value Value to combine current hash with.
 */
template<class T>
inline void hash_combine(std::size_t& seed, const T& value)
{
    if constexpr (sizeof(std::size_t) == 1) {
        seed ^= std::hash<T> {}(value) + 0x9eU + (seed << 1) + (seed >> 1);
    } else if constexpr (sizeof(std::size_t) == 2) {
        seed ^= std::hash<T> {}(value) + 0x9e37U + (seed << 3) + (seed >> 1);
    } else if constexpr (sizeof(std::size_t) == 4) {
        seed ^= std::hash<T> {}(value) + 0x9e3779b9U + (seed << 6) + (seed >> 2);
    } else if constexpr (sizeof(std::size_t) == 8) {
        seed ^= std::hash<T> {}(value) + 0x9e3779b97f4a7c15LLU + (seed << 12) + (seed >> 4);
    } else {
        static_assert(dependent_false<T>::value,
                "hash_combine is not implemented for the current size of std::size_t");
    }
}

/**
 * Hash combination implementation for the multiple values.
 * @tparam T First value type.
 * @tparam Types Types of the rest of values.
 * @param[inout] seed Hash seed and result.
 * @param value First value to combine current hash with.
 * @param args Rest of values to combine current hash with.
 */
template<typename T, typename... Types>
inline void hash_combine(std::size_t& seed, const T& value, const Types&... args)
{
    hash_combine(seed, value);
    hash_combine(seed, args...);
}

/**
 * Computes combined hash for the one or more values starting from zero seed.
 * @tparam Types Types of the rest of values.
 * @param args Value to combine hashes.
 * @return Combined hash value.
 */
template<typename... Types>
inline std::size_t hash_val(const Types&... args)
{
    std::size_t seed = 0;
    hash_combine(seed, args...);
    return seed;
}

}  // namespace stdext

namespace std {

/**
 * Generic std::hash specialization for std::pair.
 * @tparam First Type of pair's first member.
 * @tparam Second Type of pair's second member.
 */
template<class First, class Second>
struct hash<pair<First, Second>> {
    /**
     * Computes hash value for the std::pair object.
     * @param p Pair object.
     * @return Hash value.
     */
    std::size_t operator()(const pair<First, Second>& p) const
    {
        return stdext::hash_val(p.first, p.first);
    }
};

namespace detail {

/**
 * Recursive implementation of the tuple hash.
 * @see https://stackoverflow.com/a/7115547/1540501
 */
template<class Tuple, std::size_t Index = std::tuple_size<Tuple>::value - 1>
struct TupleHashImpl {
    static void apply(std::size_t& seed, const Tuple& t)
    {
        TupleHashImpl<Tuple, Index - 1>::apply(seed, t);
        stdext::hash_combine(seed, std::get<Index>(t));
    }
};

/**
 * Recursive implementation of the tuple hash for the first tuple member.
 * @see https://stackoverflow.com/a/7115547/1540501
 */
template<class Tuple>
struct TupleHashImpl<Tuple, 0> {
    static void apply(std::size_t& seed, const Tuple& t)
    {
        stdext::hash_combine(seed, std::get<0>(t));
    }
};

}  // namespace detail

/**
 * Generic std::hash specialization for std::tuple.
 * @tparam Types List of std::tuple member types.
 * @see https://stackoverflow.com/a/7115547/1540501
 */
template<class... Types>
struct hash<tuple<Types...>> {
    /**
     * Computes hash value for the std::tuple object.
     * @param t Tuple object.
     * @return Hash value.
     */
    std::size_t operator()(const tuple<Types...>& t) const
    {
        std::size_t seed = 0;
        detail::TupleHashImpl<std::tuple<Types...>>::apply(seed, t);
        return seed;
    }
};

}  // namespace std
