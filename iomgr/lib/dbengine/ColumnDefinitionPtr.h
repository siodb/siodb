// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class ColumnDefinition;

/** Column definition shared pointer shortcut type */
using ColumnDefinitionPtr = std::shared_ptr<ColumnDefinition>;

/** Constant column definition shared pointer shortcut type */
using ConstColumnDefinitionPtr = std::shared_ptr<const ColumnDefinition>;

}  // namespace siodb::iomgr::dbengine
