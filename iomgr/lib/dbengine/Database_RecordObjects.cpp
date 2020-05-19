// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Database.h"

// Project headers
#include "ColumnDefinitionConstraintList.h"
#include "ColumnSetColumn.h"
#include "IndexColumn.h"

// Common project headers
#include <siodb/common/log/Log.h>

namespace siodb::iomgr::dbengine {

void Database::recordTable(const Table& table, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording table #" << table.getId() << ' '
              << table.getName();
    std::vector<Variant> values(m_sysTablesTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = static_cast<std::int8_t>(table.getType());
    values.at(i++) = table.getName();
    values.at(i++) = table.getFirstUserTrid();
    values.at(i++) = table.getCurrentColumnSetId();
    m_sysTablesTable->insertRow(values, tp, table.getId());
    m_sysTablesTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recorded table #" << table.getId();
}

void Database::recordConstraintDefinition(
        const ConstraintDefinition& constraintDefinition, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording constraint definition #"
              << constraintDefinition.getId();
    std::vector<Variant> values(m_sysConstraintDefsTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = static_cast<std::int8_t>(constraintDefinition.getType());
    if (constraintDefinition.hasExpression()) {
        const auto& expr = constraintDefinition.getExpression();
        BinaryValue bv(expr.getSerializedSize());
        expr.serializeUnchecked(bv.data());
        values.at(i++) = std::move(bv);
    }
    m_sysConstraintDefsTable->insertRow(values, tp, constraintDefinition.getId());
    m_sysConstraintDefsTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recorded constraint definition #"
              << constraintDefinition.getId();
}

void Database::recordConstraint(const Constraint& constraint, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording constraint #" << constraint.getId() << ' '
              << constraint.getName();
    std::vector<Variant> values(m_sysConstraintsTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = constraint.getName();
    values.at(i++) = static_cast<std::int8_t>(constraint.getState());
    values.at(i++) = constraint.getTableId();
    values.at(i++) = constraint.getColumn() ? constraint.getColumn()->getId() : 0;
    values.at(i++) = constraint.getDefinitionId();
    m_sysConstraintsTable->insertRow(values, tp, constraint.getId());
    m_sysConstraintsTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recorded constraint #" << constraint.getId();
}

void Database::recordColumnSet(const ColumnSet& columnSet, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording column set #" << columnSet.getId();
    std::vector<Variant> values(m_sysColumnSetsTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = static_cast<std::uint32_t>(columnSet.getTableId());
    values.at(i++) = columnSet.getColumns().size();
    m_sysColumnSetsTable->insertRow(values, tp, columnSet.getId());
    m_sysColumnSetsTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recorded column set #" << columnSet.getId();
}

void Database::recordColumnSetColumn(
        const ColumnSetColumn& columnSetColumn, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording column set column #"
              << columnSetColumn.getId();
    std::vector<Variant> values(m_sysColumnSetColumnsTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = columnSetColumn.getColumnSet().getId();
    values.at(i++) = columnSetColumn.getColumnDefinitionId();
    m_sysColumnSetColumnsTable->insertRow(values, tp, columnSetColumn.getId());
    m_sysColumnSetColumnsTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recorded column set column #"
              << columnSetColumn.getId();
}

void Database::recordColumn(const Column& column, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording column #" << column.getId() << ' '
              << column.getTableName() << '.' << column.getName();
    std::vector<Variant> values(m_sysColumnsTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = static_cast<std::int32_t>(column.getTableId());
    values.at(i++) = static_cast<std::int8_t>(column.getDataType());
    values.at(i++) = column.getName();
    values.at(i++) = static_cast<std::int8_t>(column.getState());
    values.at(i++) = column.getDataBlockDataAreaSize();
    m_sysColumnsTable->insertRow(values, tp, column.getId());
    m_sysColumnsTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recorded column #" << column.getId();
}

void Database::recordColumnDefinition(
        const ColumnDefinition& columnDefinition, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording column definition #"
              << columnDefinition.getId() << ' ' << columnDefinition.getTableName() << '.'
              << columnDefinition.getColumnName();
    std::vector<Variant> values(m_sysColumnDefsTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = columnDefinition.getColumnId();
    values.at(i++) = columnDefinition.getConstraintCount();
    m_sysColumnDefsTable->insertRow(values, tp, columnDefinition.getId());
    m_sysColumnDefsTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recorded column definition #"
              << columnDefinition.getId();
}

void Database::recordColumnDefinitionConstraint(
        const ColumnDefinitionConstraint& columnDefinitionConstraint,
        const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording column definition constraint #"
              << columnDefinitionConstraint.getId() << ' '
              << columnDefinitionConstraint.getConstraint().getName();
    std::vector<Variant> values(m_sysColumnDefConstraintsTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = columnDefinitionConstraint.getColumnDefinition().getId();
    values.at(i++) = columnDefinitionConstraint.getConstraint().getId();
    m_sysColumnDefConstraintsTable->insertRow(values, tp, columnDefinitionConstraint.getId());
    m_sysColumnDefConstraintsTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recorded column definition constraint #"
              << columnDefinitionConstraint.getId();
}

void Database::recordIndexAndColumns(const Index& index, const TransactionParameters& tp)
{
    bool indexRecorded = false;
    std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> indexRecordingResult;
    try {
        indexRecordingResult = recordIndex(index, tp);
        indexRecorded = true;
        recordIndexColumns(index, tp);
    } catch (...) {
        if (indexRecorded) {
            m_sysIndicesTable->rollbackLastRow(
                    *indexRecordingResult.first, indexRecordingResult.second);
        }
        throw;
    }
}

std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> Database::recordIndex(
        const Index& index, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording index #" << index.getId() << ' '
              << index.getName();
    std::vector<Variant> values(m_sysIndicesTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = static_cast<std::int16_t>(index.getType());
    values.at(i++) = index.isUnique();
    values.at(i++) = index.getName();
    values.at(i++) = index.getTableId();
    values.at(i++) = index.getDataFileSize();
    auto result = m_sysIndicesTable->insertRow(values, tp, index.getId());
    m_sysIndicesTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recorded index #" << index.getId();
    return result;
}

void Database::recordIndexColumns(const Index& index, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording index columns for the index #"
              << index.getId() << ' ' << index.getName();
    std::vector<std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>>> recordedColumns;
    const auto columnCount = index.getColumns().size();
    recordedColumns.reserve(columnCount);
    try {
        for (std::size_t i = 0; i < columnCount; ++i)
            recordedColumns.push_back(recordIndexColumn(index, i, tp));
    } catch (...) {
        for (auto rit = recordedColumns.rbegin(); rit != recordedColumns.rend(); ++rit) {
            m_sysIndexColumnsTable->rollbackLastRow(*rit->first, rit->second);
        }
        throw;
    }
    LOG_DEBUG << "Database " << m_name << ": Recording index columns for the index #"
              << index.getId();
}

std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> Database::recordIndexColumn(
        const Index& index, std::size_t columnIndex, const TransactionParameters& tp)
{
    const auto& indexColumn = *index.getColumns().at(columnIndex);
    LOG_DEBUG << "Database " << m_name << ": Recording index column [" << columnIndex << "] #"
              << indexColumn.getId() << " for the index #" << index.getId() << ' '
              << index.getName();
    std::vector<Variant> values(m_sysIndexColumnsTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = index.getId();
    values.at(i++) = indexColumn.getColumnDefinitionId();
    values.at(i++) = indexColumn.isDescendingSortOrder();
    auto result = m_sysIndexColumnsTable->insertRow(values, tp, index.getId());
    m_sysIndicesTable->flushIndices();
    LOG_DEBUG << "Database " << m_name << ": Recording index column [" << columnIndex << "] #"
              << indexColumn.getId();
    return result;
}

void Database::recordTableDefinition(const Table& table, const TransactionParameters& tp)
{
    // Record table
    recordTable(table, tp);

    // Record columns
    const auto columns = table.getColumnsOrderedByPosition();
    for (const auto& column : columns)
        recordColumn(*column, tp);

    // Record column definitions
    for (const auto& column : columns)
        recordColumnDefinition(*column->getCurrentColumnDefinition(), tp);

    // Record column set
    const auto columnSet = table.getCurrentColumnSet();
    recordColumnSet(*columnSet, tp);

    // Record column set columns
    for (const auto& columnSetColumn : columnSet->getColumns())
        recordColumnSetColumn(*columnSetColumn, tp);

    // Record constraint definitions, constraints and column constraint definitions
    for (const auto& column : columns) {
        const auto columnDefinition = column->getCurrentColumnDefinition();
        if (!columnDefinition->hasConstraints()) continue;
        const auto& constraintsIndex = columnDefinition->getConstraints().byConstraintId();
        for (const auto& columnDefinitionConstraint : constraintsIndex) {
            const auto& constraint = columnDefinitionConstraint->getConstraint();
            const auto& constraintDefinition = constraint.getDefinition();
            if (!constraintDefinition.isWrittenToStorage()) {
                recordConstraintDefinition(constraintDefinition, tp);
                stdext::as_mutable(constraintDefinition).setWrittenToStorage();
            }
            recordConstraint(constraint, tp);
            recordColumnDefinitionConstraint(*columnDefinitionConstraint, tp);
        }
    }

    // Record MC index
    recordIndexAndColumns(*table.getMasterColumnMainIndex(), tp);
}

}  // namespace siodb::iomgr::dbengine
