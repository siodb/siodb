// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/BinaryValue.h>

// STL headers
#include <string>

namespace siodb::iomgr::dbengine::crypto {

/**
 * Generates cipher key of the specified length.
 * @param keyLength Key length in bits. Must be nonzero, multiple of 8, must not exceeed 512.
 * @param seed Seed string.
 * @throw std::invalid_argument if key length is invalid.
 * @throw std::runtime_error if there was error reading entropy data.
 */
BinaryValue generateCipherKey(unsigned keyLength, const std::string& seed);

/**
 * Generates cipher key of the specified length.
 * @param keyLength Key length in bits. Must be nonzero, multiple of 8, must not exceeed 512.
 * @param seed Seed string.
 * @throw std::invalid_argument if key length is invalid.
 * @throw std::runtime_error if there was error reading entropy data.
 */
BinaryValue generateCipherKey(unsigned keyLength, const char* seed);

/**
 * Generates cipher key of the specified length.
 * @param keyLength Key length in bits. Must be nonzero, multiple of 8, must not exceeed 512.
 * @param seed Seed data.
 * @param seedLength Seed length.
 * @throw std::invalid_argument if key length is invalid.
 * @throw std::runtime_error if there was error reading entropy data.
 */
BinaryValue generateCipherKey(unsigned keyLength, const void* seed, std::size_t seedLength);

}  // namespace siodb::iomgr::dbengine::crypto
