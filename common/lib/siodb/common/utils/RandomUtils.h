// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstddef>
#include <cstdint>

namespace siodb::utils {

/**
 * Fills buffer with bytes read from /dev/urandom.
 * @param buffer A buffer
 * @param length Buffer length.
 * @throws std::runtime_error of exception occurs.
 */
void getRandomBytes(std::uint8_t* buffer, std::size_t length);

/**
 * Fills buffer with bytes read from /dev/random.
 * @param buffer A buffer
 * @param length Buffer length.
 * @throws std::runtime_error of exception occurs.
 */
void getStrongRandomBytes(std::uint8_t* buffer, std::size_t length);

}  // namespace siodb::utils
