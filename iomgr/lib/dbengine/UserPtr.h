// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class User;

/** User shared pointer shortcut type */
using UserPtr = std::shared_ptr<User>;

/** Constant user shared pointer shortcut type */
using ConstUserPtr = std::shared_ptr<const User>;

}  // namespace siodb::iomgr::dbengine
