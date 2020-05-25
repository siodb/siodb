// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnConstraint.h"

namespace siodb::iomgr::dbengine {

/** NOT NULL SQL constraint. */
class NotNullConstraint : public ColumnConstraint {
public:
    /**
     * Initializes object of class NotNullConstraint for new constraint.
     * @param column Column to which this constaint applies.
     * @param name Constraint name.
     * @param constraintDefinition Constraint definition.
     * @param description Constraint description.
     */
    NotNullConstraint(Column& column, std::string&& name,
            const ConstConstraintDefinitionPtr& constraintDefinition,
            std::optional<std::string>&& description);

    /**
     * Initializes object of class NotNullConstraint for existing constraint.
     * @param column Column to which constaint should apply.
     * @param constraintRecord Constraint registry record.
     */
    NotNullConstraint(Column& column, const ConstraintRecord& constraintRecord);

    /**
     * Returns indication that NULL value is not allowed.
     * @return true if NULL value is not allowed, false otherwise.
     */
    bool isNotNull() const noexcept
    {
        return m_notNull;
    }

private:
    /** Indicates that NULL value is not allowed */
    const bool m_notNull;
};

}  // namespace siodb::iomgr::dbengine
