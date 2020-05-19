// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class Database;

/** Database shared pointer shortcut type */
using DatabasePtr = std::shared_ptr<Database>;

/** Constant database shared pointer shortcut type */
using ConstDatabasePtr = std::shared_ptr<const Database>;

}  // namespace siodb::iomgr::dbengine
