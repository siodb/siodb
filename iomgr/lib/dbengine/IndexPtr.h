// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class Index;

/** Index shared pointer shortcut type */
using IndexPtr = std::shared_ptr<Index>;

/** Constant index shared pointer shortcut type */
using ConstIndexPtr = std::shared_ptr<const Index>;

}  // namespace siodb::iomgr::dbengine
