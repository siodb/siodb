// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class ColumnSetColumn;

/** Column set column shared pointer shortcut type */
using ColumnSetColumnPtr = std::shared_ptr<ColumnSetColumn>;

/** Constant column set shared pointer shortcut type */
using ConstColumnSetColumnPtr = std::shared_ptr<const ColumnSetColumn>;

}  // namespace siodb::iomgr::dbengine
