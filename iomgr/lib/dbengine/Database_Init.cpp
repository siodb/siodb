// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Database.h"

// Project headers
#include "ColumnDataBlock.h"
#include "ColumnDefinitionConstraintPtr.h"
#include "ColumnSetColumn.h"
#include "ConstraintDefinition.h"
#include "Table.h"
#include "parser/expr/ConstantExpression.h"

// Common project headers
#include <siodb/common/utils/FsUtils.h>

// STL headers
#include <numeric>

namespace siodb::iomgr::dbengine {

const Uuid Database::kSystemDatabaseUuid {{0x68, 0xba, 0x3, 0x8e, 0xb7, 0x4, 0x2c, 0xb9, 0x1d, 0xd,
        0xb9, 0x18, 0x64, 0xc8, 0x19, 0xcd}};

const std::unordered_map<std::string, std::unordered_set<std::string>> Database::m_allSystemTables {
        {kSysUsersTable,
                {
                        kMasterColumnName,
                        kSysUsers_Name_Column,
                        kSysUsers_RealName_Column,
                        kSysUsers_State_Column,
                }},
        {kSysUserAccessKeysTable,
                {
                        kMasterColumnName,
                        kSysUserAccessKeys_UserId_Column,
                        kSysUserAccessKeys_Name_Column,
                        kSysUserAccessKeys_State_Column,
                        kSysUserAccessKeys_Text_Column,
                }},
        {kSysDatabasesTable,
                {
                        kMasterColumnName,
                        kSysDatabases_Uuid_Column,
                        kSysDatabases_Name_Column,
                        kSysDatabases_CipherId_Column,
                        kSysDatabases_CipherKey_Column,

                }},
        {kSysUserPermissionsTable,
                {
                        kMasterColumnName,
                        kSysUserPermissions_UserId_Column,
                        kSysUserPermissions_DatabaseId_Column,
                        kSysUserPermissions_ObjectType_Column,
                        kSysUserPermissions_ObjectId_Column,
                        kSysUserPermissions_Permissions_Column,
                        kSysUserPermissions_GrantOptions_Column,
                }},
        {kSysTablesTable,
                {
                        kMasterColumnName,
                        kSysTables_Type_Column,
                        kSysTables_Name_Column,
                        kSysTables_FirstUserTrid_Column,
                        kSysTables_CurrentColumnSetId_Column,
                }},
        {kSysDummyTable,
                {
                        kMasterColumnName,
                        kSysDummy_Dummy_Column,
                }},
        {kSysColumnSetsTable,
                {
                        kMasterColumnName,
                        kSysColumnSets_TableId_Column,
                        kSysColumnSets_ColumnCount_Column,
                }},
        {kSysColumnsTable,
                {
                        kMasterColumnName,
                        kSysColumns_TableId_Column,
                        kSysColumns_DataType_Column,
                        kSysColumns_Name_Column,
                        kSysColumns_State_Column,
                }},
        {kSysColumnDefsTable,
                {
                        kMasterColumnName,
                        kSysColumnDefs_ColumnId_Column,
                        kSysColumnDefs_ConstraintCount_Column,
                }},
        {kSysColumnSetColumnsTable,
                {
                        kMasterColumnName,
                        kSysColumnSetColumns_ColumnSetId_Column,
                        kSysColumnSetColumns_ColumnDefinitionId_Column,
                }},
        {kSysConstraintDefsTable,
                {
                        kMasterColumnName,
                        kSysConstraintDefs_Type_Column,
                        kSysConstraintDefs_Expr_Column,
                }},
        {kSysConstraintsTable,
                {
                        kMasterColumnName,
                        kSysConstraints_State_Column,
                        kSysConstraints_Name_Column,
                        kSysConstraints_TableId_Column,
                        kSysConstraints_ColumnId_Column,
                        kSysConstraints_DefinitionId_Column,
                }},
        {kSysColumnDefConstraintsTable,
                {
                        kMasterColumnName,
                        kSysColumnDefinitionConstraintList_ColumnDefinitionId_Column,
                        kSysColumnDefinitionConstraintList_ConstraintId_Column,
                }},
        {kSysIndicesTable,
                {
                        kMasterColumnName,
                        kSysIndices_Type_Column,
                        kSysIndices_Unique_Column,
                        kSysIndices_Name_Column,
                        kSysIndices_TableId_Column,
                }},
        {kSysIndexColumnsTable,
                {
                        kMasterColumnName,
                        kSysIndexColumns_IndexId_Column,
                        kSysIndexColumns_ColumnDefinitionId_Column,
                        kSysIndexColumns_SortDesc_Column,
                }},
};

const std::unordered_set<std::string> Database::m_systemDatabaseOnlySystemTables {
        kSysDatabasesTable,
        kSysUsersTable,
        kSysUserAccessKeysTable,
        kSysUserPermissionsTable,
};

// New database
Database::Database(Instance& instance, const std::string& name, const std::string& cipherId,
        const BinaryValue& cipherKey, std::size_t tableCacheCapacity)
    : m_instance(instance)
    , m_uuid(computeDatabaseUuid(
              name, name == kSystemDatabaseName ? kSystemDatabaseCreationTime : std::time(nullptr)))
    , m_name(validateDatabaseName(name))
    , m_id(instance.generateNextDatabaseId(name == kSystemDatabaseName))
    , m_dataDir(ensureDataDir(true))
    , m_cipher(crypto::getCipher(cipherId))
    , m_cipherKey(cipherKey)
    , m_encryptionContext(m_cipher ? m_cipher->createEncryptionContext(m_cipherKey) : nullptr)
    , m_decryptionContext(m_cipher ? m_cipher->createDecryptionContext(m_cipherKey) : nullptr)
    , m_metadataFile(createMetadataFile())
    , m_metadata(static_cast<DatabaseMetadata*>(m_metadataFile->getMappingAddress()))
    , m_createTransactionParams(User::kSuperUserId, generateNextTransactionId())
    , m_tableCache(m_name,
              tableCacheCapacity > 0 ? tableCacheCapacity : instance.getTableCacheCapacity())
    , m_constraintDefinitionCache(*this, kConstraintDefinitionCacheCapacity)
    , m_useCount(0)
    , m_systemNotNullConstraintDefinition(createSystemConstraintDefinitionUnlocked(
              ConstraintType::kNotNull, std::make_unique<requests::ConstantExpression>(true)))
    , m_systenDefaultZeroConstraintDefinition(createSystemConstraintDefinitionUnlocked(
              ConstraintType::kDefaultValue, std::make_unique<requests::ConstantExpression>(0)))
{
    createSystemTables();
}

// Init existing database
Database::Database(
        Instance& instance, const DatabaseRecord& dbRecord, std::size_t tableCacheCapacity)
    : m_instance(instance)
    , m_uuid(dbRecord.m_uuid)
    , m_name(dbRecord.m_name)
    , m_id(dbRecord.m_id)
    , m_dataDir(ensureDataDir(false))
    , m_cipher(crypto::getCipher(dbRecord.m_cipherId))
    , m_cipherKey(dbRecord.m_cipherKey)
    , m_encryptionContext(m_cipher ? m_cipher->createEncryptionContext(m_cipherKey) : nullptr)
    , m_decryptionContext(m_cipher ? m_cipher->createDecryptionContext(m_cipherKey) : nullptr)
    , m_metadataFile(openMetadataFile())
    , m_metadata(static_cast<DatabaseMetadata*>(m_metadataFile->getMappingAddress()))
    , m_tableCache(m_name,
              tableCacheCapacity > 0 ? tableCacheCapacity : instance.getTableCacheCapacity())
    , m_constraintDefinitionCache(*this, kConstraintDefinitionCacheCapacity)
    , m_useCount(0)
    , m_sysTablesTable(loadSystemTable(kSysTablesTable))
    , m_sysDummyTable(loadSystemTable(kSysDummyTable))
    , m_sysColumnSetsTable(loadSystemTable(kSysColumnSetsTable))
    , m_sysColumnsTable(loadSystemTable(kSysColumnsTable))
    , m_sysColumnDefsTable(loadSystemTable(kSysColumnDefsTable))
    , m_sysColumnSetColumnsTable(loadSystemTable(kSysColumnSetColumnsTable))
    , m_sysConstraintDefsTable(loadSystemTable(kSysConstraintDefsTable))
    , m_sysConstraintsTable(loadSystemTable(kSysConstraintsTable))
    , m_sysColumnDefConstraintsTable(loadSystemTable(kSysColumnDefConstraintsTable))
    , m_sysIndicesTable(loadSystemTable(kSysIndicesTable))
    , m_sysIndexColumnsTable(loadSystemTable(kSysIndexColumnsTable))
    , m_systemNotNullConstraintDefinition(createSystemConstraintDefinitionUnlocked(
              ConstraintType::kNotNull, std::make_unique<requests::ConstantExpression>(true)))
    , m_systenDefaultZeroConstraintDefinition(createSystemConstraintDefinitionUnlocked(
              ConstraintType::kDefaultValue, std::make_unique<requests::ConstantExpression>(0)))
{
    readAllTables();
    readAllColumnSets();
    readAllColumns();
    readAllColumnDefs();
    readAllColumnSetColumns();
    readAllConstraintDefs();
    readAllConstraints();
    readAllColumnDefConstraints();
    readAllIndices();
    checkDataConsistency();
}

void Database::createSystemTables()
{
    // Initialize buffers
    std::vector<TablePtr> allTables;
    allTables.reserve(m_allSystemTables.size() - m_systemDatabaseOnlySystemTables.size());

    std::vector<ColumnSetPtr> allColumnSets;
    allColumnSets.reserve(allTables.capacity());

    std::vector<ColumnPtr> allColumns, masterColumns;
    masterColumns.reserve(allTables.capacity());
    allColumns.reserve(std::accumulate(
            m_allSystemTables.cbegin(), m_allSystemTables.cend(), static_cast<std::size_t>(0),
            [](std::size_t currentSize, const auto& e) noexcept {
                const auto it = m_systemDatabaseOnlySystemTables.find(e.first);
                return (it == m_systemDatabaseOnlySystemTables.cend())
                               ? currentSize
                               : currentSize + e.second.size();
            }));

    std::vector<ColumnDefinitionPtr> allColumnDefinitions;
    allColumnDefinitions.reserve(allColumns.capacity());

    std::vector<ConstraintDefinitionPtr> allConstraintDefinitions;
    allConstraintDefinitions.reserve(16);

    std::vector<ConstraintPtr> allConstraints;
    allConstraints.reserve(allColumnDefinitions.capacity() * 2);

    std::vector<ColumnDefinitionConstraintPtr> allColumnDefinitionConstraintList;
    allColumnDefinitionConstraintList.reserve(allConstraints.capacity());

    // Collect existing so far constraint definitions
    allConstraintDefinitions.push_back(m_systemNotNullConstraintDefinition);
    allConstraintDefinitions.push_back(m_systenDefaultZeroConstraintDefinition);

    // Create all tables.

    // Create table SYS_TABLES
    m_sysTablesTable = createTableUnlocked(kSysTablesTable, TableType::kDisk, kFirstUserTableId);
    m_sysTablesTable->generateNextSystemTrid();
    allTables.push_back(m_sysTablesTable);

    // Create table SYS_DUMMY
    m_sysDummyTable = createTableUnlocked(kSysDummyTable, TableType::kDisk, 0);
    allTables.push_back(m_sysDummyTable);

    // Create table SYS_COLUMN_SETS
    m_sysColumnSetsTable =
            createTableUnlocked(kSysColumnSetsTable, TableType::kDisk, kFirstUserTableColumnSetId);
    allTables.push_back(m_sysColumnSetsTable);
    m_sysColumnSetsTable->setLastSystemTrid(m_tmpTridCounters.m_lastColumnSetId);

    // Create table SYS_COLUMNS
    m_sysColumnsTable =
            createTableUnlocked(kSysColumnsTable, TableType::kDisk, kFirstUserTableColumnId);
    allTables.push_back(m_sysColumnsTable);
    m_sysColumnsTable->setLastSystemTrid(m_tmpTridCounters.m_lastColumnId);

    // Create table SYS_COLUMN_DEFS
    m_sysColumnDefsTable = createTableUnlocked(
            kSysColumnDefsTable, TableType::kDisk, kFirstUserTableColumnDefinitionId);
    allTables.push_back(m_sysColumnDefsTable);
    m_sysColumnDefsTable->setLastSystemTrid(m_tmpTridCounters.m_lastColumnDefinitionId);

    // Create table SYS_COLUMN_SET_COLUMNS
    m_sysColumnSetColumnsTable = createTableUnlocked(
            kSysColumnSetColumnsTable, TableType::kDisk, kFirstUserTableColumnSetColumnId);
    allTables.push_back(m_sysColumnSetColumnsTable);

    // Create table SYS_CONSTRAINT_DEFS
    m_sysConstraintDefsTable = createTableUnlocked(
            kSysConstraintDefsTable, TableType::kDisk, kFirstUserTableConstraintDefinitionId);
    allTables.push_back(m_sysConstraintDefsTable);
    m_sysConstraintDefsTable->setLastSystemTrid(m_tmpTridCounters.m_lastConstraintDefinitionId);

    // Create table SYS_CONSTRAINTS
    m_sysConstraintsTable = createTableUnlocked(
            kSysConstraintsTable, TableType::kDisk, kFirstUserTableConstraintId);
    allTables.push_back(m_sysConstraintsTable);
    m_sysConstraintsTable->setLastSystemTrid(m_tmpTridCounters.m_lastConstraintId);

    // Create table SYS_COLUMN_DEF_CONSTRAINTS
    m_sysColumnDefConstraintsTable = createTableUnlocked(kSysColumnDefConstraintsTable,
            TableType::kDisk, kFirstUserTableColumnDefinitionConstraintId);
    allTables.push_back(m_sysColumnDefConstraintsTable);
    m_sysConstraintsTable->setLastSystemTrid(m_tmpTridCounters.m_lastColumnDefinitionConstraintId);

    // Create table SYS_INDICES
    m_sysIndicesTable =
            createTableUnlocked(kSysIndicesTable, TableType::kDisk, kFirstUserTableIndexId);
    allTables.push_back(m_sysIndicesTable);
    m_sysIndicesTable->setLastSystemTrid(m_tmpTridCounters.m_lastIndexId);

    // Create table SYS_INDEX_COLUMNS
    m_sysIndexColumnsTable = createTableUnlocked(
            kSysIndexColumnsTable, TableType::kDisk, kFirstUserTableIndexColumnId);
    allTables.push_back(m_sysIndexColumnsTable);
    m_sysIndexColumnsTable->setLastSystemTrid(m_tmpTridCounters.m_lastIndexColumnId);

    // Empty constraint set
    const ColumnConstraintSpecificationList noConstraintsSpec;

    // Column constraint specification list with single "NOT NULL" constraint
    // and empty name, which will cause automatic constraint name generation.
    const ColumnConstraintSpecificationList notNullConstraintSpec {
            ColumnConstraintSpecification(std::string(), ConstraintType::kNotNull,
                    requests::ExpressionPtr(
                            m_systemNotNullConstraintDefinition->getExpression().clone())),
    };

    // Create columns of the table SYS_TABLES
    auto column = m_sysTablesTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysTablesTable->createColumn(ColumnSpecification(kSysTables_Type_Column,
            COLUMN_DATA_TYPE_INT8, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysTablesTable->createColumn(ColumnSpecification(kSysTables_Name_Column,
            COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysTablesTable->createColumn(ColumnSpecification(kSysTables_FirstUserTrid_Column,
            COLUMN_DATA_TYPE_UINT64, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysTablesTable->createColumn(
            ColumnSpecification(kSysTables_CurrentColumnSetId_Column, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    // Create columns of the table SYS_DUMMY
    column = m_sysDummyTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysDummyTable->createColumn(ColumnSpecification(kSysDummy_Dummy_Column,
            COLUMN_DATA_TYPE_INT32, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));

    // Create columns of the table SYS_COLUMN_SETS
    column = m_sysColumnSetsTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysColumnSetsTable->createColumn(
            ColumnSpecification(kSysColumnSets_TableId_Column, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysColumnSetsTable->createColumn(
            ColumnSpecification(kSysColumnSets_ColumnCount_Column, COLUMN_DATA_TYPE_UINT32,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    // Create columns of the table SYS_COLUMNS
    column = m_sysColumnsTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysColumnsTable->createColumn(
            ColumnSpecification(kSysColumns_TableId_Column, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);
    column = m_sysColumnsTable->createColumn(ColumnSpecification(kSysColumns_DataType_Column,
            COLUMN_DATA_TYPE_INT8, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysColumnsTable->createColumn(ColumnSpecification(kSysColumns_Name_Column,
            COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysColumnsTable->createColumn(ColumnSpecification(kSysColumns_State_Column,
            COLUMN_DATA_TYPE_INT8, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysColumnsTable->createColumn(
            ColumnSpecification(kSysColumns_BlockDataAreaSize_Column, COLUMN_DATA_TYPE_UINT32,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    // Create columns of the table SYS_COLUMN_DEFS
    column = m_sysColumnDefsTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysColumnDefsTable->createColumn(
            ColumnSpecification(kSysColumnDefs_ColumnId_Column, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysColumnDefsTable->createColumn(
            ColumnSpecification(kSysColumnDefs_ConstraintCount_Column, COLUMN_DATA_TYPE_UINT32,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    // Create columns of the table SYS_COLUMN_SET_COLUMNS
    column = m_sysColumnSetColumnsTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysColumnSetColumnsTable->createColumn(ColumnSpecification(
            kSysColumnSetColumns_ColumnSetId_Column, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysColumnSetColumnsTable->createColumn(ColumnSpecification(
            kSysColumnSetColumns_ColumnDefinitionId_Column, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    // Create columns of the table SYS_CONSTRAINT_DEFS
    column = m_sysConstraintDefsTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysConstraintDefsTable->createColumn(
            ColumnSpecification(kSysConstraintDefs_Type_Column, COLUMN_DATA_TYPE_INT8,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysConstraintDefsTable->createColumn(
            ColumnSpecification(kSysConstraintDefs_Expr_Column, COLUMN_DATA_TYPE_BINARY,
                    kSystemTableDataFileDataAreaSize, noConstraintsSpec));
    allColumns.push_back(column);

    // Create columns of the table SYS_CONSTRAINTS
    column = m_sysConstraintsTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysConstraintsTable->createColumn(ColumnSpecification(kSysConstraints_Name_Column,
            COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysConstraintsTable->createColumn(ColumnSpecification(kSysConstraints_State_Column,
            COLUMN_DATA_TYPE_INT8, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysConstraintsTable->createColumn(ColumnSpecification(kSysConstraints_TableId_Column,
            COLUMN_DATA_TYPE_UINT32, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysConstraintsTable->createColumn(
            ColumnSpecification(kSysConstraints_ColumnId_Column, COLUMN_DATA_TYPE_UINT64,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysConstraintsTable->createColumn(
            ColumnSpecification(kSysConstraints_DefinitionId_Column, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    // Create columns of the table SYS_COLUMN_DEF_CONSTRAINTS
    column = m_sysColumnDefConstraintsTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysColumnDefConstraintsTable->createColumn(
            ColumnSpecification(kSysColumnDefinitionConstraintList_ColumnDefinitionId_Column,
                    Column::kMasterColumnDataType, kSystemTableDataFileDataAreaSize,
                    notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysColumnDefConstraintsTable->createColumn(ColumnSpecification(
            kSysColumnDefinitionConstraintList_ConstraintId_Column, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    // Create columns of the table SYS_INDICES
    column = m_sysIndicesTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysIndicesTable->createColumn(ColumnSpecification(kSysIndices_Type_Column,
            COLUMN_DATA_TYPE_INT16, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysIndicesTable->createColumn(ColumnSpecification(kSysIndices_Unique_Column,
            COLUMN_DATA_TYPE_BOOL, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysIndicesTable->createColumn(ColumnSpecification(kSysIndices_Name_Column,
            COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysIndicesTable->createColumn(
            ColumnSpecification(kSysIndices_TableId_Column, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysIndicesTable->createColumn(ColumnSpecification(kSysIndices_DataFileSize_Column,
            COLUMN_DATA_TYPE_UINT32, kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    // Create columns of the table SYS_INDEX_COLUMNS
    column = m_sysIndexColumnsTable->getMasterColumn();
    allColumns.push_back(column);
    masterColumns.push_back(column);

    column = m_sysIndexColumnsTable->createColumn(
            ColumnSpecification(kSysIndexColumns_IndexId_Column, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysIndexColumnsTable->createColumn(ColumnSpecification(
            kSysIndexColumns_ColumnDefinitionId_Column, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    column = m_sysIndexColumnsTable->createColumn(
            ColumnSpecification(kSysIndexColumns_SortDesc_Column, COLUMN_DATA_TYPE_BOOL,
                    kSystemTableDataFileDataAreaSize, notNullConstraintSpec));
    allColumns.push_back(column);

    // Close column sets
    for (const auto& table : allTables)
        table->closeCurrentColumnSet();

    // Create blocks for column.
    // NOTE: This is important to do in order to have rollback on error working correctly
    for (const auto& column : allColumns) {
        auto block = column->createBlock(0);
        column->updateBlockState(block->getId(), ColumnDataBlockState::kCurrent);
        block->setState(ColumnDataBlockState::kCurrent);
    }

    const auto& tp = m_metadata->getInitTransactionParams();

    // Record all tables and related objects
    for (const auto& table : allTables)
        recordTableDefinition(*table, tp);

    // Save system table info
    saveSystemObjectsInfo();
}

}  // namespace siodb::iomgr::dbengine
