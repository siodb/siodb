// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine::crypto {

class CipherContext;

/** Cipher context shared pointer shortcut type */
using CipherContextPtr = std::shared_ptr<CipherContext>;

/** Constant cipher context shared pointer shortcut type */
using ConstCipherContextPtr = std::shared_ptr<const CipherContext>;

}  // namespace siodb::iomgr::dbengine::crypto
