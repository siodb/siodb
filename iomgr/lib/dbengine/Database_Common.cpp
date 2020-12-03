// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Database.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDataBlock.h"
#include "ColumnDefinition.h"
#include "ColumnDefinitionConstraint.h"
#include "ColumnSet.h"
#include "DefaultValueConstraint.h"
#include "Index.h"
#include "NotNullConstraint.h"
#include "SystemDatabase.h"
#include "Table.h"
#include "ThrowDatabaseError.h"
#include "reg/CipherKeyRecord.h"

// Common project headers
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_ext/algorithm_ext.h>
#include <siodb/common/stl_ext/string_ext.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>
#include <siodb/iomgr/shared/dbengine/io/EncryptedFile.h>
#include <siodb/iomgr/shared/dbengine/io/NormalFile.h>

// STL headers
#include <iomanip>
#include <numeric>

// Boost headers
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

// OpenSSL
#include <openssl/md5.h>

namespace siodb::iomgr::dbengine {

bool Database::isSystemDatabase() const noexcept
{
    return false;
}

std::string Database::makeDisplayName() const
{
    std::ostringstream oss;
    oss << '\'' << m_name << '\'';
    return oss.str();
}

std::vector<std::string> Database::getTableNames(bool includeSystemTables) const
{
    std::lock_guard lock(m_mutex);
    std::vector<std::string> result;
    result.reserve(m_tableRegistry.size());
    const auto& index = m_tableRegistry.byName();
    stdext::transform_if(
            index.cbegin(), index.cend(), std::back_inserter(result),
            [](const auto& tableRecord) { return tableRecord.m_name; },
            [includeSystemTables](const auto& tableRecord) {
                return isSystemTable(tableRecord.m_name) == includeSystemTables;
            });
    return result;
}

std::vector<TableRecord> Database::getTableRecordsOrderedByName(bool includeSystemTables) const
{
    std::lock_guard lock(m_mutex);
    std::vector<TableRecord> result;
    const auto& index = m_tableRegistry.byName();
    const auto tableCount = includeSystemTables
                                    ? m_tableRegistry.size()
                                    : std::count_if(index.begin(), index.end(),
                                            [](const auto& tableRecord) noexcept {
                                                return !isSystemTable(tableRecord.m_name);
                                            });
    if (tableCount > 0) {
        result.reserve(tableCount);
        if (includeSystemTables)
            std::copy(index.begin(), index.end(), std::back_inserter(result));
        else {
            std::copy_if(index.begin(), index.end(), std::back_inserter(result),
                    [](const auto& tableRecord) noexcept {
                        return !isSystemTable(tableRecord.m_name);
                    });
        }
        std::sort(result.begin(), result.end(), [](const auto& left, const auto& right) noexcept {
            return left.m_name < right.m_name;
        });
    }
    return result;
}

TablePtr Database::findTableChecked(const std::string& tableName)
{
    std::lock_guard lock(m_mutex);
    if (auto table = findTableUnlocked(tableName)) return table;
    throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotExist, m_name, tableName);
}

TablePtr Database::findTableChecked(std::uint32_t tableId)
{
    std::lock_guard lock(m_mutex);
    if (auto table = findTableUnlocked(tableId)) return table;
    throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotExist, m_name, tableId);
}

ConstraintDefinitionPtr Database::createConstraintDefinition(bool system,
        ConstraintType constraintType, requests::ConstExpressionPtr&& expression, bool& existing)
{
    std::lock_guard lock(m_mutex);
    return createConstraintDefinitionUnlocked(
            system, constraintType, std::move(expression), existing);
}

ConstraintDefinitionPtr Database::findOrCreateConstraintDefinition(
        bool system, ConstraintType type, const BinaryValue& serializedExpression)
{
    std::lock_guard lock(m_mutex);

    // Try to find suitable constraint definition
    const auto hash = ConstraintDefinitionRecord::computeHash(type, serializedExpression);
    auto r = m_constraintDefinitionRegistry.byHash().equal_range(hash);
    for (; r.first != r.second; ++r.first) {
        if (r.first->m_type == type && r.first->m_expression == serializedExpression
                && ((system && r.first->m_id < kFirstUserTableConstraintDefinitionId)
                        || (!system && r.first->m_id >= kFirstUserTableConstraintDefinitionId))) {
            return findConstraintDefinitionChecked(r.first->m_id);
        }
    }

    // No such constraint definition, create new one
    requests::ExpressionPtr expression;
    requests::Expression::deserialize(
            serializedExpression.data(), serializedExpression.size(), expression);
    auto constraintDefinition =
            std::make_shared<ConstraintDefinition>(system, *this, type, std::move(expression));
    m_constraintDefinitions.emplace(constraintDefinition->getId(), constraintDefinition);
    m_constraintDefinitionRegistry.emplace(*constraintDefinition);
    return constraintDefinition;
}

ConstraintDefinitionPtr Database::findConstraintDefinitionChecked(
        std::uint64_t constraintDefinitionId)
{
    std::lock_guard lock(m_mutex);
    if (auto constraintDefinition = findConstraintDefinitionUnlocked(constraintDefinitionId))
        return constraintDefinition;
    throwDatabaseError(IOManagerMessageId::kErrorConstraintDefinitionDoesNotExist, m_name,
            constraintDefinitionId);
}

ConstraintPtr Database::createConstraint(Table& table, Column* column, std::string&& name,
        const ConstConstraintDefinitionPtr& constraintDefinition,
        std::optional<std::string>&& description)
{
    // Validate table and column
    checkTableBelongsToThisDatabase(table, __func__);
    if (column) table.checkColumnBelongsToTable(*column, __func__);

    std::lock_guard lock(m_mutex);
    const auto& index = m_constraintRegistry.byName();
    const auto it = index.find(name);
    if (it != index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorConstraintAlreadyExists, m_name, name);

    ConstraintPtr constraint;
    switch (constraintDefinition->getType()) {
        case ConstraintType::kNotNull: {
            constraint = std::make_shared<NotNullConstraint>(
                    *column, std::move(name), constraintDefinition, std::move(description));
            break;
        }
        case ConstraintType::kDefaultValue: {
            constraint = std::make_shared<DefaultValueConstraint>(
                    *column, std::move(name), constraintDefinition, std::move(description));
            break;
        }
        default: {
            throwDatabaseError(IOManagerMessageId::kErrorConstraintNotSupported, m_name,
                    constraintDefinition->getId(), m_uuid,
                    static_cast<int>(constraintDefinition->getType()));
        }
    }

    m_constraintRegistry.emplace(*constraint);
    return constraint;
}

ConstraintPtr Database::createConstraint(
        Table& table, Column* column, const ConstraintRecord& constraintRecord)
{
    // Validate table and column
    checkTableBelongsToThisDatabase(table, __func__);
    if (column) table.checkColumnBelongsToTable(*column, __func__);

    std::lock_guard lock(m_mutex);

    const auto constraintDefinition =
            findConstraintDefinitionChecked(constraintRecord.m_constraintDefinitionId);

    switch (constraintDefinition->getType()) {
        case ConstraintType::kNotNull: {
            return std::make_shared<NotNullConstraint>(*column, constraintRecord);
        }
        case ConstraintType::kDefaultValue: {
            return std::make_shared<DefaultValueConstraint>(*column, constraintRecord);
        }
        default: {
            throwDatabaseError(IOManagerMessageId::kErrorConstraintNotSupported, m_name,
                    constraintDefinition->getId(), m_uuid,
                    static_cast<int>(constraintDefinition->getType()));
        }
    }
}

bool Database::isConstraintExists(const std::string& constraintName) const
{
    std::lock_guard lock(m_mutex);
    return m_constraintRegistry.byName().count(constraintName) > 0;
}

ColumnSetRecord Database::findColumnSetRecord(std::uint64_t columnSetId) const
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_columnSetRegistry.byId();
    const auto it = index.find(columnSetId);
    if (it == index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorColumnSetDoesNotExist, m_name, columnSetId);
    return *it;
}

ColumnRecord Database::findColumnRecord(std::uint64_t columnId) const
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_columnRegistry.byId();
    const auto it = index.find(columnId);
    if (it == index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorColumnDoesNotExist3, m_name, columnId);
    return *it;
}

ColumnDefinitionRecord Database::findColumnDefinitionRecord(std::uint64_t columnDefinitionId) const
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_columnDefinitionRegistry.byId();
    const auto it = index.find(columnDefinitionId);
    if (it == index.cend()) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnDefinitionDoesNotExist2, m_name,
                columnDefinitionId);
    }
    return *it;
}

std::uint64_t Database::findLatestColumnDefinitionIdForColumn(
        std::uint32_t tableId, std::uint64_t columnId)
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_columnDefinitionRegistry.byColumnIdAndId();
    if (!m_columnDefinitionRegistry.empty()) {
        auto it = index.lower_bound(std::make_pair(columnId + 1, std::uint64_t(0)));
        if (it != index.cbegin() && (--it)->m_columnId == columnId) return it->m_id;
    }
    throwDatabaseError(
            IOManagerMessageId::kErrorMissingColumnDefinitionsForColumn, m_uuid, tableId, columnId);
}

ConstraintRecord Database::findConstraintRecord(std::uint64_t constraintId) const
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_constraintRegistry.byId();
    const auto it = index.find(constraintId);
    if (it == index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorConstraintDoesNotExist2, m_name, constraintId);
    return *it;
}

IndexRecord Database::findIndexRecord(std::uint64_t indexId) const
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_indexRegistry.byId();
    const auto it = index.find(indexId);
    if (it == index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorIndexDoesNotExist2, m_name, indexId);
    return *it;
}

void Database::release()
{
    std::size_t useCount, desiredUseCount;
    do {
        useCount = m_useCount.load();
        if (useCount == 0) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorCannotReleaseUnusedDatabase, m_name, m_uuid);
        }
        desiredUseCount = useCount - 1;
    } while (!m_useCount.compare_exchange_strong(useCount, desiredUseCount));
}

std::uint32_t Database::generateNextTableId(bool system)
{
    const auto tableId = system ? (m_sysTablesTable ? m_sysTablesTable->generateNextSystemTrid()
                                                    : ++m_tmpTridCounters.m_lastTableId)
                                : m_sysTablesTable->generateNextUserTrid();
    if (tableId >= std::numeric_limits<std::uint32_t>::max())
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseResourceExhausted, m_name, "Table ID");
    return static_cast<std::uint32_t>(tableId);
}

std::uint64_t Database::generateNextColumnId(bool system)
{
    return system ? (m_sysColumnsTable ? m_sysColumnsTable->generateNextSystemTrid()
                                       : ++m_tmpTridCounters.m_lastColumnId)
                  : m_sysColumnsTable->generateNextUserTrid();
}

std::uint64_t Database::generateNextColumnDefinitionId(bool system)
{
    return system ? (m_sysColumnDefsTable ? m_sysColumnDefsTable->generateNextSystemTrid()
                                          : ++m_tmpTridCounters.m_lastColumnDefinitionId)
                  : m_sysColumnDefsTable->generateNextUserTrid();
}

std::uint64_t Database::generateNextColumnSetId(bool system)
{
    return system ? (m_sysColumnSetsTable ? m_sysColumnSetsTable->generateNextSystemTrid()
                                          : ++m_tmpTridCounters.m_lastColumnSetId)
                  : m_sysColumnSetsTable->generateNextUserTrid();
}

std::uint64_t Database::generateNextColumnSetColumnId(bool system)
{
    return system ? (m_sysColumnSetColumnsTable
                             ? m_sysColumnSetColumnsTable->generateNextSystemTrid()
                             : ++m_tmpTridCounters.m_lastColumnSetColumnId)
                  : m_sysColumnSetColumnsTable->generateNextUserTrid();
}

std::uint64_t Database::generateNextConstraintDefinitionId(bool system)
{
    return system ? (m_sysConstraintDefsTable ? m_sysConstraintDefsTable->generateNextSystemTrid()
                                              : ++m_tmpTridCounters.m_lastConstraintDefinitionId)
                  : m_sysConstraintDefsTable->generateNextUserTrid();
}

std::uint64_t Database::generateNextConstraintId(bool system)
{
    return system ? (m_sysConstraintsTable ? m_sysConstraintsTable->generateNextSystemTrid()
                                           : ++m_tmpTridCounters.m_lastConstraintId)
                  : m_sysConstraintsTable->generateNextUserTrid();
}

std::uint64_t Database::generateNextColumnDefinitionConstraintId(bool system)
{
    return system ? (m_sysColumnDefConstraintsTable
                             ? m_sysColumnDefConstraintsTable->generateNextSystemTrid()
                             : ++m_tmpTridCounters.m_lastColumnDefinitionConstraintId)
                  : m_sysColumnDefConstraintsTable->generateNextUserTrid();
}

std::uint64_t Database::generateNextIndexId(bool system)
{
    return system ? (m_sysIndicesTable ? m_sysIndicesTable->generateNextSystemTrid()
                                       : ++m_tmpTridCounters.m_lastIndexId)
                  : m_sysIndicesTable->generateNextUserTrid();
}

std::uint64_t Database::generateNextIndexColumnId(bool system)
{
    return system ? (m_sysIndexColumnsTable ? m_sysIndexColumnsTable->generateNextSystemTrid()
                                            : ++m_tmpTridCounters.m_lastIndexColumnId)
                  : m_sysIndexColumnsTable->generateNextUserTrid();
}

void Database::checkConstraintType(const Table& table, const Column* column,
        const std::string& constraintName, const ConstraintDefinition& constraintDefinition,
        ConstraintType expectedType) const
{
    if (constraintDefinition.getType() == expectedType) return;
    if (column) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnConstraintTypeDoesNotMatch,
                static_cast<int>(constraintDefinition.getType()), static_cast<int>(expectedType),
                m_name, table.getName(), column->getName(), constraintName, m_uuid, table.getId(),
                column->getId(), 0, constraintDefinition.getId());
    } else {
        throwDatabaseError(IOManagerMessageId::kErrorTableConstraintTypeDoesNotMatch,
                static_cast<int>(constraintDefinition.getType()), static_cast<int>(expectedType),
                m_name, table.getName(), constraintName, m_uuid, table.getId(), 0,
                constraintDefinition.getId());
    }
}

void Database::checkConstraintType(const Table& table, const Column* column,
        const ConstraintRecord& constraintRecord, ConstraintType expectedType) const
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_constraintDefinitionRegistry.byId();
    const auto it = index.find(constraintRecord.m_constraintDefinitionId);
    if (it == index.end()) {
        throwDatabaseError(IOManagerMessageId::kErrorConstraintDefinitionDoesNotExist, m_name,
                constraintRecord.m_constraintDefinitionId);
    }
    if (it->m_type == expectedType) return;
    if (column) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnConstraintTypeDoesNotMatch,
                static_cast<int>(it->m_type), static_cast<int>(expectedType), m_name,
                table.getName(), column->getName(), constraintRecord.m_name, m_uuid, table.getId(),
                column->getId(), constraintRecord.m_id, constraintRecord.m_constraintDefinitionId);
    } else {
        throwDatabaseError(IOManagerMessageId::kErrorTableConstraintTypeDoesNotMatch,
                static_cast<int>(it->m_type), static_cast<int>(expectedType), m_name,
                table.getName(), constraintRecord.m_name, m_uuid, table.getId(),
                constraintRecord.m_id, constraintRecord.m_constraintDefinitionId);
    }
}

void Database::registerTable(const Table& table)
{
    std::lock_guard lock(m_mutex);
    m_tableRegistry.emplace(table);
}

void Database::registerColumn(const Column& column)
{
    std::lock_guard lock(m_mutex);
    m_columnRegistry.emplace(column);
}

void Database::registerColumnDefinition(const ColumnDefinition& columnDefinition)
{
    std::lock_guard lock(m_mutex);
    m_columnDefinitionRegistry.emplace(columnDefinition);
}

void Database::updateColumnDefinitionRegistration(const ColumnDefinition& columnDefinition)
{
    std::lock_guard lock(m_mutex);
    auto& index = m_columnDefinitionRegistry.byId();
    const auto it = index.find(columnDefinition.getId());
    if (it == index.cend()) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnDefinitionDoesNotExist2, m_name,
                columnDefinition.getId());
    }
    ColumnDefinitionRecord newRecord(columnDefinition);
    index.replace(it, newRecord);
}

void Database::registerColumnSet(const ColumnSet& columnSet)
{
    std::lock_guard lock(m_mutex);
    m_columnSetRegistry.emplace(columnSet);
}

void Database::updateColumnSetRegistration(const ColumnSet& columnSet)
{
    std::lock_guard lock(m_mutex);
    auto& index = m_columnSetRegistry.byId();
    const auto it = index.find(columnSet.getId());
    if (it == index.end()) {
        throwDatabaseError(
                IOManagerMessageId::kErrorColumnSetDoesNotExist, m_name, columnSet.getId());
    }
    ColumnSetRecord newRecord(columnSet);
    index.replace(it, newRecord);
}

void Database::registerConstraintDefinition(const ConstraintDefinition& constraintDefinition)
{
    std::lock_guard lock(m_mutex);
    m_constraintDefinitionRegistry.emplace(constraintDefinition);
}

void Database::registerConstraint(const Constraint& constraint)
{
    std::lock_guard lock(m_mutex);
    m_constraintRegistry.emplace(constraint);
}

void Database::registerIndex(const Index& index)
{
    std::lock_guard lock(m_mutex);
    m_indexRegistry.emplace(index);
}

TablePtr Database::createUserTable(std::string&& name, TableType type,
        const std::vector<SimpleColumnSpecification>& columnSpecs, std::uint32_t currentUserId,
        std::optional<std::string>&& description)
{
    std::vector<ColumnSpecification> columnSpecs2;
    if (!columnSpecs.empty()) {
        columnSpecs2.reserve(columnSpecs.size());
        for (const auto& columnInfo : columnSpecs)
            columnSpecs2.emplace_back(columnInfo);
    }
    return createUserTable(
            std::move(name), type, columnSpecs2, currentUserId, std::move(description));
}

TablePtr Database::createUserTable(std::string&& name, TableType type,
        const std::vector<ColumnSpecification>& columnSpecs, std::uint32_t currentUserId,
        std::optional<std::string>&& description)
{
    if (type != TableType::kDisk)
        throwDatabaseError(IOManagerMessageId::kErrorTableTypeNotSupported, static_cast<int>(type));

    if (isSystemDatabase() && !canContainUserTables())
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateUserTablesInSystemDatabase);

    LOG_DEBUG << "Database " << m_name << ": Creating user table " << name;

    std::lock_guard lock(m_mutex);

    if (isTableExistsUnlocked(name))
        throwDatabaseError(IOManagerMessageId::kErrorTableAlreadyExists, m_name, name);

    std::vector<char> columnPresent(columnSpecs.size());
    std::vector<CompoundDatabaseError::ErrorRecord> errors;

    std::unordered_set<std::reference_wrapper<const std::string>, std::hash<std::string>,
            std::equal_to<std::string>>
            knownColumns, knownConstraints;

    std::unordered_map<unsigned, std::size_t> constraintCounts;

    const auto& constraintIndex = m_constraintRegistry.byName();

    for (const auto& columnSpec : columnSpecs) {
        // Validate column name
        if (!isValidDatabaseObjectName(columnSpec.m_name)) {
            errors.push_back(makeDatabaseError(
                    IOManagerMessageId::kErrorInvalidColumnName, columnSpec.m_name));
            continue;
        }

        // Check for a duplicate column name
        if (!knownColumns.insert(columnSpec.m_name).second) {
            errors.push_back(makeDatabaseError(
                    IOManagerMessageId::kErrorCreateTableDuplicateColumnName, columnSpec.m_name));
            continue;
        }

        // Check constraint names for uniqueness with existing constaints and each other.
        constraintCounts.clear();
        for (const auto& constraintSpec : columnSpec.m_constraints) {
            // Assume empty names are unique (will be replaced with automatic name later).
            ++constraintCounts[static_cast<unsigned>(constraintSpec.m_type)];
            if (constraintSpec.m_name.empty()) continue;
            if (!isValidDatabaseObjectName(constraintSpec.m_name)) {
                errors.push_back(makeDatabaseError(
                        IOManagerMessageId::kErrorInvalidConstraintName, columnSpec.m_name));
                continue;
            }
            if (!knownConstraints.insert(constraintSpec.m_name).second) {
                errors.push_back(makeDatabaseError(
                        IOManagerMessageId::kErrorCreateTableDuplicateConstraintName,
                        constraintSpec.m_name));
            }
            if (constraintIndex.count(constraintSpec.m_name) > 0) {
                errors.push_back(
                        makeDatabaseError(IOManagerMessageId::kErrorConstraintAlreadyExists, m_name,
                                constraintSpec.m_name));
            }
        }

        // Check that each type of constraint is specified only once
        for (const auto& e : constraintCounts) {
            if (e.second > 1) {
                DBG_LOG_DEBUG("Errors in the column " << columnSpec.m_name);
                errors.push_back(makeDatabaseError(
                        IOManagerMessageId::kErrorCreateTableDuplicateColumnConstraintType,
                        getConstraintTypeName(static_cast<ConstraintType>(e.first)),
                        columnSpec.m_name));
            }
        }
    }

    if (!errors.empty()) {
#ifdef _DEBUG
        LOG_ERROR << "Multiple errors (" << errors.size() << "):";
        for (const auto& error : errors) {
            LOG_ERROR << '[' << error.m_errorCode << "] " << error.m_message;
        }
#endif
        throw CompoundDatabaseError(std::move(errors));
    }

    const auto table = createTable(std::move(name), type, 0, std::move(description));

    std::vector<ColumnPtr> columns;
    columns.reserve(columnSpecs.size() + 1);

    const auto masterColumn = table->getMasterColumn();
    columns.push_back(masterColumn);

    for (const auto& columnSpec : columnSpecs)
        columns.push_back(table->createColumn(ColumnSpecification(columnSpec)));

    table->closeCurrentColumnSet();

    const TransactionParameters tp(currentUserId, generateNextTransactionId());
    recordTableDefinition(*table, tp);

    // Preallocate first block for each column
    for (const auto& column : columns) {
        LOG_DEBUG << "Pre-allocating data block for the column " << column->makeDisplayName();
        column->selectAvailableBlock(1);
        if (column->isMasterColumn()) {
            LOG_DEBUG << "Pre-allocating index storage for the column "
                      << column->makeDisplayName();
            std::uint8_t key[8];
            ::pbeEncodeUInt64(1, key);
            column->getMasterColumnMainIndex()->preallocate(key);
        }
    }

    return table;
}

void Database::dropTable(const std::string& name, bool tableMustExists, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_mutex);

    auto table = findTableUnlocked(name);
    if (!table) {
        if (!tableMustExists) return;
        throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotExist, m_name, name);
    }

    const auto tableId = table->getId();
    const auto fullTableName = table->makeDisplayName();
    const auto tableDataDir = table->getDataDir();

    // DROP TABLE algorithm:
    // 1. Collect all sorts of objects related to table.
    // 2. Determine which of them must be removed.
    // 3. Remove these objects.
    // 4. Update indices of the affected system tables.
    // 5. Free in-memory data structures.
    // 6. Remove records from internal dictionaries.
    // 7. Remove data directory of the table.
    //
    // Hierarchy of the affected system objects:
    //
    // Table
    // |--> Table has column sets
    // |    |--> Column set has column set columns
    // |--> Table has columns
    // |    |--> Column has column definitions
    // |         |--> Column definition has column definition constraints
    // |              |--> Column definition constraint is related to constaint
    // |                   |--> Constaint is linked to constrain definition
    // |                        |--> Constraint definition can be shared by multiple constraints
    // |--> Table has indices
    //      |--> Index has indexed columns

    // Below we use lots of ordered maps in order to ensure stable sequence of delete actions

    // Key is column set ID, value is list of ColumnSetColumn IDs
    std::map<std::uint64_t, std::vector<std::uint64_t>> columnSetsToRemove;

    // Key is column ID, value is map where: key is ColumnDefinition ID, and value is another map,
    // where: key is ColumnDefinitionConstaint ID, value is constraint ID
    std::map<std::uint64_t, std::map<std::uint64_t, std::map<std::uint64_t, std::uint64_t>>>
            columnsToRemove;

    // Key is ConstraintDefinition ID, value is list of correspodning ColumnDefinitionConstaint IDs
    std::map<std::uint64_t, std::unordered_set<std::uint64_t>> constraintDefinitionsToRemove;

    // Key is index ID, value is list of index column IDs
    std::map<std::uint64_t, std::vector<std::uint64_t>> indicesToRemove;

    // Determine system objects to be deleted

    auto& columnsById = m_columnRegistry.byId();
    auto& constraintsById = m_constraintRegistry.byId();

    for (auto columnSetsRange = m_columnSetRegistry.byTableId().equal_range(tableId);
            columnSetsRange.first != columnSetsRange.second; ++columnSetsRange.first) {
        // Capture column set columns
        auto& columnSet = *columnSetsRange.first;
        std::vector<std::uint64_t> columnSetColumns0;
        columnSetColumns0.reserve(columnSet.m_columns.size());
        std::transform(columnSet.m_columns.byId().cbegin(), columnSet.m_columns.byId().cend(),
                std::back_inserter(columnSetColumns0),
                [](const auto& columnSetColumnRecord) noexcept {
                    return columnSetColumnRecord.m_id;
                });
        std::sort(columnSetColumns0.begin(), columnSetColumns0.end());
        auto& columnSetColumns =
                columnSetsToRemove.emplace(columnSet.m_id, std::move(columnSetColumns0))
                        .first->second;

        // Capture columns
        for (const auto& columnSetColumnId : columnSetColumns) {
            DBG_LOG_DEBUG("dropTable: Processing ColumnSet #"
                          << columnSet.m_id << " ColumnSetColumn #" << columnSetColumnId);
            const auto& columnSetColumnRecord = *columnSet.m_columns.byId().find(columnSetColumnId);
            if (columnsToRemove.count(columnSetColumnRecord.m_columnId) > 0) continue;

            DBG_LOG_DEBUG("dropTable: Processing Column #" << columnSetColumnRecord.m_columnId);

            // Skip non-existing columns - is it good?
            const auto columnIt = columnsById.find(columnSetColumnRecord.m_columnId);
            if (columnIt == columnsById.end()) continue;

            // Capture column definitions
            std::map<std::uint64_t, std::map<std::uint64_t, std::uint64_t>>
                    columnDefinitionsToRemove;
            const auto& columnDefsIndex = m_columnDefinitionRegistry.byColumnIdAndId();
            for (auto range = std::make_pair(columnDefsIndex.lower_bound(std::make_pair(
                                                     columnSetColumnRecord.m_columnId, 0ULL)),
                         columnDefsIndex.lower_bound(
                                 std::make_pair(columnSetColumnRecord.m_columnId + 1, 0ULL)));
                    range.first != range.second; ++range.first) {
                const auto& columnDefinitionRecord = *range.first;
                DBG_LOG_DEBUG("dropTable: Processing ColumnDefinition #"
                              << columnDefinitionRecord.m_id << " Column #"
                              << columnDefinitionRecord.m_columnId);
                std::map<std::uint64_t, std::uint64_t> columnDefConstraints;
                for (const auto& columnDefinitionConstraintRecord :
                        columnDefinitionRecord.m_constraints.byId()) {
                    DBG_LOG_DEBUG("dropTable: Processing ColumnDefinition #"
                                  << columnDefinitionRecord.m_id << " ColumnDefinitionConstraint #"
                                  << columnDefinitionConstraintRecord.m_id);

                    // Record constraint
                    const auto constraintIt =
                            constraintsById.find(columnDefinitionConstraintRecord.m_constraintId);
                    if (constraintIt != constraintsById.end()) {
                        DBG_LOG_DEBUG("dropTable: Processing ColumnDefinition #"
                                      << columnDefinitionRecord.m_id
                                      << " ColumnDefinitionConstraint #"
                                      << columnDefinitionConstraintRecord.m_id << " Constraint #"
                                      << columnDefinitionConstraintRecord.m_constraintId);
                        columnDefConstraints.emplace(columnDefinitionConstraintRecord.m_id,
                                columnDefinitionConstraintRecord.m_constraintId);

                        DBG_LOG_DEBUG("dropTable: Processing Constraint #"
                                      << columnDefinitionConstraintRecord.m_constraintId
                                      << " ConstraintDefinition #"
                                      << constraintIt->m_constraintDefinitionId);
                        constraintDefinitionsToRemove[constraintIt->m_constraintDefinitionId]
                                .insert(columnDefinitionConstraintRecord.m_constraintId);
                    }
                }
                columnDefinitionsToRemove.emplace(
                        columnDefinitionRecord.m_id, std::move(columnDefConstraints));
            }
            columnsToRemove.emplace(
                    columnSetColumnRecord.m_columnId, std::move(columnDefinitionsToRemove));
        }
    }

    // Determine which constraint definitions should be removed.
    // For this, check if constraint has links to something else
    // than captured ColumnDefinitionConstraint IDs.
    for (auto it = constraintDefinitionsToRemove.begin();
            it != constraintDefinitionsToRemove.end();) {
        DBG_LOG_DEBUG("dropTable: Processing ConstraintDefintion #" << it->first);
        std::unordered_set<std::uint64_t> allConstraints;
        for (auto range = m_constraintRegistry.byConstraintDefinitionId().equal_range(it->first);
                range.first != range.second; ++range.first) {
            allConstraints.insert(range.first->m_id);
        }
        if (allConstraints == it->second)
            ++it;
        else {
            DBG_LOG_DEBUG("dropTable: Not removing ConstraintDefintion #" << it->first);
            it = constraintDefinitionsToRemove.erase(it);
        }
    }

    // Determine indices to delete
    for (auto tableIndicesRange = m_indexRegistry.byTableId().equal_range(tableId);
            tableIndicesRange.first != tableIndicesRange.second; ++tableIndicesRange.first) {
        const auto& indexRecord = *tableIndicesRange.first;
        DBG_LOG_DEBUG("dropTable: Processing Index #" << indexRecord.m_id << ": "
                                                      << indexRecord.m_columns.size()
                                                      << " columns to remove");
        std::vector<std::uint64_t> indexColumnsToRemove;
        indexColumnsToRemove.reserve(indexRecord.m_columns.size());
        std::transform(indexRecord.m_columns.byId().cbegin(), indexRecord.m_columns.byId().cend(),
                std::back_inserter(indexColumnsToRemove),
                [](const auto& indexColumnRecord) noexcept { return indexColumnRecord.m_id; });
        indicesToRemove.emplace(indexRecord.m_id, std::move(indexColumnsToRemove));
    }

    // Delete records in tables
    // NOTE: Later on, all affected system tables must be write-locked before doing this

    const TransactionParameters tp(currentUserId, generateNextTransactionId());

    class SystemTableRowDeleter {
    public:
        SystemTableRowDeleter(Table& table, const TransactionParameters& tp,
                const std::string& tableName) noexcept
            : m_table(table)
            , m_tp(tp)
            , m_tableName(tableName)
            , m_nextBlockId(0)
        {
        }

        DECLARE_NONCOPYABLE(SystemTableRowDeleter);

        void deleteRow(std::uint64_t trid)
        {
            LOG_DEBUG << "Database " << m_table.getDatabaseName()
                      << ": DROP TABLE: " << m_table.getName() << ": Removing TRID #" << trid;
            const auto deleteResult = m_table.deleteRow(trid, m_tp, false);
            if (std::get<0>(deleteResult)) {
                if (!m_rollbackAddress) m_rollbackAddress = std::get<2>(deleteResult);
                m_nextBlockId = std::get<3>(deleteResult).getBlockId();
            }
        }

        void rollbackIfChanged()
        {
            if (m_rollbackAddress) {
                try {
                    LOG_DEBUG << "Database " << m_table.getDatabaseName()
                              << ": DROP TABLE: Rolling back " << m_table.getName();
                    m_table.getMasterColumn()->rollbackToAddress(m_rollbackAddress, m_nextBlockId);
                } catch (std::exception& ex) {
                    LOG_ERROR << "Database " << m_table.getDatabaseName() << ": DROP TABLE "
                              << m_tableName << ": Rollback failed for the system table "
                              << m_table.getName() << ": " << ex.what();
                    throw;
                } catch (...) {
                    LOG_ERROR << "Database " << m_table.getDatabaseName() << ": DROP TABLE "
                              << m_tableName << ": Rollback failed for the system table "
                              << m_table.getName() << ": other error";
                    throw;
                }
            }
        }

        void updateMainIndex(std::uint64_t trid)
        {
            try {
                LOG_DEBUG << "Database " << m_table.getDatabaseName()
                          << ": DROP TABLE: " << m_table.getName()
                          << ": Updating index for the TRID #" << trid;
                m_table.getMasterColumn()->eraseFromMasterColumnMainIndex(trid);
            } catch (std::exception& ex) {
                LOG_ERROR << "Database " << m_table.getDatabaseName() << ": DROP TABLE "
                          << m_tableName << ": Update main index failed for the system table "
                          << m_table.getName() << ": " << ex.what();
                throw;
            } catch (...) {
                LOG_ERROR << "Database " << m_table.getDatabaseName() << ": DROP TABLE "
                          << m_tableName << ": Update main index failed for the system table "
                          << m_table.getName() << ": other error";
                throw;
            }
        }

    private:
        Table& m_table;
        const TransactionParameters& m_tp;
        const std::string& m_tableName;
        ColumnDataAddress m_rollbackAddress;
        std::uint64_t m_nextBlockId;
    };

    SystemTableRowDeleter sysIndexColumnsDeleter(*m_sysIndexColumnsTable, tp, name);
    SystemTableRowDeleter sysIndicesDeleter(*m_sysIndicesTable, tp, name);
    SystemTableRowDeleter sysColumnSetColumnsDeleter(*m_sysColumnSetColumnsTable, tp, name);
    SystemTableRowDeleter sysColumnSetsDeleter(*m_sysColumnSetsTable, tp, name);
    SystemTableRowDeleter sysTablesDeleter(*m_sysTablesTable, tp, name);
    SystemTableRowDeleter sysConstraintsDeleter(*m_sysConstraintsTable, tp, name);
    SystemTableRowDeleter sysColumnDefConstraintsDeleter(*m_sysColumnDefConstraintsTable, tp, name);
    SystemTableRowDeleter sysColumnDefsDeleter(*m_sysColumnDefsTable, tp, name);
    SystemTableRowDeleter sysColumnsDeleter(*m_sysColumnsTable, tp, name);
    SystemTableRowDeleter sysConstraintDefsDeleter(*m_sysConstraintDefsTable, tp, name);

    try {
        for (const auto& e : indicesToRemove) {
            for (const auto indexColumnId : e.second)
                sysIndexColumnsDeleter.deleteRow(indexColumnId);
            sysIndicesDeleter.deleteRow(e.first);
        }

        for (const auto& e : columnSetsToRemove) {
            for (const auto columnSetColumnId : e.second)
                sysColumnSetColumnsDeleter.deleteRow(columnSetColumnId);
            sysColumnSetsDeleter.deleteRow(e.first);
        }

        sysTablesDeleter.deleteRow(tableId);

        for (const auto& e : columnsToRemove) {
            for (const auto& e2 : e.second) {
                for (const auto& e3 : e2.second) {
                    sysConstraintsDeleter.deleteRow(e3.second);
                    sysColumnDefConstraintsDeleter.deleteRow(e3.first);
                }
                sysColumnDefsDeleter.deleteRow(e2.first);
            }
            sysColumnsDeleter.deleteRow(e.first);
        }

        for (const auto& e : constraintDefinitionsToRemove)
            sysConstraintDefsDeleter.deleteRow(e.first);

    } catch (std::exception& ex) {
        // Rollback changed tables
        sysConstraintDefsDeleter.rollbackIfChanged();
        sysColumnsDeleter.rollbackIfChanged();
        sysColumnDefsDeleter.rollbackIfChanged();
        sysColumnDefConstraintsDeleter.rollbackIfChanged();
        sysConstraintsDeleter.rollbackIfChanged();
        sysTablesDeleter.rollbackIfChanged();
        sysColumnSetsDeleter.rollbackIfChanged();
        sysColumnSetColumnsDeleter.rollbackIfChanged();
        sysIndicesDeleter.rollbackIfChanged();
        sysIndexColumnsDeleter.rollbackIfChanged();
        throw;
    }

    // Update main indexes

    for (const auto& e : indicesToRemove) {
        for (const auto indexColumnId : e.second)
            sysIndexColumnsDeleter.updateMainIndex(indexColumnId);
        sysIndicesDeleter.updateMainIndex(e.first);
    }

    for (const auto& e : columnSetsToRemove) {
        for (const auto columnSetColumnId : e.second)
            sysColumnSetColumnsDeleter.updateMainIndex(columnSetColumnId);
        sysColumnSetsDeleter.updateMainIndex(e.first);
    }

    sysTablesDeleter.updateMainIndex(tableId);

    for (const auto& e : columnsToRemove) {
        for (const auto& e2 : e.second) {
            for (const auto& e3 : e2.second) {
                sysConstraintsDeleter.updateMainIndex(e3.second);
                sysColumnDefConstraintsDeleter.updateMainIndex(e3.first);
            }
            sysColumnDefsDeleter.updateMainIndex(e2.first);
        }
        sysColumnsDeleter.updateMainIndex(e.first);
    }

    for (const auto& e : constraintDefinitionsToRemove)
        sysConstraintDefsDeleter.updateMainIndex(e.first);

    // Remove in-memory objects from collections, starting from table and further

    table.reset();
    m_tables.erase(tableId);

    for (const auto& e : constraintDefinitionsToRemove)
        m_constraintDefinitions.erase(e.first);

    // Remove records from registries

    auto& indicesById = m_indexRegistry.byId();
    for (const auto& e : indicesToRemove)
        indicesById.erase(e.first);

    auto& columnSetsById = m_columnSetRegistry.byId();
    for (const auto& e : columnSetsToRemove)
        columnSetsById.erase(e.first);

    m_tableRegistry.byId().erase(tableId);

    auto& columnDefinitionsById = m_columnDefinitionRegistry.byId();
    for (const auto& e : columnsToRemove) {
        columnsById.erase(e.first);
        for (const auto& e2 : e.second) {
            columnDefinitionsById.erase(e2.first);
            for (const auto& e3 : e2.second)
                constraintsById.erase(e3.second);
        }
    }

    auto& constraintDefinitionsById = m_constraintDefinitionRegistry.byId();
    for (const auto& e : constraintDefinitionsToRemove)
        constraintDefinitionsById.erase(e.first);

    // Finally, remove data directory
    system_error_code ec;
    fs::remove_all(tableDataDir, ec);
    if (ec) {
        LOG_WARNING << "DROP TABLE " << fullTableName << ": Can't remove data directory '"
                    << tableDataDir << "': " << ec.value() << ": " << ec.message();
    }
}

io::FilePtr Database::createFile(
        const std::string& path, int extraFlags, int createMode, off_t initialSize) const
{
    if (m_cipher) {
        return std::make_unique<io::EncryptedFile>(path, extraFlags, createMode,
                m_encryptionContext, m_decryptionContext, initialSize);
    } else
        return std::make_unique<io::NormalFile>(path, extraFlags, createMode, initialSize);
}

io::FilePtr Database::openFile(const std::string& path, int extraFlags) const
{
    if (m_cipher) {
        return std::make_unique<io::EncryptedFile>(
                path, extraFlags, m_encryptionContext, m_decryptionContext);
    } else
        return std::make_unique<io::NormalFile>(path, extraFlags);
}

// ---- internal ----

void Database::checkTableBelongsToThisDatabase(const Table& table, const char* operationName) const
{
    if (&table.getDatabase() != this) {
        throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotBelongToDatabase, operationName,
                table.getName(), table.getDatabaseName(), table.getDatabaseUuid(), table.getId(),
                m_name, m_uuid);
    }
}

TablePtr Database::createTableUnlocked(std::string&& name, TableType type,
        std::uint64_t firstUserTrid, std::optional<std::string>&& description)
{
    if (m_tableRegistry.size() >= m_maxTableCount)
        throwDatabaseError(IOManagerMessageId::kErrorTooManyTables, m_name);

    if (m_tableRegistry.byName().count(name) > 0)
        throwDatabaseError(IOManagerMessageId::kErrorTableAlreadyExists, m_name, name);

    // Create table
    auto table = std::make_shared<Table>(
            *this, type, std::move(name), firstUserTrid, std::move(description));

    // Register table
    registerTable(*table);
    m_tables.emplace(table->getId(), table);
    return table;
}

TablePtr Database::loadSystemTable(const std::string& name)
{
    if (SIODB_UNLIKELY(m_tableRegistry.empty())) loadSystemObjectsInfo();
    auto table = findTableUnlocked(name);
    if (table) return table;
    throwDatabaseError(IOManagerMessageId::kErrorMissingSystemTable, m_name, name, m_id, 0);
}

Uuid Database::computeDatabaseUuid(
        const std::string& databaseName, std::time_t createTimestamp) noexcept
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, databaseName.c_str(), databaseName.length());
    MD5_Update(&ctx, &createTimestamp, sizeof(createTimestamp));
    Uuid result;
    MD5_Final(result.data, &ctx);
    return result;
}

void Database::createInitializationFlagFile() const
{
    const auto initFlagFile = utils::constructPath(m_dataDir, kInitializationFlagFile);
    std::ofstream ofs(initFlagFile);
    if (!ofs.is_open()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateDatabaseInitializationFlagFile,
                initFlagFile, m_name, m_uuid, "create file failed");
    }
    ofs.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        ofs << std::time(nullptr) << std::flush;
    } catch (std::exception& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateDatabaseInitializationFlagFile,
                initFlagFile, m_name, m_uuid, "write failed");
    }
}

void Database::checkDataConsistency()
{
    // Just by loading all tables we enforce data consistency check.
    for (const auto& e : m_tableRegistry.byName()) {
        const auto table = findTableChecked(e.m_id);
        LOG_DEBUG << "Table " << table->makeDisplayName() << " OK";
    }
}

BinaryValue Database::loadCipherKey() const
{
    if (!m_cipher) return BinaryValue();

    // Check file size
    const auto path = makeCipherKeyFilePath();
    system_error_code ec;
    const auto fileSize = fs::file_size(path, ec);
    if (fileSize == static_cast<std::uintmax_t>(-1)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenDatabaseCipherKeyFile, path, m_name,
                m_uuid, ec.value(), std::strerror(ec.value()));
    }
    if (m_cipher && fileSize < kCipherKeyFileMinSize) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseCipherKeyFileCorrupted, path, m_name,
                m_uuid, "File is too small");
    }
    if (fileSize > kCipherKeyFileMaxSize) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseCipherKeyFileCorrupted, path, m_name,
                m_uuid, std::strerror(EFBIG));
    }

    // Open cipher key file
    constexpr auto kOpenFlags = O_RDONLY | O_CLOEXEC;
    FDGuard fd(::open(path.c_str(), kOpenFlags));
    if (!fd.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenDatabaseCipherKeyFile, path, m_name,
                m_uuid, errorCode, std::strerror(errorCode));
    }

    // Read cipher key file
    BinaryValue encryptedKey(fileSize);
    const auto n = ::readExact(fd.getFD(), encryptedKey.data(), fileSize, kIgnoreSignals);
    if (n != fileSize) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadDatabaseCipherKeyFile, m_name,
                m_uuid, errorCode, std::strerror(errorCode), fileSize, n);
    }

    // Decrypt and deserialize key
    CipherKeyRecord cipherKeyRecord;
    try {
        const auto decryptedKey =
                m_instance.decryptWithMasterEncryption(encryptedKey.data(), encryptedKey.size());
        cipherKeyRecord.deserialize(decryptedKey.data(), decryptedKey.size());
    } catch (std::invalid_argument& ex) {
        std::ostringstream err;
        err << "Key decryption error: " << ex.what();
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseCipherKeyFileCorrupted, path, m_name,
                m_uuid, err.str());
    } catch (utils::DeserializationError& ex) {
        std::ostringstream err;
        err << "Key deserialization error: " << ex.what();
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseCipherKeyFileCorrupted, path, m_name,
                m_uuid, err.str());
    }

    if (cipherKeyRecord.m_id != (static_cast<std::uint64_t>(m_id) << 32)) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseCipherKeyFileCorrupted, path, m_name,
                m_uuid, "Cipher mistmatch");
    }

    if (cipherKeyRecord.m_cipherId != m_cipher->getCipherId()) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseCipherKeyFileCorrupted, path, m_name,
                m_uuid, "Cipher mistmatch");
    }

    if (cipherKeyRecord.m_key.size() != m_cipher->getKeySizeInBits() / 8) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseCipherKeyFileCorrupted, path, m_name,
                m_uuid, "Cipher key length mismatch");
    }

    return cipherKeyRecord.m_key;
}

void Database::saveCurrentCipherKey() const
{
    // Don't create this file if encryption is not used
    if (!m_cipher) return;

    // Create cipher key file
    const auto path = makeCipherKeyFilePath();
    constexpr auto kOpenFlags = O_CREAT | O_RDWR | O_CLOEXEC | O_NOATIME;
    FDGuard fd(::open(path.c_str(), kOpenFlags, kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateDatabaseCipherKeyFile, path,
                m_name, m_uuid, errorCode, std::strerror(errorCode));
    }

    // Serialize and encrypt database encryption key
    const CipherKeyRecord cipherKeyRecord((static_cast<std::uint64_t>(m_id) << 32),
            m_cipher->getCipherId(), BinaryValue(m_cipherKey));
    BinaryValue serializedKey(cipherKeyRecord.getSerializedSize());
    cipherKeyRecord.serializeUnchecked(serializedKey.data());
    const auto encryptedKey =
            m_instance.encryptWithMasterEncryption(serializedKey.data(), serializedKey.size());

    // Write encrypted key to file
    const auto n =
            ::writeExact(fd.getFD(), encryptedKey.data(), encryptedKey.size(), kIgnoreSignals);
    if (n != encryptedKey.size()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteDatabaseCipherKeyFile, m_name,
                m_uuid, errorCode, std::strerror(errorCode), encryptedKey.size(), n);
    }
}

std::string Database::makeCipherKeyFilePath() const
{
    return utils::constructPath(m_dataDir, kCipherKeyFileName);
}

std::unique_ptr<MemoryMappedFile> Database::createMetadataFile() const
{
    const auto path = makeMetadataFilePath();

    // Create metadata file
    constexpr auto kOpenFlags = O_CREAT | O_RDWR | O_CLOEXEC | O_NOATIME;
    FDGuard fd(::open(path.c_str(), kOpenFlags, kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateDatabaseMetadataFile, path, m_name,
                m_uuid, errorCode, std::strerror(errorCode));
    }

    // Write initial metadata
    const DatabaseMetadata initialMetadata(User::kSuperUserId);
    const auto n =
            ::writeExact(fd.getFD(), &initialMetadata, sizeof(initialMetadata), kIgnoreSignals);
    if (n != sizeof(initialMetadata)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteDatabaseMetadataFile, m_name,
                m_uuid, errorCode, std::strerror(errorCode), sizeof(initialMetadata), n);
    }

    fd.reset();

    return openMetadataFile();
}

std::unique_ptr<MemoryMappedFile> Database::openMetadataFile() const
{
    const auto path = makeMetadataFilePath();

    // Open metadata file
    constexpr auto kOpenFlags = O_RDWR | O_CLOEXEC | O_NOATIME;
    FDGuard fd(::open(path.c_str(), kOpenFlags, kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenDatabaseMetadataFile, path, m_name,
                m_uuid, m_name, m_uuid, errorCode, std::strerror(errorCode));
    }

    // Create memory mapping
    auto file = std::make_unique<MemoryMappedFile>(fd.getFD(), false,
            MemoryMappedFile::deduceMemoryProtectionMode(kOpenFlags), MAP_POPULATE, 0, 0);
    fd.release();
    file->setFdOwner();

    // Check metadata version
    const auto metadata = reinterpret_cast<DatabaseMetadata*>(file->getMappingAddress());
    const auto version = metadata->getVersion();
    if (version == 0xFFFFFFFFFFFFFFFFULL) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseMetadataFileCorrupted, path, m_name,
                m_uuid, "Invalid metadata version");
    }
    if (version > DatabaseMetadata::kCurrentVersion) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseMetadataFileCorrupted, path, m_name,
                m_uuid, "Unsupported metadata version");
    }

    // NOTE: upgrade metadata here

    metadata->adjustByteOrder();

    // Check schema version
    if (metadata->getSchemaVersion() > DatabaseMetadata::kCurrentSchemaVersion) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseMetadataFileCorrupted, path, m_name,
                m_uuid, "Unsupported database schema version");
    }

    // NOTE: Maybe upgrade schema here

    // Check schema version
    if (metadata->getSchemaVersion() != DatabaseMetadata::kCurrentSchemaVersion) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseMetadataFileCorrupted, path, m_name,
                m_uuid, "Different database schema version");
    }

    return file;
}

std::string Database::makeMetadataFilePath() const
{
    return utils::constructPath(m_dataDir, kMetadataFileName);
}

std::string Database::makeSystemObjectsFilePath() const
{
    return utils::constructPath(m_dataDir, kSystemObjectsFileName);
}

std::string&& Database::validateDatabaseName(std::string&& databaseName)
{
    if (isValidDatabaseObjectName(databaseName)) return std::move(databaseName);
    throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);
}

std::string Database::findTableNameUnlocked(std::uint32_t tableId) const
{
    const auto& index = m_tableRegistry.byId();
    const auto it = index.find(tableId);
    if (it != index.cend()) return it->m_name;
    throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotExist, m_name, tableId);
}

TablePtr Database::findTableUnlocked(const std::string& tableName)
{
    const auto& index = m_tableRegistry.byName();
    const auto it = index.find(tableName);
    if (it == index.cend()) return nullptr;
    const auto itt = m_tables.find(it->m_id);
    if (itt != m_tables.end()) return itt->second;
    return loadTableUnlocked(*it);
}

TablePtr Database::findTableUnlocked(std::uint32_t tableId)
{
    const auto& index = m_tableRegistry.byId();
    const auto it = index.find(tableId);
    if (it == index.cend()) return nullptr;
    const auto itt = m_tables.find(tableId);
    if (itt != m_tables.end()) return itt->second;
    return loadTableUnlocked(*it);
}

TablePtr Database::loadTableUnlocked(const TableRecord& tableRecord)
{
    auto table = std::make_shared<Table>(*this, tableRecord);
    m_tables.emplace(table->getId(), table);
    return table;
}

ConstraintDefinitionPtr Database::createSystemConstraintDefinitionUnlocked(
        ConstraintType constraintType, requests::ConstExpressionPtr&& expression)
{
    bool existing = false;
    return createConstraintDefinitionUnlocked(
            true, constraintType, std::move(expression), existing);
}

ConstraintDefinitionPtr Database::createConstraintDefinitionUnlocked(bool system,
        ConstraintType constraintType, requests::ConstExpressionPtr&& expression, bool& existing)
{
    // Try to find exisitng matching constraint definition
    BinaryValue bv(expression->getSerializedSize());
    expression->serializeUnchecked(bv.data());
    ConstraintDefinitionRecord constraintDefinitionRecord(0, constraintType, std::move(bv));
    for (auto r = m_constraintDefinitionRegistry.byHash().equal_range(
                 constraintDefinitionRecord.m_hash);
            r.first != r.second; ++r.first) {
        if ((r.first->m_id < kFirstUserTableConstraintDefinitionId) == system
                && r.first->isEqualDefinition(constraintDefinitionRecord)) {
            // Matching constraint definition found
            const auto it = m_constraintDefinitions.find(constraintDefinitionRecord.m_id);
            if (it != m_constraintDefinitions.end()) {
                existing = true;
                return it->second;
            }
            auto constraintDefinition = loadConstraintDefinitionUnlocked(*r.first);
            existing = true;
            return constraintDefinition;
        }
    }

    // There is no matching constraint definition, so create a new one
    auto constraintDefinition = std::make_shared<ConstraintDefinition>(
            system, *this, constraintType, std::move(expression));
    constraintDefinitionRecord.m_id = constraintDefinition->getId();
    m_constraintDefinitionRegistry.insert(std::move(constraintDefinitionRecord));
    existing = false;
    return constraintDefinition;
}

ConstraintDefinitionPtr Database::findConstraintDefinitionUnlocked(
        std::uint64_t constraintDefinitionId)
{
    const auto& index = m_constraintDefinitionRegistry.byId();
    const auto it = index.find(constraintDefinitionId);
    if (it == index.cend()) return nullptr;
    const auto itcd = m_constraintDefinitions.find(constraintDefinitionId);
    if (itcd != m_constraintDefinitions.end()) return itcd->second;
    return loadConstraintDefinitionUnlocked(*it);
}

ConstraintDefinitionPtr Database::loadConstraintDefinitionUnlocked(
        const ConstraintDefinitionRecord& constraintDefinitionRecord)
{
    auto constraintDefinition =
            std::make_shared<ConstraintDefinition>(*this, constraintDefinitionRecord);
    m_constraintDefinitions.emplace(constraintDefinition->getId(), constraintDefinition);
    return constraintDefinition;
}

std::string Database::ensureDataDir(bool create) const
{
    auto dataDir = utils::constructPath(m_instance.getDataDir(), kDatabaseDataDirPrefix, m_uuid);
    const auto initFlagFile = utils::constructPath(dataDir, kInitializationFlagFile);
    const auto initFlagFileExists = fs::exists(initFlagFile);
    if (create) {
        // Check that database doesn't exist
        if (initFlagFileExists)
            throwDatabaseError(IOManagerMessageId::kErrorDatabaseAlreadyExists, m_name);

        // Create data directory
        try {
            const fs::path dataDirPath(dataDir);
            if (fs::exists(dataDirPath)) fs::remove_all(dataDirPath);
            fs::create_directories(dataDirPath);
        } catch (fs::filesystem_error& ex) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateDatabaseDataDir, dataDir,
                    m_name, m_uuid, ex.code().value(), ex.code().message());
        }
    } else {
        // Check that database is initialized
        if (!boost::filesystem::exists(dataDir)) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorDatabaseDataFolderDoesNotExist, m_name, dataDir);
        }

        if (!initFlagFileExists) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorDatabaseInitFileDoesNotExist, m_name, initFlagFile);
        }
    }
    return dataDir;
}

}  // namespace siodb::iomgr::dbengine
