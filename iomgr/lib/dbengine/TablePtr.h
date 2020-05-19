// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class Table;

/** Table shared pointer shortcut type */
using TablePtr = std::shared_ptr<Table>;

/** Constant table shared pointer shortcut type */
using ConstTablePtr = std::shared_ptr<const Table>;

}  // namespace siodb::iomgr::dbengine
