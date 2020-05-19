// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class ColumnDefinitionConstraintList;

/** Column definition constraint list shared pointer shortcut type */
using ColumnDefinitionConstraintListPtr = std::shared_ptr<ColumnDefinitionConstraintList>;

/** Constant column definition constraint list shared pointer shortcut type */
using ConstColumnDefinitionConstraintListPtr =
        std::shared_ptr<const ColumnDefinitionConstraintList>;

}  // namespace siodb::iomgr::dbengine
