// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnConstraint.h"
#include "parser/expr/Expression.h"

namespace siodb::iomgr::dbengine {

/** DEFAULT SQL constraint with constant value */
class DefaultValueConstraint : public ColumnConstraint {
public:
    /**
     * Initializes object of class DefaultValueConstraint for new constraint.
     * @param column Column to which this constaint applies.
     * @param name Constraint name.
     * @param constraintDefinition Constraint definition.
     */
    DefaultValueConstraint(Column& column, const std::string& name,
            const ConstConstraintDefinitionPtr& constraintDefinition)
        : ColumnConstraint(column, name, constraintDefinition, ConstraintType::kDefaultValue)
    {
    }

    /**
     * Initializes object of class DefaultValueConstraint for existing constraint.
     * @param column Column to which constaint should apply.
     * @param constraintRecord Constraint registry record.
     */
    DefaultValueConstraint(Column& column, const ConstraintRecord& constraintRecord)
        : ColumnConstraint(column, constraintRecord, ConstraintType::kDefaultValue)
    {
    }
};

}  // namespace siodb::iomgr::dbengine
