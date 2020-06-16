// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/CipherPtr.h>

// STL headers
#include <string>

namespace siodb::iomgr::dbengine::crypto {

/**
 * Returns specified cipher object.
 * @param cipherId Cipher identification string.
 * @return Cipher object, nullptr if cipher is "none"
 * @throw DatabaseError if cipher doesn't exist.
 */
CipherPtr getCipher(const std::string& cipherId);

}  // namespace siodb::iomgr::dbengine::crypto
