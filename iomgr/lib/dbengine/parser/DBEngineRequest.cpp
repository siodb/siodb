// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DBEngineRequest.h"

namespace siodb::iomgr::dbengine::requests {

const std::unordered_set<ConstraintType> Constraint::m_tableOnlyConstraintTypes {
        ConstraintType::kForeignKey,
};

const std::unordered_set<ConstraintType> Constraint::m_columnOnlyConstraintTypes {
        ConstraintType::kNotNull,
        ConstraintType::kDefaultValue,
        ConstraintType::kReferences,
};

}  // namespace siodb::iomgr::dbengine::requests
