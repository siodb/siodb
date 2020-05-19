// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDefinition.h"
#include "ColumnDefinitionConstraintPtr.h"
#include "Constraint.h"

namespace siodb::iomgr::dbengine {

/** Column definition associated constraint record */
class ColumnDefinitionConstraint {
public:
    /**
     * Initializes object of class ColumnDefinitionConstraint.
     * @param columnDefinition Column definition.
     * @param id Column definition constraint record ID.
     * @param constraint A constraint.
     */
    ColumnDefinitionConstraint(ColumnDefinition& columnDefinition, const ConstraintPtr& constraint);

    /**
     * Initializes object of class ColumnDefinitionConstraint for the exisiting
     * column definiton constraint.
     * @param columnDefinition Column definition.
     * @param columnDefinitionConstraintRecord Column definition constraint record.
     */
    ColumnDefinitionConstraint(ColumnDefinition& columnDefinition,
            const ColumnDefinitionConstraintRecord& columnDefinitionConstraintRecord);

    /**
     * Returns underlying column definition.
     * @return Column definition.
     */
    ColumnDefinition& getColumnDefinition() const noexcept
    {
        return m_columnDefinition;
    }

    /**
     * Returns column definition constraint record ID.
     * @return Record ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns underlying constraint.
     * @return Constraint object.
     */
    const Constraint& getConstraint() const noexcept
    {
        return *m_constraint;
    }

private:
    /**
     * Validates column definition.
     * @param columnDefinition Column definition to which this column definition constraint
     *              is supposed to belong to.
     * @param columnDefinitionConstraintRecord Column definition constraint registry record.
     * @return The same column definition, if it is valid.
     * @throw DatabaseError if column definition has different ID.
     */
    static ColumnDefinition& validateColumnDefinition(ColumnDefinition& columnDefinition,
            const ColumnDefinitionConstraintRecord& columnDefinitionConstraintRecord);

private:
    /** Parent column definition. */
    ColumnDefinition& m_columnDefinition;

    /** Column definition constraint record ID */
    const std::uint64_t m_id;

    /** Constraint object */
    const ConstConstraintPtr m_constraint;
};

}  // namespace siodb::iomgr::dbengine
