// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class Instance;

/** Insance shared pointer shortcut type */
using InstancePtr = std::shared_ptr<Instance>;

/** Constant instance shared pointer shortcut type */
using ConstInstancePtr = std::shared_ptr<const Instance>;

}  // namespace siodb::iomgr::dbengine
