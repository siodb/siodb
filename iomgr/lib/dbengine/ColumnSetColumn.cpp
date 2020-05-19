// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnSetColumn.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

ColumnSetColumn::ColumnSetColumn(ColumnSet& columnSet, const ColumnDefinition& columnDefinition)
    : m_columnSet(columnSet)
    , m_id(m_columnSet.getTable().getDatabase().generateNextColumnSetColumnId(
              m_columnSet.getTable().isSystemTable()))
    , m_columnDefinitionId(columnDefinition.getId())
    , m_columnId(columnDefinition.getColumnId())
{
}

ColumnSetColumn::ColumnSetColumn(
        ColumnSet& columnSet, const ColumnSetColumnRecord& columnSetColumnRecord)
    : m_columnSet(validateColumnSet(columnSet, columnSetColumnRecord))
    , m_id(columnSetColumnRecord.m_id)
    , m_columnDefinitionId(columnSetColumnRecord.m_columnDefinitionId)
    , m_columnId(columnSetColumnRecord.m_columnId)
{
}

// ----- internals -----

ColumnSet& ColumnSetColumn::validateColumnSet(
        ColumnSet& columnSet, const ColumnSetColumnRecord& columnSetColumnRecord)
{
    if (columnSetColumnRecord.m_columnSetId == columnSet.getId()) return columnSet;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnSetColumnColumnSet,
            columnSetColumnRecord.m_id, columnSetColumnRecord.m_columnDefinitionId,
            columnSet.getDatabaseName(), columnSet.getTableName(), columnSet.getId(),
            columnSet.getDatabaseUuid(), columnSet.getTableId());
}

}  // namespace siodb::iomgr::dbengine
