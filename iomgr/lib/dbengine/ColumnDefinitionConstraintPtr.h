// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class ColumnDefinitionConstraint;

/** Column definition constraint shared pointer shortcut type */
using ColumnDefinitionConstraintPtr = std::shared_ptr<ColumnDefinitionConstraint>;

/** Constant column definition constraint shared pointer shortcut type */
using ConstColumnDefinitionConstraintPtr = std::shared_ptr<const ColumnDefinitionConstraint>;

}  // namespace siodb::iomgr::dbengine
