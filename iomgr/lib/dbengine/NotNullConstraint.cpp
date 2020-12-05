// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "NotNullConstraint.h"

// Common project headers
#include <siodb/iomgr/shared/dbengine/parser/expr/ConstantExpression.h>

namespace siodb::iomgr::dbengine {

NotNullConstraint::NotNullConstraint(Column& column, std::string&& name,
        const ConstConstraintDefinitionPtr& constraintDefinition,
        std::optional<std::string>&& description)
    : ColumnConstraint(column, std::move(name), constraintDefinition, ConstraintType::kNotNull,
            std::move(description))
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
