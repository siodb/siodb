// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Column.h"
#include "Constraint.h"

namespace siodb::iomgr::dbengine {

/** Constraint for the single column. */
class ColumnConstraint : public Constraint {
protected:
    /**
     * Initializes object of class ColumnConstraint for the new constraint.
     * @param column Column to which this constaint applies.
     * @param name Constraint name.
     * @param constraintDefinition Constraint definition.
     * @param expectedType Expected constaint type.
     */
    ColumnConstraint(Column& column, const std::string& name,
            const ConstConstraintDefinitionPtr& constraintDefinition, ConstraintType expectedType)
        : Constraint(column.getTable(), name,
                checkConstraintType(column, name, constraintDefinition, expectedType))
        , m_column(column)
    {
    }

    /**
     * Initializes object of class ColumnConstraint for the existing constraint.
     * @param column Column to which this constaint applies.
     * @param constraintRecord Constraint registry record.
     * @param expectedType Expected constaint type.
     */
    ColumnConstraint(
            Column& column, const ConstraintRecord& constraintRecord, ConstraintType expectedType)
        : Constraint(column.getTable(), checkConstraintType(column, constraintRecord, expectedType))
        , m_column(column)
    {
    }

public:
    /**
     * Returns column to which this constraint applies.
     * @return Column object.
     */
    Column* getColumn() const noexcept override;

private:
    /**
     * Checks that constraint type matches to the required one.
     * @param column Column, to which the constraint belongs.
     * @param constraintName Constraint name.
     * @param constraintDefinition Constraint definition.
     * @param expectedType Expected constraint type.
     * @return Same constraint definition if constraint type matches required.
     * @throw DatabaseError if constraint type doesn't match.
     */
    static const ConstConstraintDefinitionPtr& checkConstraintType(const Column& column,
            const std::string& constraintName,
            const ConstConstraintDefinitionPtr& constraintDefinition, ConstraintType expectedType)
    {
        column.getDatabase().checkConstraintType(
                column.getTable(), &column, constraintName, *constraintDefinition, expectedType);
        return constraintDefinition;
    }

    /**
     * Checks that constraint type matches to the required one.
     * @param column Column, to which the constraint belongs.
     * @param constraintRecord Constraint record.
     * @param expectedType Expected constraint type.
     * @return Same constraint record if constraint type matches required.
     * @throw DatabaseError if constraint type doesn't match.
     */
    static const ConstraintRecord& checkConstraintType(const Column& column,
            const ConstraintRecord& constraintRecord, ConstraintType expectedType)
    {
        column.getDatabase().checkConstraintType(
                column.getTable(), &column, constraintRecord, expectedType);
        return constraintRecord;
    }

protected:
    /** Column to which this constraint applies */
    Column& m_column;
};

}  // namespace siodb::iomgr::dbengine
