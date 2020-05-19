// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class UserAccessKey;

/** UserAccessKey shared pointer shortcut type */
using UserAccessKeyPtr = std::shared_ptr<UserAccessKey>;

/** Constant user access key shared pointer shortcut type */
using ConstUserAccessKeyPtr = std::shared_ptr<const UserAccessKey>;

}  // namespace siodb::iomgr::dbengine
