// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Database.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Column.h"
#include "Constraint.h"
#include "Index.h"
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

// STL headers
#include <numeric>

namespace siodb::iomgr::dbengine {

void Database::readAllTables()
{
    LOG_DEBUG << "Database " << m_name << ": Reading all tables.";

    // Obtain columns
    const auto masterColumn = m_sysTablesTable->getMasterColumn();
    const auto typeColumn = m_sysTablesTable->findColumnChecked(kSysTables_Type_ColumnName);
    const auto nameColumn = m_sysTablesTable->findColumnChecked(kSysTables_Name_ColumnName);
    const auto firstUserTridColumn =
            m_sysTablesTable->findColumnChecked(kSysTables_FirstUserTrid_ColumnName);
    const auto currentColumnSetIdColumn =
            m_sysTablesTable->findColumnChecked(kSysTables_CurrentColumnSetId_ColumnName);
    const auto descriptionColumn =
            m_sysTablesTable->findColumnChecked(kSysTables_Description_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name << ": readAllTables: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysTablesTable->getName(), m_uuid, m_sysTablesTable->getId(), 1);
    }
    if (maxTrid == 0) {
        m_tableRegistry.clear();
        LOG_DEBUG << "Database " << m_name << ": There are no tables.";
        return;
    }

    const auto expectedColumnCount = m_sysTablesTable->getColumnCount() - 1;

    bool hasInvalidTables = false;
    TableRegistry reg;
    IndexValue indexValue;
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        if (index->findValue(currentKey, indexValue.m_data, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysTablesTable->getName(), m_uuid, m_sysTablesTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysTablesTable->getName(), m_uuid, m_sysTablesTable->getId(),
                    mcrAddr.getBlockId(), mcrAddr.getOffset(), expectedColumnCount,
                    mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant typeValue, nameValue, firstUserTridValue, currentColumnSetIdValue, descriptionValue;
        std::size_t colIndex = 0;
        typeColumn->readRecord(columnRecords.at(colIndex++).getAddress(), typeValue);
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue);
        firstUserTridColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), firstUserTridValue);
        currentColumnSetIdColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), currentColumnSetIdValue);
        descriptionColumn->readRecord(columnRecords.at(colIndex++).getAddress(), descriptionValue);

        const auto tableId = static_cast<std::uint32_t>(mcr.getTableRowId());
        const auto tableType = typeValue.asInt32();
        const auto name = nameValue.asString();
        const auto firstUserTrid = firstUserTridValue.asUInt64();
        const auto currentColumnSetId = currentColumnSetIdValue.asUInt64();

        // Validate table type
        if (tableType < static_cast<int>(TableType::kDisk)
                || tableType >= static_cast<int>(TableType::kMax)) {
            hasInvalidTables = true;
            LOG_ERROR << "Database " << m_name << ": readAllTables: Invalid type " << tableType
                      << " of the table #" << tableId << '.';
            continue;
        }

        // Validate name
        if (!isValidDatabaseObjectName(*name)) {
            hasInvalidTables = true;
            LOG_ERROR << "Database " << m_name << ": readAllTables: Invalid name '" << *name
                      << "' of the table #" << tableId << '.';
            continue;
        }

        // Add table record
        TableRecord tableRecord(tableId, static_cast<TableType>(tableType), std::move(*name),
                firstUserTrid, currentColumnSetId, descriptionValue.asOptionalString());
        LOG_DEBUG << "Database " << m_name << ": readAllTables: Table #" << tableRecord.m_id << " '"
                  << tableRecord.m_name << '\'';
        reg.insert(std::move(tableRecord));
    } while (index->findNextKey(currentKey, nextKey));

    if (hasInvalidTables) throw std::runtime_error("There are invalid table records");

    m_tableRegistry.swap(reg);
    LOG_DEBUG << "Database " << m_name << ": Read " << m_tableRegistry.size() << " tables.";
}

void Database::readAllColumnSets()
{
    LOG_DEBUG << "Database " << m_name << ": Reading all column sets.";

    // Obtain columns
    const auto masterColumn = m_sysColumnSetsTable->getMasterColumn();
    const auto tableIdColumn =
            m_sysColumnSetsTable->findColumnChecked(kSysColumnSets_TableId_ColumnName);
    const auto columnCountColumn =
            m_sysColumnSetsTable->findColumnChecked(kSysColumnSets_ColumnCount_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name << ": readAllColumnSets: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysColumnSetsTable->getName(), m_uuid, m_sysColumnSetsTable->getId(), 1);
    }
    if (maxTrid == 0) {
        m_columnSetRegistry.clear();
        LOG_DEBUG << "Database " << m_name << ": There are no column sets.";
        return;
    }

    const auto expectedColumnCount = m_sysColumnSetsTable->getColumnCount() - 1;

    const auto& tablesById = m_tableRegistry.byId();
    bool hasInvalidColumnSets = false;
    ColumnSetRegistry reg;
    IndexValue indexValue;
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        if (index->findValue(currentKey, indexValue.m_data, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysColumnSetsTable->getName(), m_uuid, m_sysColumnSetsTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysColumnSetsTable->getName(), m_uuid, m_sysColumnSetsTable->getId(),
                    mcrAddr.getBlockId(), mcrAddr.getOffset(), expectedColumnCount,
                    mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant tableIdValue, columnCountValue;
        std::size_t colIndex = 0;
        tableIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), tableIdValue);
        columnCountColumn->readRecord(columnRecords.at(colIndex++).getAddress(), columnCountValue);
        const auto columnSetId = mcr.getTableRowId();
        const auto tableId = tableIdValue.asUInt32();

        // Validate table ID
        if (tablesById.count(tableId) == 0) {
            hasInvalidColumnSets = true;
            LOG_ERROR << "Database " << m_name << ": readAllColumnSets: Invalid table ID "
                      << tableId << " in the column set #" << columnSetId << '.';
            continue;
        }

        // Add column set record
        ColumnSetRecord columnSetRecord(columnSetId, tableId);
        reg.insert(std::move(columnSetRecord));
        LOG_DEBUG << "Database " << m_name << ": readAllColumnSets: Column set #" << trid;
    } while (index->findNextKey(currentKey, nextKey));

    if (hasInvalidColumnSets) throw std::runtime_error("There are invalid column sets");

    m_columnSetRegistry.swap(reg);
    LOG_DEBUG << "Database " << m_name << ": Read " << m_columnSetRegistry.size()
              << " column sets.";
}

void Database::readAllColumns()
{
    LOG_DEBUG << "Database " << m_name << ": Reading all columns.";

    // Obtain columns
    const auto masterColumn = m_sysColumnsTable->getMasterColumn();
    const auto tableIdColumn = m_sysColumnsTable->findColumnChecked(kSysColumns_TableId_ColumnName);
    const auto dataTypeColumn =
            m_sysColumnsTable->findColumnChecked(kSysColumns_DataType_ColumnName);
    const auto nameColumn = m_sysColumnsTable->findColumnChecked(kSysColumns_Name_ColumnName);
    const auto stateColumn = m_sysColumnsTable->findColumnChecked(kSysColumns_State_ColumnName);
    const auto blockDataAreaSizeColumn =
            m_sysColumnsTable->findColumnChecked(kSysColumns_BlockDataAreaSize_ColumnName);
    const auto descriptionColumn =
            m_sysColumnsTable->findColumnChecked(kSysColumns_Description_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name << ": readAllColumns: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysColumnsTable->getName(), m_uuid, m_sysTablesTable->getId(), 1);
    }
    if (maxTrid == 0) {
        m_columnRegistry.clear();
        LOG_DEBUG << "Database " << m_name << ": There are no columns.";
        return;
    }

    struct TableColumns {
        std::vector<ColumnRecord> m_columns;
        std::unordered_map<std::string, std::size_t> m_columnNames;
    };

    std::unordered_map<std::uint32_t, TableColumns> columnsByTable;

    const auto expectedColumnCount = m_sysColumnsTable->getColumnCount() - 1;

    IndexValue indexValue;
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(key, &trid);
        //DBG_LOG_DEBUG("Database " << m_name << ": readAllColumns: Looking up TRID " << trid);
        if (index->findValue(currentKey, indexValue.m_data, 1) != 1)
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysColumnsTable->getName(), m_uuid, m_sysColumnsTable->getId(), 2);
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysColumnsTable->getName(), m_uuid, m_sysColumnsTable->getId(),
                    mcrAddr.getBlockId(), mcrAddr.getOffset(), expectedColumnCount,
                    mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant tableIdValue, dataTypeValue, nameValue, stateValue, blockDataAreaSizeValue,
                descriptionValue;
        std::size_t colIndex = 0;
        tableIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), tableIdValue);
        dataTypeColumn->readRecord(columnRecords.at(colIndex++).getAddress(), dataTypeValue);
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue);
        stateColumn->readRecord(columnRecords.at(colIndex++).getAddress(), stateValue);
        blockDataAreaSizeColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), blockDataAreaSizeValue);
        descriptionColumn->readRecord(columnRecords.at(colIndex++).getAddress(), descriptionValue);

        // Record column into the temporary map
        const auto columnId = mcr.getTableRowId();
        const auto tableId = tableIdValue.asUInt32();
        ColumnRecord columnRecord(columnId, std::move(*nameValue.asString()),
                static_cast<ColumnDataType>(dataTypeValue.asInt32()), tableId,
                static_cast<ColumnState>(stateValue.asInt32()), blockDataAreaSizeValue.asUInt32(),
                std::move(descriptionValue.asOptionalString()));
        auto& tableColumns = columnsByTable[tableId];
        ++tableColumns.m_columnNames[columnRecord.m_name];
        tableColumns.m_columns.push_back(std::move(columnRecord));
        LOG_DEBUG << "Database " << m_name << ": readAllColumns: Column #" << trid << " '"
                  << columnRecord.m_name << '\'';
    } while (index->findNextKey(currentKey, nextKey));

    const auto& tablesById = m_tableRegistry.byId();

    // Check that all columns correspond to existing tables
    for (const auto& e : columnsByTable) {
        if (tablesById.count(e.first) == 0) {
            LOG_WARNING << "Database " << m_name
                        << ": readAllColumns: " << e.second.m_columns.size()
                        << " columns related to non-existent table #" << e.first
                        << ". These columns are ignored.";
            for (const auto& column : e.second.m_columns) {
                LOG_WARNING << "Database " << m_name << ": readAllColumns: ... column #"
                            << column.m_id << " (" << column.m_name << ").";
            }
        }
    }

    // Check that every known table has columns at all and TRID in particular
    bool allTablesHaveColumns = true;
    bool allTablesHaveUniqueColumnNames = true;
    bool allTablesHaveValidColumnNames = true;
    bool allColumnsHaveValidDataTypes = true;
    bool allTablesHaveProperColumnPositions = true;
    bool allTablesHaveProperTridColumn = true;
    for (const auto& tableRecord : tablesById) {
        // Check that table has columns
        const auto it = columnsByTable.find(tableRecord.m_id);
        if (it == columnsByTable.end()) {
            allTablesHaveColumns = false;
            LOG_ERROR << "Database " << m_name << ": readAllColumns: No columns for the table #"
                      << tableRecord.m_id << " (" << tableRecord.m_name << ").";
            continue;
        }

        // At this point, we surely have some columns, and we need to perform some checks over them.
        const auto& tableColumns = it->second;

        // Check that all column names are unique
        if (tableColumns.m_columns.size() != tableColumns.m_columnNames.size()) {
            allTablesHaveUniqueColumnNames = false;
            LOG_ERROR << "Database " << m_name
                      << ": readAllColumns: Non-unique columns for the table #" << tableRecord.m_id
                      << " (" << tableRecord.m_name << ").";
            for (const auto& e : tableColumns.m_columnNames) {
                if (e.second > 1) {
                    LOG_ERROR << "Database " << m_name << ": readAllColumns: ... " << e.first
                              << " happens " << e.second << " times.";
                }
            }
        }

        // Check that all column names are valid
        for (const auto& columnInfo : tableColumns.m_columns) {
            if (!isValidDatabaseObjectName(columnInfo.m_name)) {
                allTablesHaveValidColumnNames = false;
                LOG_ERROR << "Database " << m_name
                          << ": readAllColumns: Invalid name of the column #" << columnInfo.m_id
                          << " (" << columnInfo.m_name << ").";
            }
        }

        // Check that all columns have valid data types
        for (const auto& columnInfo : tableColumns.m_columns) {
            if (columnInfo.m_dataType < 0
                    || columnInfo.m_dataType >= static_cast<int>(COLUMN_DATA_TYPE_MAX)) {
                allColumnsHaveValidDataTypes = false;
                LOG_ERROR << "Database " << m_name << ": readAllColumns: Invalid data type "
                          << columnInfo.m_dataType << " in the column #" << columnInfo.m_id << " ("
                          << columnInfo.m_name << ").";
            }
        }

        // Check that all tables have proper TRID column
        if (tableColumns.m_columnNames.count(kMasterColumnName) == 0) {
            allTablesHaveProperTridColumn = false;
            LOG_ERROR << "Database " << m_name << ": readAllColumns: able #" << tableRecord.m_id
                      << " (" << tableRecord.m_name << ") is missing master column.";
        } else {
            const auto itTrid = std::find_if(tableColumns.m_columns.cbegin(),
                    tableColumns.m_columns.cend(),
                    [](const auto& columnInfo) { return columnInfo.m_name == kMasterColumnName; });
            if (itTrid == tableColumns.m_columns.cend())
                throw std::runtime_error("Database::readAllColumns(): Internal error #1");
            if (itTrid->m_dataType != static_cast<int>(COLUMN_DATA_TYPE_UINT64)) {
                allTablesHaveProperTridColumn = false;
                LOG_ERROR << "Database " << m_name << ": readAllColumns: Table #"
                          << tableRecord.m_id << " (" << tableRecord.m_name
                          << ") has master column of the wrong data type: "
                          << static_cast<int>(COLUMN_DATA_TYPE_UINT64)
                          << " is expected, but the actual data type is " << itTrid->m_dataType
                          << '.';
            }
        }
    }

    // Ensure all required conditions are met
    if (!allTablesHaveColumns || !allTablesHaveUniqueColumnNames || !allTablesHaveValidColumnNames
            || !allColumnsHaveValidDataTypes || !allTablesHaveProperColumnPositions
            || !allTablesHaveProperTridColumn) {
        throw std::runtime_error("There are errors in table columns");
    }

    // Add column records to the temporary registry
    ColumnRegistry reg;
    for (const auto& tableRecord : tablesById) {
        const auto it = columnsByTable.find(tableRecord.m_id);
        if (it == columnsByTable.end()) continue;
        for (auto& columnRecord : it->second.m_columns) {
            reg.insert(std::move(columnRecord));
        }
    }

    // Replace normal registry
    m_columnRegistry.swap(reg);
    LOG_DEBUG << "Database " << m_name << ": Read " << m_columnRegistry.size() << " columns.";
}

void Database::readAllColumnDefs()
{
    LOG_DEBUG << "Database " << m_name << ": Reading all column definitions.";

    // Obtain columns
    const auto masterColumn = m_sysColumnDefsTable->getMasterColumn();
    const auto columnIdColumn =
            m_sysColumnDefsTable->findColumnChecked(kSysColumnDefs_ColumnId_ColumnName);
    const auto constraintCountColumn =
            m_sysColumnDefsTable->findColumnChecked(kSysColumnDefs_ConstraintCount_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name << ": readAllColumnDefs: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysColumnDefsTable->getName(), m_uuid, m_sysColumnDefsTable->getId(), 1);
    }
    if (maxTrid == 0) {
        m_columnDefinitionRegistry.clear();
        LOG_DEBUG << "Database " << m_name << ": There are no column definitions.";
        return;
    }

    const auto expectedColumnCount = m_sysColumnDefsTable->getColumnCount() - 1;

    const auto& columnsById = m_columnRegistry.byId();
    bool hasInvalidColumnDefs = false;
    ColumnDefinitionRegistry reg;
    IndexValue indexValue;
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        if (index->findValue(currentKey, indexValue.m_data, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysColumnDefsTable->getName(), m_uuid, m_sysColumnDefsTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysColumnDefsTable->getName(), m_uuid, m_sysColumnDefsTable->getId(),
                    mcrAddr.getBlockId(), mcrAddr.getOffset(), expectedColumnCount,
                    mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant columnIdValue, constraintCountValue;
        std::size_t colIndex = 0;
        columnIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), columnIdValue);
        constraintCountColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), constraintCountValue);
        const auto columnDefinitionId = mcr.getTableRowId();
        const auto columnId = columnIdValue.asUInt64();

        // Validate column ID
        if (columnsById.count(columnId) == 0) {
            hasInvalidColumnDefs = true;
            LOG_ERROR << "Database " << m_name << ": readAllColumnDefs: Invalid column ID "
                      << columnId << " in the column definition #" << columnDefinitionId << '.';
            continue;
        }

        // Add column definition record
        ColumnDefinitionRecord columnDefinitionRecord(columnDefinitionId, columnId);
        reg.insert(std::move(columnDefinitionRecord));
        LOG_DEBUG << "Database " << m_name << ": readAllColumnDefs: Column definition #" << trid;
    } while (index->findNextKey(currentKey, nextKey));

    if (hasInvalidColumnDefs) throw std::runtime_error("There are invalid column definitions");

    m_columnDefinitionRegistry.swap(reg);
    LOG_DEBUG << "Database " << m_name << ": Read " << m_columnDefinitionRegistry.size()
              << " column definitions.";
}

void Database::readAllColumnSetColumns()
{
    LOG_DEBUG << "Database " << m_name << ": Reading all column set columns.";

    // Obtain columns
    const auto masterColumn = m_sysColumnSetColumnsTable->getMasterColumn();
    const auto columnSetIdColumn = m_sysColumnSetColumnsTable->findColumnChecked(
            kSysColumnSetColumns_ColumnSetId_ColumnName);
    const auto columnDefinitionIdColumn = m_sysColumnSetColumnsTable->findColumnChecked(
            kSysColumnSetColumns_ColumnDefinitionId_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name
                  << ": readAllColumnSetColumns: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysColumnSetColumnsTable->getName(), m_uuid, m_sysColumnSetColumnsTable->getId(),
                1);
    }
    if (maxTrid == 0) {
        m_columnDefinitionRegistry.clear();
        LOG_DEBUG << "Database " << m_name << ": There are no column set columns.";
        return;
    }

    const auto expectedColumnCount = m_sysColumnSetColumnsTable->getColumnCount() - 1;

    const auto& columnSetsById = m_columnSetRegistry.byId();
    const auto& columnDefinitionsById = m_columnDefinitionRegistry.byId();
    auto reg = m_columnSetRegistry;
    const auto& newColumnSetsById = reg.byId();
    bool hasInvalidColumnSetColumns = false;
    IndexValue indexValue;
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        if (index->findValue(currentKey, indexValue.m_data, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysColumnSetColumnsTable->getName(), m_uuid,
                    m_sysColumnSetColumnsTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysColumnSetColumnsTable->getName(), m_uuid,
                    m_sysColumnSetColumnsTable->getId(), mcrAddr.getBlockId(), mcrAddr.getOffset(),
                    expectedColumnCount, mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant columnSetIdValue, columnDefinitionIdValue, columnIdValue;
        std::size_t colIndex = 0;
        columnSetIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), columnSetIdValue);
        columnDefinitionIdColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), columnDefinitionIdValue);
        const auto columnSetColumnId = mcr.getTableRowId();
        const auto columnSetId = columnSetIdValue.asUInt64();
        const auto columnDefinitionId = columnDefinitionIdValue.asUInt64();

        // Validate column set ID
        const auto itColumnSet = columnSetsById.find(columnSetId);
        if (itColumnSet == columnSetsById.cend()) {
            hasInvalidColumnSetColumns = true;
            LOG_ERROR << "Database " << m_name
                      << ": readAllColumnSetColumns: Invalid column set ID " << columnSetId
                      << " in the column set column record #" << columnSetColumnId << '.';
            continue;
        }

        // Validate column definition ID
        const auto itColumnDefinition = columnDefinitionsById.find(columnDefinitionId);
        if (itColumnDefinition == columnDefinitionsById.cend()) {
            hasInvalidColumnSetColumns = true;
            LOG_ERROR << "Database " << m_name
                      << ": readAllColumnSetColumns: Invalid column definition ID "
                      << columnDefinitionId << " in the column set column record #"
                      << columnSetColumnId << '.';
            continue;
        }

        // Add column set column record record
        const auto it = newColumnSetsById.find(columnSetId);
        if (it == newColumnSetsById.end()) {
            hasInvalidColumnSetColumns = true;
            LOG_ERROR << "Database " << m_name << ": readAllColumnSetColumns: missing ColumnSet #"
                      << columnSetId << " for the column set column #" << columnSetColumnId << '.';
            continue;
        }
        auto& columnSetRecord = stdext::as_mutable(*it);

        const auto& columnsByColumnDefinitionId = columnSetRecord.m_columns.byColumnDefinitionId();
        const auto itDuplicate = std::find_if(columnsByColumnDefinitionId.cbegin(),
                columnsByColumnDefinitionId.cend(), [columnDefinitionId](const auto& r) {
                    return r.m_columnDefinitionId == columnDefinitionId;
                });
        if (itDuplicate != columnsByColumnDefinitionId.cend()) {
            LOG_ERROR << "Database " << m_name
                      << ": readAllColumnSetColumns: Duplicate column definition ID "
                      << columnDefinitionId << " in the column set column record #"
                      << columnSetColumnId << '.';
            hasInvalidColumnSetColumns = true;
            continue;
        }
        ColumnSetColumnRecord columnSetColumnRecord(
                columnSetColumnId, columnSetId, columnDefinitionId, itColumnDefinition->m_columnId);
        columnSetRecord.m_columns.insert(std::move(columnSetColumnRecord));
        LOG_DEBUG << "Database " << m_name
                  << ": readAllColumnSetColumns: Column set column record #" << trid;
    } while (index->findNextKey(currentKey, nextKey));

    if (hasInvalidColumnSetColumns)
        throw std::runtime_error("There are invalid column set columns");

    const auto& columnSetsById2 = reg.byId();
    const auto totalCount = std::accumulate(columnSetsById2.cbegin(), columnSetsById2.cend(),
            std::size_t(0), [](std::size_t v, const ColumnSetRecord& r) noexcept {
                return v + r.m_columns.size();
            });

    m_columnSetRegistry.swap(reg);

    LOG_DEBUG << "Database " << m_name << ": Read " << totalCount << " column set columns.";
}

void Database::readAllConstraintDefs()
{
    LOG_DEBUG << "Database " << m_name << ": Reading all constraint definitions.";

    // Obtain columns
    const auto masterColumn = m_sysConstraintDefsTable->getMasterColumn();
    const auto typeColumn =
            m_sysConstraintDefsTable->findColumnChecked(kSysConstraintDefs_Type_ColumnName);
    const auto exprColumn =
            m_sysConstraintDefsTable->findColumnChecked(kSysConstraintDefs_Expr_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name << ": readAllConstraintDefs: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysConstraintDefsTable->getName(), m_uuid, m_sysConstraintDefsTable->getId(), 1);
    }
    if (maxTrid == 0) {
        m_constraintRegistry.clear();
        LOG_DEBUG << "Database " << m_name << ": There are no constraint definitions.";
        return;
    }

    const auto expectedColumnCount = m_sysConstraintDefsTable->getColumnCount() - 1;

    bool hasInvalidConstraintDefs = false;
    ConstraintDefinitionRegistry reg;
    IndexValue indexValue;
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        if (index->findValue(currentKey, indexValue.m_data, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysConstraintDefsTable->getName(), m_uuid, m_sysConstraintDefsTable->getId(),
                    2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysConstraintDefsTable->getName(), m_uuid,
                    m_sysConstraintDefsTable->getId(), mcrAddr.getBlockId(), mcrAddr.getOffset(),
                    expectedColumnCount, mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant typeValue, exprValue;
        std::size_t colIndex = 0;
        typeColumn->readRecord(columnRecords.at(colIndex++).getAddress(), typeValue);
        exprColumn->readRecord(columnRecords.at(colIndex++).getAddress(), exprValue);
        const auto constraintDefinitionId = mcr.getTableRowId();
        const auto constraintType = typeValue.asInt32();
        const auto expr = exprValue.isNull()
                                  ? Variant::OptionalOwnershipUniquePtr<BinaryValue>(nullptr, false)
                                  : exprValue.asBinary();

        // Validate constraint type
        if (constraintType < static_cast<int>(ConstraintType::kNotNull)
                || constraintType >= static_cast<int>(ConstraintType::kMax)) {
            hasInvalidConstraintDefs = true;
            LOG_ERROR << "Database " << m_name << ": readAllConstraintDefs: Invalid type "
                      << constraintType << " of the constraint definition #"
                      << constraintDefinitionId << '.';
            continue;
        }

        // Add constraint definition record
        ConstraintDefinitionRecord constraintDefinitionRecord(constraintDefinitionId,
                static_cast<ConstraintType>(constraintType),
                expr ? std::move(*expr) : BinaryValue());
        reg.insert(std::move(constraintDefinitionRecord));
        LOG_DEBUG << "Database " << m_name << ": readAllConstraintDefs: Constraint definition #"
                  << trid;
    } while (index->findNextKey(currentKey, nextKey));

    if (hasInvalidConstraintDefs)
        throw std::runtime_error("There are invalid coinstraint definition records");

    m_constraintDefinitionRegistry.swap(reg);
    LOG_DEBUG << "Database " << m_name << ": Read " << m_constraintDefinitionRegistry.size()
              << " constraint definitions.";
}

void Database::readAllConstraints()
{
    LOG_DEBUG << "Database " << m_name << ": Reading all constraints.";

    // Obtain columns
    const auto masterColumn = m_sysConstraintsTable->getMasterColumn();
    const auto nameColumn =
            m_sysConstraintsTable->findColumnChecked(kSysConstraints_Name_ColumnName);
    const auto stateColumn =
            m_sysConstraintsTable->findColumnChecked(kSysConstraints_State_ColumnName);
    const auto tableIdColumn =
            m_sysConstraintsTable->findColumnChecked(kSysConstraints_TableId_ColumnName);
    const auto columnIdColumn =
            m_sysConstraintsTable->findColumnChecked(kSysConstraints_ColumnId_ColumnName);
    const auto defIdColumn =
            m_sysConstraintsTable->findColumnChecked(kSysConstraints_DefinitionId_ColumnName);
    const auto descriptionColumn =
            m_sysConstraintsTable->findColumnChecked(kSysConstraints_Description_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name << ": readAllConstraints: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysConstraintsTable->getName(), m_uuid, m_sysConstraintsTable->getId(), 1);
    }
    if (maxTrid == 0) {
        m_constraintRegistry.clear();
        LOG_DEBUG << "Database " << m_name << ": There are no constraints.";
        return;
    }

    const auto expectedColumnCount = m_sysConstraintsTable->getColumnCount() - 1;

    const auto& constraintDefsById = m_constraintDefinitionRegistry.byId();
    bool hasInvalidConstraints = false;
    ConstraintRegistry reg;
    IndexValue indexValue;
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        if (index->findValue(currentKey, indexValue.m_data, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysConstraintsTable->getName(), m_uuid, m_sysConstraintsTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysConstraintsTable->getName(), m_uuid,
                    m_sysConstraintsTable->getId(), mcrAddr.getBlockId(), mcrAddr.getOffset(),
                    expectedColumnCount, mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant nameValue, stateValue, tableIdValue, columnIdValue, defIdValue, descriptionValue;
        std::size_t colIndex = 0;
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue);
        stateColumn->readRecord(columnRecords.at(colIndex++).getAddress(), stateValue);
        tableIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), tableIdValue);
        columnIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), columnIdValue);
        defIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), defIdValue);
        descriptionColumn->readRecord(columnRecords.at(colIndex++).getAddress(), descriptionValue);
        const auto constraintId = mcr.getTableRowId();
        const auto name = nameValue.asString();
        const auto constraintState = stateValue.asInt32();
        const auto tableId = tableIdValue.asUInt32();
        const auto columnId = columnIdValue.asUInt64();
        const auto constraintDefinitionId = defIdValue.asUInt64();

        // Validate constraint type
        if (constraintState < static_cast<int>(ConstraintState::kCreating)
                || constraintState >= static_cast<int>(ConstraintState::kMax)) {
            hasInvalidConstraints = true;
            LOG_ERROR << "Database " << m_name << ": readAllConstraints: Invalid state "
                      << constraintState << " of the constraint #" << constraintId << '.';
            continue;
        }

        // Validate name
        if (!isValidDatabaseObjectName(*name)) {
            hasInvalidConstraints = true;
            LOG_ERROR << "Database " << m_name << ": readAllConstraints: Invalid name '" << *name
                      << "' of the constraint #" << constraintId << '.';
            continue;
        }

        // Validate constraint definition ID
        if (constraintDefsById.count(constraintDefinitionId) == 0) {
            hasInvalidConstraints = true;
            LOG_ERROR << "Database " << m_name
                      << ": readAllConstraints: Invalid constraint definition ID "
                      << constraintDefinitionId << " in the constraint #" << constraintId << '.';
            continue;
        }

        // Add constraint record
        ConstraintRecord constraintRecord(constraintId, std::move(*name),
                static_cast<ConstraintState>(constraintState), tableId, columnId,
                constraintDefinitionId, descriptionValue.asOptionalString());
        reg.insert(std::move(constraintRecord));
        LOG_DEBUG << "Database " << m_name << ": readAllConstraints: Constraint #" << trid << " '"
                  << constraintRecord.m_name << '\'';
    } while (index->findNextKey(currentKey, nextKey));

    if (hasInvalidConstraints) throw std::runtime_error("There are invalid constraint records");

    m_constraintRegistry.swap(reg);
    LOG_DEBUG << "Database " << m_name << ": Read " << m_constraintRegistry.size()
              << " constraints.";
}

void Database::readAllColumnDefConstraints()
{
    LOG_DEBUG << "Database " << m_name << ": Reading all column definition constraints.";

    // Obtain columns
    const auto masterColumn = m_sysColumnDefConstraintsTable->getMasterColumn();
    const auto columnDefinitionIdColumn = m_sysColumnDefConstraintsTable->findColumnChecked(
            kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnName);
    const auto constraintIdColumn = m_sysColumnDefConstraintsTable->findColumnChecked(
            kSysColumnDefinitionConstraintList_ConstraintId_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name
                  << ": readAllColumnDefConstraints: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysColumnDefConstraintsTable->getName(), m_uuid,
                m_sysColumnDefConstraintsTable->getId(), 1);
    }
    if (maxTrid == 0) {
        m_columnDefinitionRegistry.clear();
        LOG_DEBUG << "Database " << m_name << ": There are no column definition constraints.";
        return;
    }

    const auto expectedColumnCount = m_sysColumnDefConstraintsTable->getColumnCount() - 1;

    const auto& columnDefsById = m_columnDefinitionRegistry.byId();
    const auto& constraintsById = m_constraintRegistry.byId();
    ColumnDefinitionRegistry reg = m_columnDefinitionRegistry;
    const auto& newColumnDefsById = reg.byId();
    bool hasInvalidColumnDefinitionConstraintList = false;
    IndexValue indexValue;
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        if (index->findValue(currentKey, indexValue.m_data, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysColumnDefConstraintsTable->getName(), m_uuid,
                    m_sysColumnDefConstraintsTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysColumnDefConstraintsTable->getName(), m_uuid,
                    m_sysColumnDefConstraintsTable->getId(), mcrAddr.getBlockId(),
                    mcrAddr.getOffset(), expectedColumnCount, mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant columnDefinitionIdValue, constraintIdValue;
        std::size_t colIndex = 0;
        columnDefinitionIdColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), columnDefinitionIdValue);
        constraintIdColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), constraintIdValue);
        const auto columnDefinitionConstraintId = mcr.getTableRowId();
        const auto columnDefinitionId = columnDefinitionIdValue.asUInt64();
        const auto constraintId = constraintIdValue.asUInt64();

        // Validate column definition ID
        if (columnDefsById.count(columnDefinitionId) == 0) {
            hasInvalidColumnDefinitionConstraintList = true;
            LOG_ERROR << "Database " << m_name
                      << ": readAllColumnDefConstraints: Invalid column definition ID "
                      << columnDefinitionId << " in the column definition constraint record #"
                      << columnDefinitionConstraintId << '.';
            continue;
        }

        // Validate constraint ID
        if (constraintsById.count(constraintId) == 0) {
            hasInvalidColumnDefinitionConstraintList = true;
            LOG_ERROR << "Database " << m_name
                      << ": readAllColumnDefConstraints: Invalid column definition ID "
                      << constraintId << " in the column definition constraint record #"
                      << columnDefinitionConstraintId << '.';
            continue;
        }

        // Add column set column record record
        const auto it = newColumnDefsById.find(columnDefinitionId);
        if (it == newColumnDefsById.end()) abort();  // Should never happen
        auto& columnDefinitionRecord = stdext::as_mutable(*it);
        const auto& columnDefinitionConstraintsById = columnDefinitionRecord.m_constraints.byId();
        const auto itDuplicate = std::find_if(columnDefinitionConstraintsById.cbegin(),
                columnDefinitionConstraintsById.cend(),
                [constraintId](const auto& r) { return r.m_constraintId == constraintId; });
        if (itDuplicate != columnDefinitionConstraintsById.cend()) {
            LOG_ERROR << "Database " << m_name
                      << ": readAllColumnDefConstraints: Duplicate constraint ID " << constraintId
                      << " in the column definition constraiont record #"
                      << columnDefinitionConstraintId << '.';
            hasInvalidColumnDefinitionConstraintList = true;
            continue;
        }
        columnDefinitionRecord.m_constraints.emplace(
                columnDefinitionConstraintId, columnDefinitionId, constraintId);
        LOG_DEBUG << "Database " << m_name
                  << ": readAllColumnDefConstraints: Column definition constraint record #" << trid;
    } while (index->findNextKey(currentKey, nextKey));

    if (hasInvalidColumnDefinitionConstraintList)
        throw std::runtime_error("There are invalid column definition constraints");

    m_columnDefinitionRegistry.swap(reg);
    const auto& columnDefinitionsById = m_columnDefinitionRegistry.byId();
    const auto totalCount =
            std::accumulate(columnDefinitionsById.cbegin(), columnDefinitionsById.cend(),
                    std::size_t(0), [](std::size_t v, const ColumnDefinitionRecord& r) noexcept {
                        return v + r.m_constraints.size();
                    });

    LOG_DEBUG << "Database " << m_name << ": Read " << totalCount
              << " column definition constraints.";
}

void Database::readAllIndices()
{
    LOG_DEBUG << "Database " << m_name << ": Reading all indices.";

    // Obtain columns of the SYS_INDICES table
    const auto sysIndicesMasterColumn = m_sysIndicesTable->getMasterColumn();
    const auto sysIndicesTypeColumn =
            m_sysIndicesTable->findColumnChecked(kSysIndices_Type_ColumnName);
    const auto sysIndicesIsUniqueColumn =
            m_sysIndicesTable->findColumnChecked(kSysIndices_Unique_ColumnName);
    const auto sysIndicesNameColumn =
            m_sysIndicesTable->findColumnChecked(kSysIndices_Name_ColumnName);
    const auto sysIndicesTableIdColumn =
            m_sysIndicesTable->findColumnChecked(kSysIndices_TableId_ColumnName);
    const auto sysIndicesDataFileSizeColumn =
            m_sysIndicesTable->findColumnChecked(kSysIndices_DataFileSize_ColumnName);
    const auto descriptionColumn =
            m_sysIndicesTable->findColumnChecked(kSysIndices_Description_ColumnName);

    // Obtain columns of the SYS_INDEX_COLUMNS table
    const auto sysIndexColumnsMasterColumn = m_sysIndexColumnsTable->getMasterColumn();
    const auto sysIndexColumnsIndexIdColumn =
            m_sysIndexColumnsTable->findColumnChecked(kSysIndexColumns_IndexId_ColumnName);
    const auto sysIndexColumnsColumnDefinitionIdColumn = m_sysIndexColumnsTable->findColumnChecked(
            kSysIndexColumns_ColumnDefinitionId_ColumnName);
    const auto sysIndexColumnsSortDescColumn =
            m_sysIndexColumnsTable->findColumnChecked(kSysIndexColumns_SortDesc_ColumnName);

    // Obtain min and max TRID
    const auto sysIndexColumnsIndex = sysIndexColumnsMasterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    auto p = std::make_pair(
            sysIndexColumnsIndex->getMinKey(key), sysIndexColumnsIndex->getMaxKey(key + 8));
    if (p.first && p.second) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name
                  << ": readAllIndices: sysIndexColumnsIndex: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    } else {
        LOG_WARNING << "Database " << m_name
                    << ": readAllIndices: sysIndexColumnsIndex: No records. Additional info: "
                    << p.first << " " << p.second;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysIndexColumnsTable->getName(), m_uuid, m_sysIndexColumnsTable->getId(), 1);
    }

    struct IndexInfo {
        std::vector<IndexColumnRecord> m_columns;
        std::unordered_map<std::uint64_t, std::size_t> m_columnDefinitionIds;
    };

    std::unordered_map<std::uint64_t, IndexInfo> indexInfos;

    if (maxTrid > 0) {
        const auto expectedColumnCount = m_sysIndexColumnsTable->getColumnCount() - 1;
        IndexValue indexValue;
        std::uint8_t* currentKey = key + 8;
        std::uint8_t* nextKey = key;
        do {
            // Obtain master column record address
            std::swap(currentKey, nextKey);
            std::uint64_t trid = 0;
            ::pbeDecodeUInt64(currentKey, &trid);
            //DBG_LOG_DEBUG("Database " << m_name
            //                          << ": readAllIndices: Looking up SYS_INDEX_COLUMNS.TRID "
            //                          << trid);
            if (sysIndexColumnsIndex->findValue(currentKey, indexValue.m_data, 1) != 1) {
                throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted,
                        m_name, m_sysIndexColumnsTable->getName(), m_uuid,
                        m_sysIndexColumnsTable->getId(), 2);
            }
            ColumnDataAddress mcrAddr;
            mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

            // Read and validate master column record
            MasterColumnRecord mcr;
            sysIndexColumnsMasterColumn->readMasterColumnRecord(mcrAddr, mcr);
            if (mcr.getColumnCount() != expectedColumnCount) {
                throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                        m_name, m_sysIndexColumnsTable->getName(), m_uuid,
                        m_sysIndexColumnsTable->getId(), mcrAddr.getBlockId(), mcrAddr.getOffset(),
                        expectedColumnCount, mcr.getColumnCount());
            }

            // Read data from columns
            const auto& columnRecords = mcr.getColumnRecords();
            Variant indexIdValue, columnDefinitionIdValue, sortDescendingValue;
            std::size_t colIndex = 0;
            sysIndexColumnsIndexIdColumn->readRecord(
                    columnRecords.at(colIndex++).getAddress(), indexIdValue);
            sysIndexColumnsColumnDefinitionIdColumn->readRecord(
                    columnRecords.at(colIndex++).getAddress(), columnDefinitionIdValue);
            sysIndexColumnsSortDescColumn->readRecord(
                    columnRecords.at(colIndex++).getAddress(), sortDescendingValue);

            // Save into map
            const auto indexId = indexIdValue.asUInt64();
            const auto columnDefinitionId = columnDefinitionIdValue.asUInt64();
            const auto sortDescending = sortDescendingValue.asBool();
            auto& indexInfo = indexInfos[indexId];
            indexInfo.m_columns.emplace_back(
                    mcr.getTableRowId(), indexId, columnDefinitionId, sortDescending);
            ++indexInfo.m_columnDefinitionIds[columnDefinitionId];
            LOG_DEBUG << "Database " << m_name << ": readAllIndices: Index column #" << trid;
        } while (sysIndexColumnsIndex->findNextKey(currentKey, nextKey));
    }

    const auto sysIndicesIndex = sysIndicesMasterColumn->getMasterColumnMainIndex();
    if (sysIndicesIndex->getMinKey(key) && sysIndicesIndex->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "Database " << m_name
                  << ": readAllIndices: sysIndicesIndex: Decoded MinTRID=" << minTrid
                  << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysIndicesTable->getName(), m_uuid, m_sysIndicesTable->getId(), 1);
    }

    IndexRegistry reg;
    bool containsUnrelatedReferences = false;
    bool hasInvalidIndices = false;

    if (maxTrid == 0) {
        containsUnrelatedReferences = !indexInfos.empty();
    } else {
        const auto& tablesById = m_tableRegistry.byId();
        const auto& columnsById = m_columnRegistry.byId();
        const auto& columnDefinitionsById = m_columnDefinitionRegistry.byId();
        const auto expectedColumnCount = m_sysIndicesTable->getColumnCount() - 1;
        std::uint32_t indexWithColumnsCount = 0;
        IndexValue indexValue;
        std::uint8_t* currentKey = key + 8;
        std::uint8_t* nextKey = key;
        do {
            // Obtain master column record address
            std::swap(currentKey, nextKey);
            std::uint64_t trid = 0;
            ::pbeDecodeUInt64(currentKey, &trid);
            //DBG_LOG_DEBUG("Database " << m_name << ": readAllIndices: Looking up SYS_INDICES.TRID "
            //                          << trid);
            if (sysIndicesIndex->findValue(currentKey, indexValue.m_data, 1) != 1) {
                throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted,
                        m_name, m_sysIndicesTable->getName(), m_uuid, m_sysIndicesTable->getId(),
                        2);
            }
            ColumnDataAddress mcrAddr;
            mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

            // Read and validate master column record
            MasterColumnRecord mcr;
            sysIndicesMasterColumn->readMasterColumnRecord(mcrAddr, mcr);
            if (mcr.getColumnCount() != expectedColumnCount) {
                throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                        m_name, m_sysIndicesTable->getName(), m_uuid, m_sysIndicesTable->getId(),
                        mcrAddr.getBlockId(), mcrAddr.getOffset(), expectedColumnCount,
                        mcr.getColumnCount());
            }

            // Read data from columns
            const auto& columnRecords = mcr.getColumnRecords();
            Variant typeValue, uniqueValue, nameValue, tableIdValue, dataFileSizeValue,
                    descriptionValue;
            std::size_t colIndex = 0;
            sysIndicesTypeColumn->readRecord(columnRecords.at(colIndex++).getAddress(), typeValue);
            sysIndicesIsUniqueColumn->readRecord(
                    columnRecords.at(colIndex++).getAddress(), uniqueValue);
            sysIndicesNameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue);
            sysIndicesTableIdColumn->readRecord(
                    columnRecords.at(colIndex++).getAddress(), tableIdValue);
            sysIndicesDataFileSizeColumn->readRecord(
                    columnRecords.at(colIndex++).getAddress(), dataFileSizeValue);
            descriptionColumn->readRecord(
                    columnRecords.at(colIndex++).getAddress(), descriptionValue);

            const auto indexId = mcr.getTableRowId();
            const auto tableId = tableIdValue.asUInt32();
            auto indexName = *nameValue.asString();

            // Check index name
            if (!isValidDatabaseObjectName(indexName)) {
                hasInvalidIndices = true;
                LOG_ERROR << "Database " << m_name
                          << ": readAllIndices: Invalid name of the index #" << indexId << '('
                          << nameValue << ").";
                continue;
            }

            // Check that there are some columns for this index
            const auto itIndexInfo = indexInfos.find(indexId);
            if (itIndexInfo == indexInfos.end()) {
                hasInvalidIndices = true;
                LOG_ERROR << "Database " << m_name << ": readAllIndices: No columns for the index #"
                          << indexId << '(' << nameValue << ").";
                continue;
            }
            ++indexWithColumnsCount;
            const auto& indexInfo = itIndexInfo->second;

            // Check that table which index is intended for is existent
            const auto itTable = tablesById.find(tableId);
            if (itTable == tablesById.end()) {
                hasInvalidIndices = true;
                LOG_ERROR << "Database " << m_name << ": readAllIndices: Index #" << indexId << '('
                          << nameValue << ") refers to the non-existent table #" << tableId << '.';
                continue;
            }

            const std::string nonExistent("(non-existent)");

            // Check that all columns are unique
            if (indexInfo.m_columnDefinitionIds.size() != indexInfo.m_columns.size()) {
                hasInvalidIndices = true;
                LOG_ERROR << "Database " << m_name
                          << ": readAllIndices: Non-unique columns in the index #" << indexId << '('
                          << nameValue << ").";
                for (const auto& e : indexInfo.m_columnDefinitionIds) {
                    LOG_ERROR << "Database " << m_name
                              << ": readAllIndices: ... column definition #" << e.first
                              << " happens " << e.second << " times";
                }
                continue;
            }

            // Check that all columns are really exist and belong to the same table
            // for which the index is intended for.
            std::size_t nonExistentColumnCount = 0, columnsFromOtherTablesCount = 0;
            for (const auto& e : indexInfo.m_columnDefinitionIds) {
                const auto itColumnDef = columnDefinitionsById.find(e.first);
                if (itColumnDef == columnDefinitionsById.end()) {
                    // No such column
                    hasInvalidIndices = true;
                    ++nonExistentColumnCount;
                    LOG_ERROR << "Database " << m_name << ": readAllIndices: Index #" << indexId
                              << '(' << nameValue
                              << ") refers to the non-existent column definition #" << e.first
                              << '.';
                    continue;
                }
                const auto itColumn = columnsById.find(itColumnDef->m_columnId);
                if (itColumn == columnsById.end()) {
                    hasInvalidIndices = true;
                    ++nonExistentColumnCount;
                    LOG_ERROR << "Database " << m_name << ": readAllIndices: Index #" << indexId
                              << '(' << nameValue << ") refers to the non-existent column #"
                              << itColumnDef->m_columnId << " throgh column definition #" << e.first
                              << '.';
                }
                if (itColumn->m_tableId != tableId) {
                    // Column belongs to a different table than index is intended for
                    hasInvalidIndices = true;
                    ++nonExistentColumnCount;
                    const auto itOtherTable = tablesById.find(itColumn->m_tableId);
                    const auto& otherTableName =
                            (itOtherTable == tablesById.end()) ? nonExistent : itOtherTable->m_name;
                    LOG_ERROR << "Database " << m_name << ": readAllIndices: Index #" << indexId
                              << '(' << nameValue << ") refers to the column #" << e.first << " ("
                              << itColumn->m_name << ") which belongs to table #"
                              << itColumn->m_tableId << " (" << otherTableName
                              << ") while index is for the table #" << tableId << " ("
                              << itTable->m_name << ").";
                    continue;
                }
            }
            if (nonExistentColumnCount > 0) {
                LOG_ERROR << "Database " << m_name << ": readAllIndices: Index #" << indexId << '('
                          << nameValue << ") refers to the one or more non-existent columns.";
            }
            if (columnsFromOtherTablesCount > 0) {
                LOG_ERROR << "Database " << m_name << ": readAllIndices: Index #" << indexId << '('
                          << nameValue << ") refers to the one or more columns from other tables.";
            }
            if (nonExistentColumnCount > 0 || columnsFromOtherTablesCount > 0) {
                LOG_ERROR << "Database " << m_name << ": readAllIndices: Index #" << indexId << '('
                          << nameValue << " was ignored due to above reasons.";
                continue;
            }

            // Add index record

            const auto indexType = static_cast<IndexType>(typeValue.asInt32());
            const auto unique = uniqueValue.asBool();
            const auto dataFileSize = dataFileSizeValue.asUInt32();

            IndexColumnRegistry indexColumns;
            for (const auto& column : indexInfos[indexId].m_columns)
                indexColumns.emplace(column);

            IndexRecord indexRecord(indexId, indexType, tableId, unique, std::move(indexName),
                    std::move(indexColumns), dataFileSize, descriptionValue.asOptionalString());
            LOG_DEBUG << "Database " << m_name << ": readAllIndices: Index #" << trid << " '"
                      << indexRecord.m_name << '\'';
            reg.insert(std::move(indexRecord));
        } while (sysIndicesIndex->findNextKey(currentKey, nextKey));
    }

    if (containsUnrelatedReferences) {
        LOG_WARNING << "Database '" << m_name
                    << "' readAllIndices: There are references to index columns not related to any "
                       "index.";
    }

    if (hasInvalidIndices) {
        throw std::runtime_error(stdext::string_builder()
                                 << "Database " << m_uuid
                                 << " readAllIndices: There are invalid indices");
    }

    m_indexRegistry.swap(reg);
    if (m_indexRegistry.empty()) {
        LOG_DEBUG << "Database " << m_name << ": There are no indices.";
    } else {
        LOG_DEBUG << "Database " << m_name << ": Read " << m_indexRegistry.size() << " indices.";
    }
}

}  // namespace siodb::iomgr::dbengine
