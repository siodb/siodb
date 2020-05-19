// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

class ConstraintDefinition;

/** Constraint definition shared pointer shortcut type */
using ConstraintDefinitionPtr = std::shared_ptr<ConstraintDefinition>;

/** Constant constraint definition shared pointer shortcut type */
using ConstConstraintDefinitionPtr = std::shared_ptr<const ConstraintDefinition>;

}  // namespace siodb::iomgr::dbengine
