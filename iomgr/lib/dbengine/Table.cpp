// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Table.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDefinitionConstraintList.h"
#include "ColumnSetColumn.h"
#include "Index.h"
#include "TableColumns.h"
#include "ThrowDatabaseError.h"
#include "parser/EmptyExpressionEvaluationContext.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

namespace siodb::iomgr::dbengine {

Table::Table(Database& database, TableType type, std::string&& name, std::uint64_t firstUserTrid,
        std::optional<std::string>&& description)
    : m_database(database)
    , m_name(validateTableName(std::move(name)))
    , m_description(std::move(description))
    , m_isSystemTable(Database::isSystemTable(m_name))
    , m_id(database.generateNextTableId(m_isSystemTable))
    , m_type(type)
    , m_dataDir(ensureDataDir(
              utils::constructPath(database.getDataDir(), kTableDataDirPrefix, m_id), true))
    , m_columnSetCache(kColumnSetCacheCapacity)
    , m_currentColumnSet(createColumnSetUnlocked())
    , m_constraintCache(*this, kConstraintCacheCapacity)
    , m_firstUserTrid(firstUserTrid)
{
    createMasterColumn(firstUserTrid);
    createInitializationFlagFile();
}

Table::Table(Database& database, const TableRecord& tableRecord)
    : m_database(database)
    , m_name(validateTableName(std::string(tableRecord.m_name)))
    , m_description(tableRecord.m_description)
    , m_isSystemTable(Database::isSystemTable(m_name))
    , m_id(tableRecord.m_id)
    , m_type(tableRecord.m_type)
    , m_dataDir(ensureDataDir(
              utils::constructPath(database.getDataDir(), kTableDataDirPrefix, m_id), false))
    , m_columnSetCache(kColumnSetCacheCapacity)
    , m_currentColumnSet(findColumnSetChecked(tableRecord.m_currentColumnSetId))
    , m_constraintCache(*this, kConstraintCacheCapacity)
    , m_firstUserTrid(tableRecord.m_firstUserTrid)
{
    // Populate columns from the current column set
    loadColumnsUnlocked();
    m_masterColumn->loadMasterColumnMainIndex();
}

std::string Table::makeDisplayName() const
{
    std::ostringstream oss;
    oss << '\'' << m_database.getName() << "'.'" << m_name << '\'';
    return oss.str();
}

std::string Table::makeDisplayCode() const
{
    std::ostringstream oss;
    oss << m_database.getUuid() << '.' << m_id;
    return oss.str();
}

std::uint32_t Table::getColumnCurrentPosition(std::uint64_t columnId) const
{
    std::lock_guard lock(m_mutex);
    return m_currentColumnSet->findColumnPosition(columnId);
}

std::vector<ColumnPtr> Table::getColumnsOrderedByPosition() const
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_currentColumns.byPosition();
    std::vector<ColumnPtr> columns;
    columns.reserve(index.size());
    // Columns are already sorted in the index
    for (const auto& column : index)
        columns.push_back(column.m_column);
    return columns;
}

std::uint64_t Table::getCurrentColumnSetId() const
{
    std::lock_guard lock(m_mutex);
    return m_currentColumnSet->getId();
}

ColumnSetPtr Table::findColumnSetChecked(std::uint64_t columnSetId)
{
    std::lock_guard lock(m_mutex);
    const auto cachedColumnSet = m_columnSetCache.get(columnSetId);
    if (cachedColumnSet) return *cachedColumnSet;
    const auto record = m_database.findColumnSetRecord(columnSetId);
    return createColumnSetUnlocked(record);
}

void Table::closeCurrentColumnSet()
{
    std::lock_guard lock(m_mutex);
    m_currentColumnSet->markClosedForModification();
    m_database.updateColumnSetRegistration(*m_currentColumnSet);
}

ColumnPtr Table::createColumn(ColumnSpecification&& columnSpec, std::uint64_t firstUserTrid)
{
    std::lock_guard lock(m_mutex);

    // Check column presence
    if (isColumnExistsUnlocked(columnSpec.m_name)) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnAlreadyExists, m_database.getName(),
                m_name, columnSpec.m_name);
    }

    // Create column
    const auto column = std::make_shared<Column>(*this, std::move(columnSpec), firstUserTrid);

    // Create column set column record
    const auto columnSetColumnId =
            m_currentColumnSet->addColumn(*column->getCurrentColumnDefinition());

    // Register column
    m_currentColumns.emplace(TableColumn(column, columnSetColumnId, m_currentColumns.size()));
    m_database.registerColumn(*column);
    return column;
}

IndexPtr Table::getMasterColumnMainIndex() const
{
    std::lock_guard lock(m_mutex);
    return m_masterColumn->getMasterColumnMainIndex();
}

void Table::checkColumnBelongsToTable(const Column& column, const char* operationName) const
{
    if (&column.getTable() != this) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnDoesNotBelongToTable, operationName,
                column.getDatabaseName(), column.getTableName(), column.getName(),
                column.getDatabaseUuid(), column.getTableId(), column.getId(),
                column.getDatabaseName(), m_name, column.getDatabaseUuid(), m_id);
    }
}

ConstraintPtr Table::createConstraint(std::string&& name,
        const ConstConstraintDefinitionPtr& constraintDefinition, Column* column,
        std::optional<std::string>&& description)
{
    std::lock_guard lock(m_mutex);
    auto constraint = m_database.createConstraint(
            *this, column, std::move(name), constraintDefinition, std::move(description));
    m_constraintCache.emplace(constraint->getId(), constraint);
    return constraint;
}

ConstraintPtr Table::findConstraintChecked(Column* column, std::uint64_t constraintId)
{
    std::lock_guard lock(m_mutex);
    const auto cachedConstraint = m_constraintCache.get(constraintId);
    if (cachedConstraint) return *cachedConstraint;
    return createConstraintUnlocked(column, m_database.findConstraintRecord(constraintId));
}

std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> Table::insertRow(
        const std::vector<std::string>& columnNames, std::vector<Variant>&& columnValues,
        const TransactionParameters& transactionParameters, std::uint64_t customTrid)
{
    std::lock_guard lock(m_mutex);
    const auto columnCount = m_currentColumns.size();

    // Check that number of columns matches number of values
    if (columnNames.size() != columnValues.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorNumberOfValuesMistatchOnInsert,
                m_database.getName(), m_name, columnValues.size(), columnNames.size());
    }

    // Check that number of column doesn't exceed number of columns in table except MC
    if (columnValues.size() >= columnCount) {
        throwDatabaseError(IOManagerMessageId::kErrorTooManyColumnsToInsert, m_database.getName(),
                m_name, columnValues.size(), columnCount - 1);
    }

    const auto& columnsByPosition = m_currentColumns.byPosition();
    std::vector<Variant> orderedColumnValues(columnCount - 1);

    // vector<bool> was always suboptimal, so use vector<char>
    std::vector<char> columnPresent(columnCount);
    std::vector<CompoundDatabaseError::ErrorRecord> errors;
    const auto& columnsByName = m_currentColumns.byName();

    // Check columns
    for (std::size_t i = 0, n = columnNames.size(); i < n; ++i) {
        const auto& columnName = columnNames[i];
        if (!isValidDatabaseObjectName(columnName)) {
            errors.push_back(std::move(
                    makeDatabaseError(IOManagerMessageId::kErrorInvalidColumnName, columnName)));
            continue;
        }

        const auto it = columnsByName.find(columnName);
        if (it == columnsByName.end()) {
            const auto column = columnsByPosition.find(0)->m_column;
            errors.push_back(makeDatabaseError(IOManagerMessageId::kErrorColumnDoesNotExist,
                    column->getTable().getDatabaseName(), column->getTableName(), columnName));
            continue;
        }

        if (it->m_column->isMasterColumn()) {
            errors.push_back(
                    makeDatabaseError(IOManagerMessageId::kErrorCannotInsertIntoMasterColumn));
            continue;
        }

        auto& columnPresentFlag = columnPresent.at(it->m_column->getCurrentPosition());
        if (columnPresentFlag) {
            errors.push_back(makeDatabaseError(
                    IOManagerMessageId::kErrorInsertDuplicateColumnName, columnName));
            continue;
        }

        columnPresentFlag = 1;
        orderedColumnValues[it->m_position - 1] = std::move(columnValues[i]);
    }

    if (!errors.empty()) throw CompoundDatabaseError(std::move(errors));

    // Start from column [1], skip TRID
    for (std::size_t i = 1; i < columnCount; ++i) {
        if (columnPresent[i]) continue;
        const auto column = columnsByPosition.find(i)->m_column;
        // NOTE: For now, always use current column definition.
        const auto columnDefinition = column->getCurrentColumnDefinition();
        orderedColumnValues.at(i - 1) = columnDefinition->getDefaultValue();
    }

    return doInsertRowUnlocked(std::move(orderedColumnValues), transactionParameters, customTrid);
}

std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> Table::insertRow(
        std::vector<Variant>&& columnValues, const TransactionParameters& transactionParameters,
        std::uint64_t customTrid)
{
    std::lock_guard lock(m_mutex);
    const auto columnCount = m_currentColumns.size();

    // Check that number of column doesn't exceed number of columns in table except MC
    if (columnValues.size() >= columnCount) {
        throwDatabaseError(IOManagerMessageId::kErrorTooManyColumnsToInsert, m_database.getName(),
                m_name, columnValues.size(), columnCount - 1);
    }

    // Add values for missing columns
    const auto currentValueCount = columnValues.size();
    const auto requiredValueCount = columnCount - 1;
    if (currentValueCount < requiredValueCount) {
        columnValues.resize(requiredValueCount);
        // Place a copy of a default value, if defined, into the added elements of columnValues.
        const auto& columns = m_currentColumnSet->getColumns();
        for (std::size_t i = currentValueCount; i < requiredValueCount; ++i) {
            const auto& columnSetColumn = columns.at(i + 1);
            const auto column = findColumnChecked(columnSetColumn->getColumnId());
            //const auto columnDefinition =
            //        column->findColumnDefinitionChecked(columnSetColumn->getColumnDefinitionId());
            // NOTE: For now, always use current column definition.
            const auto columnDefinition = column->getCurrentColumnDefinition();
            columnValues.at(i) = columnDefinition->getDefaultValue();
        }
    }

    return doInsertRowUnlocked(std::move(columnValues), transactionParameters, customTrid);
}

std::pair<bool, MasterColumnRecordPtr> Table::deleteRow(
        std::uint64_t trid, const TransactionParameters& transactionParameters)
{
    std::lock_guard lock(m_mutex);

    const auto lastTrid = m_masterColumn->getLastUserTrid();
    if (trid > lastTrid) return std::make_pair(false, MasterColumnRecordPtr());

    // Find row
    std::uint8_t key[8];
    IndexValue indexValue;
    ::pbeEncodeUInt64(trid, key);
    if (!m_masterColumn->getMasterColumnMainIndex()->find(key, indexValue.m_data, 1))
        return std::make_pair(false, MasterColumnRecordPtr());

    // Read master column record
    ColumnDataAddress mcrAddr;
    mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));
    MasterColumnRecord mcr;
    m_masterColumn->readMasterColumnRecord(mcrAddr, mcr);

    // Delete row
    return std::make_pair(true, deleteRow(mcr, mcrAddr, transactionParameters));
}

MasterColumnRecordPtr Table::deleteRow(const MasterColumnRecord& mcr,
        const ColumnDataAddress& mcrAddress, const TransactionParameters& transactionParameters)
{
    std::lock_guard lock(m_mutex);
    auto newMcr = std::make_unique<MasterColumnRecord>(*this, mcr.getTableRowId(),
            transactionParameters.m_transactionId, mcr.getCreateTimestamp(),
            transactionParameters.m_timestamp, mcr.getVersion() + 1,
            m_database.generateNextAtomicOperationId(), DmlOperationType::kDelete,
            transactionParameters.m_userId, m_currentColumnSet->getId(), mcrAddress);
    m_masterColumn->writeMasterColumnRecord(*newMcr);
    return newMcr;
}

std::tuple<bool, MasterColumnRecordPtr, std::vector<std::uint64_t>> Table::updateRow(
        std::uint64_t trid, const std::vector<std::string>& columnNames,
        std::vector<Variant>&& columnValues, bool allowTrid, const TransactionParameters& tp)
{
    std::lock_guard lock(m_mutex);

    std::vector<std::size_t> columnPositions;
    columnPositions.reserve(columnNames.size());

    for (const auto& columnName : columnNames) {
        const auto column = findColumnCheckedUnlocked(columnName);
        if (!allowTrid && column->isMasterColumn()) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorCannotUpdateMasterColumn, getDatabaseName(), m_name);
        }
        columnPositions.push_back(column->getCurrentPosition());
    }

    return updateRow(trid, columnPositions, std::move(columnValues), tp);
}

std::tuple<bool, MasterColumnRecordPtr, std::vector<std::uint64_t>> Table::updateRow(
        std::uint64_t trid, const std::vector<std::size_t>& columnPositions,
        std::vector<Variant>&& columnValues, const TransactionParameters& tp)
{
    std::lock_guard lock(m_mutex);

    // Find row
    std::uint8_t key[8];
    IndexValue indexValue;
    ::pbeEncodeUInt64(trid, key);
    if (!m_masterColumn->getMasterColumnMainIndex()->find(key, indexValue.m_data, 1))
        return std::make_tuple(false, MasterColumnRecordPtr(), std::vector<std::uint64_t>());

    // Read master column record
    ColumnDataAddress mcrAddr;
    mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));
    MasterColumnRecord mcr;
    m_masterColumn->readMasterColumnRecord(mcrAddr, mcr);

    // Perform update
    auto result = updateRow(mcr, mcrAddr, columnPositions, std::move(columnValues), tp);
    return std::make_tuple(true, std::move(result.first), std::move(result.second));
}

std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> Table::updateRow(
        const MasterColumnRecord& mcr, const ColumnDataAddress& mcrAddress,
        const std::vector<std::size_t>& columnPositions, std::vector<Variant>&& columnValues,
        const TransactionParameters& tp)
{
    std::lock_guard lock(m_mutex);

    if (columnValues.size() != columnPositions.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorUpdateValuesDoesNotFitToPositions,
                m_database.getName(), m_name, columnValues.size(), columnPositions.size());
    }

    auto columnRecords = mcr.getColumnRecords();
    if (columnValues.size() > columnRecords.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorUpdateValuesCountGreaterThanAddresses,
                m_database.getName(), m_name, columnValues.size(), columnRecords.size());
    }

    auto newMcr = std::make_unique<MasterColumnRecord>(*this, mcr.getTableRowId(),
            tp.m_transactionId, mcr.getCreateTimestamp(), tp.m_timestamp, mcr.getVersion() + 1,
            m_database.generateNextAtomicOperationId(), DmlOperationType::kUpdate, tp.m_userId,
            m_currentColumnSet->getId(), mcrAddress);

    const auto tableColumns = getColumnsOrderedByPosition();
    std::vector<std::uint64_t> nextBlockIds;
    nextBlockIds.reserve(newMcr->getColumnCount());
    try {
        std::size_t valueIndex = 0;
        for (const auto pos : columnPositions) {
            const auto& tableColumn = tableColumns.at(pos);
            if (tableColumn->isMasterColumn()) continue;
            auto res = tableColumn->writeRecord(std::move(columnValues[valueIndex]));
            // Normal column positions start from 1, column at position 0 is master column.
            auto& record = columnRecords[pos - 1];
            record.setAddress(res.first);
            record.setUpdateTimestamp(tp.m_timestamp);
            nextBlockIds.push_back(res.second.getBlockId());
            ++valueIndex;
        }
        newMcr->setColumnRecords(std::move(columnRecords));
        m_masterColumn->writeMasterColumnRecord(*newMcr);
    } catch (...) {
        // Rollback updated columns
        auto blockIt = nextBlockIds.cbegin();
        for (const auto columnPosition : columnPositions) {
            const auto& tableColumn = tableColumns[columnPosition];
            if (tableColumn->isMasterColumn()) continue;
            if (!columnRecords[columnPosition - 1].isNullValueAddress()) {
                try {
                    tableColumns[columnPosition]->rollbackToAddress(
                            columnRecords[columnPosition - 1].getAddress(), *blockIt);
                } catch (std::exception& ex) {
                    LOG_ERROR << ex.what();
                }
            }
            ++blockIt;
            if (blockIt == nextBlockIds.cend()) break;
        }
        throw;
    }

    return std::make_pair(std::move(newMcr), std::move(nextBlockIds));
}

void Table::rollbackLastRow(
        const MasterColumnRecord& mcr, const std::vector<std::uint64_t>& nextBlockIds)
{
    const auto& columnRecords = mcr.getColumnRecords();
    if (columnRecords.size() != nextBlockIds.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorNumberOfNextBlocksMistatchOnRollback,
                m_database.getName(), m_name, nextBlockIds.size(), columnRecords.size());
    }

    std::lock_guard lock(m_mutex);
    const auto columnCount = m_currentColumns.size();

    if (columnRecords.size() >= columnCount) {
        throwDatabaseError(IOManagerMessageId::kErrorTooManyColumnsToRollback, m_database.getName(),
                m_name, columnRecords.size(), columnCount - 1);
    }

    auto blockIt = nextBlockIds.cbegin();
    auto columnIt = m_currentColumns.byPosition().cbegin();
    for (const auto& r : columnRecords) {
        if (columnIt->m_column->isMasterColumn()) ++columnIt;
        if (!r.isNullValueAddress()) {
            try {
                columnIt->m_column->rollbackToAddress(r.getAddress(), *blockIt);
            } catch (std::exception& ex) {
                LOG_ERROR << ex.what();
            }
        }
        ++blockIt;
        ++columnIt;
    }
}

void Table::flushIndices()
{
    std::lock_guard lock(m_mutex);
    m_masterColumn->getMasterColumnMainIndex()->flush();
}

std::uint64_t Table::generateNextUserTrid()
{
    // NOTE: This function can't be moved to header or inlined due to compilation dependencies.
    return m_masterColumn->generateNextUserTrid();
}

std::uint64_t Table::generateNextSystemTrid()
{
    // NOTE: This function can't be moved to header or inlined due to compilation dependencies.
    return m_masterColumn->generateNextSystemTrid();
}

void Table::setLastSystemTrid(std::uint64_t lastSystemTrid)
{
    // NOTE: This function can't be moved to header or inlined due to compilation dependencies.
    m_masterColumn->setLastSystemTrid(lastSystemTrid);
}

void Table::setLastUserTrid(std::uint64_t lastUserTrid)
{
    // NOTE: This function can't be moved to header or inlined due to compilation dependencies.
    m_masterColumn->setLastUserTrid(lastUserTrid);
}

ColumnDefinitionPtr Table::findColumnDefinitionChecked(std::uint64_t columnDefinitionId)
{
    const auto columnDefinitionRecord = m_database.findColumnDefinitionRecord(columnDefinitionId);
    std::lock_guard lock(m_mutex);
    const auto& index = m_currentColumns.byColumnId();
    const auto it = index.find(columnDefinitionRecord.m_columnId);
    if (it == index.cend()) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableColumnDefinition,
                columnDefinitionId, columnDefinitionRecord.m_columnId, m_database.getName(), m_name,
                m_database.getUuid(), m_id);
    }
    return it->m_column->findColumnDefinitionChecked(columnDefinitionId);
}

// --- internals ---

std::string&& Table::validateTableName(std::string&& tableName)
{
    if (isValidDatabaseObjectName(tableName)) return std::move(tableName);
    throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, tableName);
}

void Table::createMasterColumn(std::uint64_t firstUserTrid)
{
    m_masterColumn =
            createColumn(ColumnSpecification(kMasterColumnName, Column::kMasterColumnDataType,
                                 isSystemTable() ? kSystemTableDataFileDataAreaSize
                                                 : kDefaultDataFileDataAreaSize),
                    firstUserTrid);
}

void Table::loadColumnsUnlocked()
{
    const auto& columns = m_currentColumnSet->getColumns();
    if (columns.empty()) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnSetMissingColumns, m_database.getName(),
                m_name, m_currentColumnSet->getId(), m_database.getUuid(), m_id);
    }

    TableColumns currentColumns;
    std::uint32_t position = 0;
    for (const auto& columnSetColumn : columns) {
        const auto columnDefinitionRecord =
                m_database.findColumnDefinitionRecord(columnSetColumn->getColumnDefinitionId());
        const auto columnRecord = m_database.findColumnRecord(columnDefinitionRecord.m_columnId);
        auto column = std::make_shared<Column>(*this, columnRecord, m_firstUserTrid);
        currentColumns.insert(TableColumn(column, columnSetColumn->getId(), position++));
    }
    m_currentColumns.swap(currentColumns);

    // Finally, update master column
    m_masterColumn = findColumnCheckedUnlocked(kMasterColumnName);
}

ColumnSetPtr Table::createColumnSetUnlocked()
{
    auto columnSet = std::make_shared<ColumnSet>(*this);
    m_columnSetCache.emplace(columnSet->getId(), columnSet);
    m_database.registerColumnSet(*columnSet);
    return columnSet;
}

ColumnSetPtr Table::createColumnSetUnlocked(const ColumnSetRecord& columnSetRecord)
{
    auto columnSet = std::make_shared<ColumnSet>(*this, columnSetRecord);
    m_columnSetCache.emplace(columnSet->getId(), columnSet);
    return columnSet;
}

ConstraintPtr Table::createConstraintUnlocked(Column* column, std::string&& name,
        const ConstConstraintDefinitionPtr& constraintDefinition,
        std::optional<std::string>&& description)
{
    auto constraint = m_database.createConstraint(
            *this, column, std::move(name), constraintDefinition, std::move(description));
    m_constraintCache.emplace(constraint->getId(), constraint);
    m_database.registerConstraint(*constraint);
    return constraint;
}

ConstraintPtr Table::createConstraintUnlocked(
        Column* column, const ConstraintRecord& constraintRecord)
{
    auto constraint = m_database.createConstraint(*this, column, constraintRecord);
    m_constraintCache.emplace(constraint->getId(), constraint);
    m_database.registerConstraint(*constraint);
    return constraint;
}

TableColumn Table::findColumnByIdUnlocked(std::uint64_t columnId) const
{
    const auto& index = m_currentColumns.byColumnId();
    const auto it = index.find(columnId);
    if (it != index.end()) return *it;
    throwDatabaseError(
            IOManagerMessageId::kErrorColumnDoesNotExist, m_database.getName(), m_name, columnId);
}

TableColumn Table::findColumnByPositionUnlocked(std::uint32_t position) const
{
    const auto& index = m_currentColumns.byPosition();
    const auto it = index.find(position);
    if (it != index.end()) return *it;
    throwDatabaseError(IOManagerMessageId::kErrorTableColumnIndexOutOfRange, m_database.getName(),
            m_name, position + 1);
}

ColumnPtr Table::findColumnCheckedUnlocked(uint64_t columnId) const
{
    auto column = findColumnUnlocked(columnId);
    if (column) return column;
    throwDatabaseError(
            IOManagerMessageId::kErrorColumnDoesNotExist2, m_database.getName(), m_name, columnId);
}

ColumnPtr Table::findColumnCheckedUnlocked(const std::string& columnName) const
{
    auto column = findColumnUnlocked(columnName);
    if (column) return column;
    throwDatabaseError(
            IOManagerMessageId::kErrorColumnDoesNotExist, m_database.getName(), m_name, columnName);
}

ColumnPtr Table::findColumnUnlocked(uint64_t columnId) const noexcept
{
    const auto& index = m_currentColumns.byColumnId();
    const auto it = index.find(columnId);
    return it == index.cend() ? nullptr : it->m_column;
}

ColumnPtr Table::findColumnUnlocked(const std::string& columnName) const noexcept
{
    const auto& index = m_currentColumns.byName();
    const auto it = index.find(columnName);
    return it == index.cend() ? nullptr : it->m_column;
}

std::optional<std::uint32_t> Table::getColumnPositionUnlocked(uint64_t columnId) const noexcept
{
    std::optional<std::uint32_t> result;
    const auto& index = m_currentColumns.byColumnId();
    const auto it = index.find(columnId);
    if (it != index.cend()) result = it->m_position;
    return result;
}

std::optional<std::uint32_t> Table::getColumnPositionUnlocked(
        const std::string& columnName) const noexcept
{
    std::optional<std::uint32_t> result;
    const auto& index = m_currentColumns.byName();
    const auto it = index.find(columnName);
    if (it != index.cend()) result = it->m_position;
    return result;
}

std::string&& Table::ensureDataDir(std::string&& dataDir, bool create) const
{
    const auto initFlagFile = utils::constructPath(dataDir, kInitializationFlagFile);
    const auto initFlagFileExists = fs::exists(initFlagFile);
    if (create) {
        if (initFlagFileExists) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorTableAlreadyExists, m_database.getName(), m_name);
        }

        // Create data directory
        try {
            fs::path dataDirPath(dataDir);
            if (fs::exists(dataDirPath)) fs::remove_all(dataDirPath);
            fs::create_directories(dataDirPath);
        } catch (fs::filesystem_error& ex) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateTableDataDir, dataDir,
                    m_database.getName(), m_name, m_database.getUuid(), m_id, ex.code().value(),
                    ex.code().message());
        }
    } else {
        if (!boost::filesystem::exists(dataDir))
            throwDatabaseError(IOManagerMessageId::kErrorTableDataFolderDoesNotExist,
                    m_database.getName(), m_name, dataDir);

        if (!initFlagFileExists)
            throwDatabaseError(IOManagerMessageId::kErrorTableInitFileDoesNotExist,
                    m_database.getName(), m_name, initFlagFileExists);
    }
    return std::move(dataDir);
}

void Table::createInitializationFlagFile() const
{
    const auto initFlagFile = utils::constructPath(m_dataDir, kInitializationFlagFile);
    std::ofstream ofs(initFlagFile);
    if (!ofs.is_open()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateTableInitializationFlagFile,
                initFlagFile, m_database.getName(), m_name, m_database.getUuid(), m_id,
                "create file failed");
    }
    ofs.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        ofs << std::time(nullptr) << std::flush;
    } catch (std::exception& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateTableInitializationFlagFile,
                initFlagFile, m_database.getName(), m_name, m_database.getUuid(), m_id,
                "write failed");
    }
}

std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> Table::doInsertRowUnlocked(
        std::vector<Variant>&& columnValues, const TransactionParameters& tp,
        std::uint64_t customTrid)
{
    // Write columns
    auto mcr = std::make_unique<MasterColumnRecord>(*this, customTrid, tp.m_transactionId,
            tp.m_timestamp, tp.m_timestamp, 0U, m_database.generateNextAtomicOperationId(),
            DmlOperationType::kInsert, tp.m_userId, m_currentColumnSet->getId(), kNullValueAddress);

    std::vector<std::uint64_t> nextBlockIds;
    nextBlockIds.reserve(mcr->getColumnCount());

    try {
        std::size_t i = 0;
        for (const auto& tableColumnRecord : m_currentColumns.byPosition()) {
            if (tableColumnRecord.m_column->isMasterColumn()) continue;
            auto res = tableColumnRecord.m_column->writeRecord(std::move(columnValues[i]));
            mcr->addColumnRecord(res.first, tp.m_timestamp, tp.m_timestamp);
            nextBlockIds.push_back(res.second.getBlockId());
            ++i;
        }
        m_masterColumn->writeMasterColumnRecord(*mcr);
    } catch (...) {
        rollbackLastRow(*mcr, nextBlockIds);
        throw;
    }

    return std::make_pair(std::move(mcr), std::move(nextBlockIds));
}

}  // namespace siodb::iomgr::dbengine
