// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class UserToken;

/** User shared pointer shortcut type */
using UserTokenPtr = std::shared_ptr<UserToken>;

/** Constant user shared pointer shortcut type */
using ConstUserTokenPtr = std::shared_ptr<const UserToken>;

}  // namespace siodb::iomgr::dbengine
