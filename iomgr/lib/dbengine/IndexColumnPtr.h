// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class IndexColumn;

/** Index column shared pointer shortcut type */
using IndexColumnPtr = std::shared_ptr<IndexColumn>;

/** Constant index column shared pointer shortcut type */
using ConstIndexColumnPtr = std::shared_ptr<const IndexColumn>;

}  // namespace siodb::iomgr::dbengine
