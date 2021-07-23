// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnSet.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnSetColumn.h"
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/log/Log.h>

namespace siodb::iomgr::dbengine {

ColumnSet::ColumnSet(Table& table, Columns&& columns)
    : m_table(table)
    , m_id(m_table.getDatabase().generateNextColumnSetId(m_table.isSystemTable()))
    , m_columns(std::move(columns))
    , m_openForModification(true)
{
}

ColumnSet::ColumnSet(Table& table, const ColumnSetRecord& columnSetRecord)
    : m_table(validateTable(table, columnSetRecord))
    , m_id(columnSetRecord.m_id)
    , m_columns(makeColumns(columnSetRecord))
    , m_columnIdToPoisitionMapping(createColumIdToPoisitionMapping())
    , m_openForModification(false)
{
}

void ColumnSet::markClosedForModification()
{
    LOG_DEBUG << "Closing column set " << m_table.makeDisplayName() << '.' << m_id;
    if (!m_openForModification) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnSetAlreadyClosedForModification,
                m_table.getDatabaseName(), m_table.getName(), m_id, m_table.getDatabaseUuid(),
                m_table.getId());
    }
    m_columnIdToPoisitionMapping = createColumIdToPoisitionMapping();
    m_openForModification = false;
}

std::uint64_t ColumnSet::addColumn(const ColumnDefinition& columnDefinition)
{
    if (!m_openForModification) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotModifyClosedColumnSet,
                m_table.getDatabaseName(), m_table.getName(), m_id, m_table.getDatabaseUuid(),
                m_table.getId());
    }
    m_columns.push_back(std::make_shared<ColumnSetColumn>(*this, columnDefinition));
    return m_columns.back()->getId();
}

std::uint32_t ColumnSet::findColumnPosition(std::uint64_t columnId) const
{
    const auto it = m_columnIdToPoisitionMapping.find(columnId);
    if (it != m_columnIdToPoisitionMapping.cend()) return it->second;
    throwDatabaseError(IOManagerMessageId::kErrorColumnDoesNotBelongToColumnSet,
            m_table.getDatabaseName(), m_table.getName(), m_id, m_table.getDatabaseUuid(),
            m_table.getId(), columnId);
}

// --- internals ---

Table& ColumnSet::validateTable(Table& table, const ColumnSetRecord& columnSetRecord)
{
    if (columnSetRecord.m_tableId == table.getId()) return table;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnSetTable, columnSetRecord.m_id,
            columnSetRecord.m_tableId, table.getDatabaseName(), table.getName(),
            table.getDatabaseUuid(), table.getId());
}

ColumnSet::Columns ColumnSet::makeColumns(const ColumnSetRecord& columnSetRecord)
{
    Columns columns;
    for (const auto& r : columnSetRecord.m_columns.byId())
        columns.push_back(std::make_shared<ColumnSetColumn>(*this, r));
    return columns;
}

ColumnSet::ColumnIdToPoisitionMapping ColumnSet::createColumIdToPoisitionMapping() const
{
    ColumnIdToPoisitionMapping m;
    m.reserve(m_columns.size());
    std::uint32_t position = 0;
    for (const auto& columnSetColumn : m_columns)
        m.emplace(columnSetColumn->getColumnId(), position++);
    return m;
}

}  // namespace siodb::iomgr::dbengine
