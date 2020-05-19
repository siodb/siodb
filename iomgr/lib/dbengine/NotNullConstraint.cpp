// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "NotNullConstraint.h"

// Project headers
#include "parser/expr/ConstantExpression.h"

namespace siodb::iomgr::dbengine {

NotNullConstraint::NotNullConstraint(Column& column, const std::string& name,
        const ConstConstraintDefinitionPtr& constraintDefinition)
    : ColumnConstraint(column, name, constraintDefinition, ConstraintType::kNotNull)
    , m_notNull(dynamic_cast<const requests::ConstantExpression&>(
              m_constraintDefinition->getExpression())
                        .getValue()
                        .getBool())
{
}

NotNullConstraint::NotNullConstraint(Column& column, const ConstraintRecord& constraintRecord)
    : ColumnConstraint(column, constraintRecord, ConstraintType::kNotNull)
    , m_notNull(dynamic_cast<const requests::ConstantExpression&>(
              m_constraintDefinition->getExpression())
                        .getValue()
                        .getBool())
{
}

}  // namespace siodb::iomgr::dbengine
