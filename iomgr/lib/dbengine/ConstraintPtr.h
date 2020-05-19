// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class Constraint;

/** Constraint shared pointer shortcut type */
using ConstraintPtr = std::shared_ptr<Constraint>;

/** Constant constraint shared pointer shortcut type */
using ConstConstraintPtr = std::shared_ptr<const Constraint>;

}  // namespace siodb::iomgr::dbengine
