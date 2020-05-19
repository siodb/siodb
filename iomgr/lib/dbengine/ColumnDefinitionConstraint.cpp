// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDefinitionConstraint.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

ColumnDefinitionConstraint::ColumnDefinitionConstraint(
        ColumnDefinition& columnDefinition, const ConstraintPtr& constraint)
    : m_columnDefinition(columnDefinition)
    , m_id(m_columnDefinition.getColumn()
                      .getTable()
                      .getDatabase()
                      .generateNextColumnDefinitionConstraintId(
                              m_columnDefinition.getTable().isSystemTable()))
    , m_constraint(constraint)
{
}

ColumnDefinitionConstraint::ColumnDefinitionConstraint(ColumnDefinition& columnDefinition,
        const ColumnDefinitionConstraintRecord& columnDefinitionConstraintRecord)
    : m_columnDefinition(
            validateColumnDefinition(columnDefinition, columnDefinitionConstraintRecord))
    , m_id(columnDefinitionConstraintRecord.m_id)
    , m_constraint(m_columnDefinition.getTable().getConstraintChecked(
              &m_columnDefinition.getColumn(), columnDefinitionConstraintRecord.m_constraintId))
{
}

// ----- internals -----

ColumnDefinition& ColumnDefinitionConstraint::validateColumnDefinition(
        ColumnDefinition& columnDefinition,
        const ColumnDefinitionConstraintRecord& columnDefinitionConstraintRecord)
{
    if (columnDefinitionConstraintRecord.m_columnDefinitionId == columnDefinition.getId())
        return columnDefinition;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnDefinitionConstraintColumnDefinition,
            columnDefinitionConstraintRecord.m_id,
            columnDefinitionConstraintRecord.m_columnDefinitionId,
            columnDefinition.getDatabaseName(), columnDefinition.getTableName(),
            columnDefinition.getColumnName(), columnDefinition.getId(),
            columnDefinition.getDatabaseUuid(), columnDefinition.getTableId(),
            columnDefinition.getColumnId());
}

}  // namespace siodb::iomgr::dbengine
