// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine::requests {

class DBEngineRequest;

/** Database engine request shared pointer shortcut type */
using DBEngineRequestPtr = std::shared_ptr<DBEngineRequest>;

/** Read-only database engine request shared pointer shortcut type */
using ConstDBEngineRequestPtr = std::shared_ptr<const DBEngineRequest>;

}  // namespace siodb::iomgr::dbengine::requests
