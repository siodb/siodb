// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Database.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Column.h"
#include "Index.h"
#include "Table.h"
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

// STL headers
#include <numeric>

namespace siodb::iomgr::dbengine {

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
        if (index->find(currentKey, indexValue.m_data, 1) != 1) {
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
                    << p.first << ' ' << p.second;
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
            if (sysIndexColumnsIndex->find(currentKey, indexValue.m_data, 1) != 1) {
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
            if (sysIndicesIndex->find(currentKey, indexValue.m_data, 1) != 1) {
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
