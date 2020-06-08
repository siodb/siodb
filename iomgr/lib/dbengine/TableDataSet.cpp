// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TableDataSet.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Database.h"
#include "DatabaseObjectName.h"
#include "Index.h"
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

TableDataSet::TableDataSet(const TablePtr& table, const std::string& tableAlias)
    : DataSet(tableAlias)
    , m_table(table)
    , m_tableColumns(m_table->getColumnsOrderedByPosition())
    , m_masterColumn(m_table->getMasterColumn())
    , m_masterColumnIndex(m_masterColumn->getMasterColumnMainIndex())
    , m_currentKey(nullptr)
    , m_nextKey(nullptr)
{
}

const std::string& TableDataSet::getName() const noexcept
{
    return m_table->getName();
}

const Variant& TableDataSet::getColumnValue(std::size_t index)
{
    // Check index
    const bool hasValue = m_valueReadMask.get(index);
    // Normally should never happen
    if (!m_hasCurrentRow) throw std::runtime_error("No more rows");
    // Read column value
    if (!hasValue) readColumnValue(index);
    return m_values[index];
}

ColumnDataType TableDataSet::getColumnDataType(std::size_t columnIndex) const
{
    return m_tableColumns.at(m_columnInfos.at(columnIndex).m_posInTable)->getDataType();
}

const std::vector<Variant>& TableDataSet::getCurrentRow()
{
    // Normally should never happen
    if (!m_hasCurrentRow) throw std::runtime_error("No more rows 2");

    // Read all column values
    for (std::size_t i = 0, n = m_columnInfos.size(); i != n; ++i) {
        if (!m_valueReadMask.get(i)) readColumnValue(i);
    }

    return m_values;
}

std::optional<std::uint32_t> TableDataSet::getDataSourceColumnPosition(
        const std::string& name) const
{
    return m_table->findColumnPosition(name);
}

std::uint32_t TableDataSet::getDataSourceId() const noexcept
{
    return m_table->getId();
}

void TableDataSet::resetCursor()
{
    // Obtain min and max TRID
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (m_masterColumnIndex->getMinKey(m_key) && m_masterColumnIndex->getMaxKey(&m_key[8])) {
        ::pbeDecodeUInt64(m_key, &minTrid);
        ::pbeDecodeUInt64(&m_key[8], &maxTrid);
    }

    m_currentKey = m_key;
    m_nextKey = &m_key[8];

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted,
                m_table->getDatabaseName(), m_table->getName(), m_table->getDatabaseUuid(),
                m_table->getId(), 1);
    }

    m_valueReadMask.resize(m_columnInfos.size());
    m_values.resize(m_columnInfos.size());

    m_hasCurrentRow = (maxTrid > 0);
    if (m_hasCurrentRow) {
        readMasterColumnRecord();
        m_valueReadMask.fill(false);
    }
}

bool TableDataSet::moveToNextRow()
{
    m_hasCurrentRow = m_masterColumnIndex->findNextKey(m_currentKey, m_nextKey);
    std::swap(m_currentKey, m_nextKey);
    if (m_hasCurrentRow) {
        readMasterColumnRecord();
        m_valueReadMask.fill(false);
    }
    return m_hasCurrentRow;
}

void TableDataSet::deleteCurrentRow(std::uint32_t currentUserId)
{
    const TransactionParameters tp(
            currentUserId, m_table->getDatabase().generateNextTransactionId());
    m_table->deleteRow(m_currentMcr, m_currentMcrAddress, tp);
}

void TableDataSet::updateCurrentRow(std::vector<Variant>&& values,
        const std::vector<std::size_t>& columnPositions, std::uint32_t currentUserId)
{
    const TransactionParameters tp(
            currentUserId, m_table->getDatabase().generateNextTransactionId());
    m_table->updateRow(m_currentMcr, m_currentMcrAddress, std::move(values), columnPositions, tp);
}

// ---- internals ----

void TableDataSet::readMasterColumnRecord()
{
    std::uint8_t value[12];

    // Obtain master column record address
    if (m_masterColumnIndex->findValue(m_currentKey, value, 1) != 1) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted,
                m_table->getDatabaseName(), m_table->getName(), m_table->getDatabaseUuid(),
                m_table->getId(), 2);
    }

    ColumnDataAddress mcrAddr;
    mcrAddr.pbeDeserialize(value, sizeof(value));

    // Read and validate master column record
    m_masterColumn->readMasterColumnRecord(mcrAddr, m_currentMcr);

    // + TRID
    if (m_currentMcr.getColumnCount() + 1 != m_table->getColumnCount()) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                m_table->getDatabaseName(), m_table->getName(), m_table->getDatabaseUuid(),
                m_table->getId(), mcrAddr.getBlockId(), mcrAddr.getOffset(),
                m_table->getColumnCount(), m_currentMcr.getColumnCount() + 1);
    }

    m_currentMcrAddress = mcrAddr;
}

void TableDataSet::readColumnValue(std::size_t index)
{
    auto& value = m_values.at(index);
    const auto pos = m_columnInfos.at(index).m_posInTable;
    auto& column = m_tableColumns.at(pos);

    if (column->isMasterColumn())
        value = m_currentMcr.getTableRowId();
    else {
        column->readRecord(m_currentMcr.getColumnRecords().at(pos - 1).getAddress(), value, false);
        if (value.isNull() && column->isNotNull()) {
            throwDatabaseError(IOManagerMessageId::kErrorUnexpectedNullValue,
                    m_table->getDatabaseName(), m_table->getName(), column->getName(),
                    m_currentMcr.getTableRowId());
        }
    }

    m_valueReadMask.set(index, true);
}

}  // namespace siodb::iomgr::dbengine
