// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// STL headers
#include <atomic>

// Boost headers
#include <boost/endian/conversion.hpp>

namespace siodb::utils {

// Some ideas are taken from:
// https://www.fluentcpp.com/2018/05/18/make-sfinae-pretty-2-hidden-beauty-sfinae/

/**
 * Reverses byte order of the given integer value.
 * @param value A value to be reversed.
 */
template<typename IntType, typename = std::enable_if_t<std::is_integral_v<IntType>>>
inline void reverseByteOrder(IntType& value)
{
    value = boost::endian::endian_reverse(value);
}

/**
 * Reverses byte order of the given atomic value.
 * @param value A value to be reversed.
 */
template<typename IntType>
inline void reverseByteOrder(std::atomic<IntType>& value)
{
    IntType v, rv;
    do {
        v = value.load();
        rv = boost::endian::endian_reverse(v);
    } while (v != rv && value.compare_exchange_weak(v, rv));
}

}  // namespace siodb::utils
