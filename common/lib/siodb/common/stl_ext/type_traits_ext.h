// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <type_traits>

namespace stdext {

/**
 * False type which depends on type parameter.
 * @tparam T Object type.
 * @see https://github.com/HowardHinnant/hash_append/issues/7#issuecomment-629460500
 */
template<class T>
struct dependent_false : std::false_type {
};

/**
 * True type which depends on type parameter.
 * @tparam T Object type.
 */
template<class T>
struct dependent_true : std::true_type {
};

}  // namespace stdext
