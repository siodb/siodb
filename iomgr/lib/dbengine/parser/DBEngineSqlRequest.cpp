// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DBEngineSqlRequest.h"

namespace siodb::iomgr::dbengine::requests {

const std::unordered_set<ConstraintType> Constraint::s_tableOnlyConstraintTypes {
        ConstraintType::kForeignKey,
};

const std::unordered_set<ConstraintType> Constraint::s_columnOnlyConstraintTypes {
        ConstraintType::kNotNull,
        ConstraintType::kDefaultValue,
        ConstraintType::kReferences,
};

}  // namespace siodb::iomgr::dbengine::requests
