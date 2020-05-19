// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class Column;

/** Column shared pointer shortcut type */
using ColumnPtr = std::shared_ptr<Column>;

/** Constant column shared pointer shortcut type */
using ConstColumnPtr = std::shared_ptr<const Column>;

}  // namespace siodb::iomgr::dbengine
