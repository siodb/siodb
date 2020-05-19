// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class ColumnSet;

/** Column set shared pointer shortcut type */
using ColumnSetPtr = std::shared_ptr<ColumnSet>;

/** Constant column set shared pointer shortcut type */
using ConstColumnSetPtr = std::shared_ptr<const ColumnSet>;

}  // namespace siodb::iomgr::dbengine
