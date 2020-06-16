// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine::crypto {

class Cipher;

/** Cipher shared pointer shortcut type */
using CipherPtr = std::shared_ptr<Cipher>;

/** Constant cipher shared pointer shortcut type */
using ConstCipherPtr = std::shared_ptr<const Cipher>;

}  // namespace siodb::iomgr::dbengine::crypto
