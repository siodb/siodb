// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstddef>

namespace stdext::detail {

/**
 * Throws std::out_of_range error with specified prefix text.
 * @param prefix Prefix text.
 * @param n Actual value.
 * @param limit Limit value.
 * @throw std::out_of_range
 */
[[noreturn]] void throw_out_of_range_error(const char* prefix, std::size_t n, std::size_t limit);

/**
 * Throws std::length_error with specified prefix text.
 * @param prefix Prefix text.
 * @param n Actual value.
 * @param limit Limit value.
 * @throw std::length_error
 */
[[noreturn]] void throw_length_error(const char* prefix, std::size_t n, std::size_t limit);

}  // namespace stdext::detail
