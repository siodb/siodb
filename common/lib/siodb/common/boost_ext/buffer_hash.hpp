// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../stl_ext/buffer.h"

// Boost headers
#include <boost/functional/hash.hpp>

namespace stdext {

/**
 * Specialization of the boost::hash_value() for the stdext::buffer.
 * @tparam Element
 * @param buffer Buffer object.
 * @return Hash value.
 */
template<class Element>
std::size_t hash_value(const buffer<Element>& buffer)
{
    std::size_t result = 0;
    auto p = buffer.data();
    const auto e = p + buffer.size();
    while (p != e)
        boost::hash_combine(result, *p++);
    return result;
}

}  // namespace stdext
