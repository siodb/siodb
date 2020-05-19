// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class ColumnDataBlock;

/** Data block shared pointer shortcut type */
using ColumnDataBlockPtr = std::shared_ptr<ColumnDataBlock>;

/** Constant data block shared pointer shortcut type */
using ConstColumnDataBlockPtr = std::shared_ptr<const ColumnDataBlock>;

}  // namespace siodb::iomgr::dbengine
