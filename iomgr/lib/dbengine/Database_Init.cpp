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
#include "User.h"
#include "crypto/GetCipher.h"
#include "parser/expr/ConstantExpression.h"

// Common project headers
#include <siodb/common/stl_ext/utility_ext.h>
#include <siodb/common/utils/FSUtils.h>

// STL headers
#include <numeric>

namespace siodb::iomgr::dbengine {

const Uuid Database::kSystemDatabaseUuid {{0x68, 0xba, 0x3, 0x8e, 0xb7, 0x4, 0x2c, 0xb9, 0x1d, 0xd,
        0xb9, 0x18, 0x64, 0xc8, 0x19, 0xcd}};

const std::unordered_map<std::string, std::unordered_set<std::string>> Database::m_allSystemTables {
        // All databases
        {kSysTablesTableName,
                {
                        kMasterColumnName,
                        kSysTables_Type_ColumnName,
                        kSysTables_Name_ColumnName,
                        kSysTables_FirstUserTrid_ColumnName,
                        kSysTables_CurrentColumnSetId_ColumnName,
                        kSysTables_Description_ColumnName,
                }},
        {kSysDummyTableName,
                {
                        kMasterColumnName,
                        kSysDummy_Dummy_ColumnName,
                }},
        {kSysColumnSetsTableName,
                {
                        kMasterColumnName,
                        kSysColumnSets_TableId_ColumnName,
                        kSysColumnSets_ColumnCount_ColumnName,
                }},
        {kSysColumnsTableName,
                {
                        kMasterColumnName,
                        kSysColumns_TableId_ColumnName,
                        kSysColumns_DataType_ColumnName,
                        kSysColumns_Name_ColumnName,
                        kSysColumns_State_ColumnName,
                        kSysColumns_BlockDataAreaSize_ColumnName,
                        kSysColumns_Description_ColumnName,
                }},
        {kSysColumnDefsTableName,
                {
                        kMasterColumnName,
                        kSysColumnDefs_ColumnId_ColumnName,
                        kSysColumnDefs_ConstraintCount_ColumnName,
                }},
        {kSysColumnSetColumnsTableName,
                {
                        kMasterColumnName,
                        kSysColumnSetColumns_ColumnSetId_ColumnName,
                        kSysColumnSetColumns_ColumnDefinitionId_ColumnName,
                }},
        {kSysConstraintDefsTableName,
                {
                        kMasterColumnName,
                        kSysConstraintDefs_Type_ColumnName,
                        kSysConstraintDefs_Expr_ColumnName,
                }},
        {kSysConstraintsTableName,
                {
                        kMasterColumnName,
                        kSysConstraints_State_ColumnName,
                        kSysConstraints_Name_ColumnName,
                        kSysConstraints_TableId_ColumnName,
                        kSysConstraints_ColumnId_ColumnName,
                        kSysConstraints_DefinitionId_ColumnName,
                        kSysConstraints_Description_ColumnName,
                }},
        {kSysColumnDefConstraintsTableName,
                {
                        kMasterColumnName,
                        kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnName,
                        kSysColumnDefinitionConstraintList_ConstraintId_ColumnName,
                }},
        {kSysIndicesTableName,
                {
                        kMasterColumnName,
                        kSysIndices_Type_ColumnName,
                        kSysIndices_Unique_ColumnName,
                        kSysIndices_Name_ColumnName,
                        kSysIndices_TableId_ColumnName,
                        kSysIndices_DataFileSize_ColumnName,
                        kSysIndices_Description_ColumnName,
                }},
        {kSysIndexColumnsTableName,
                {
                        kMasterColumnName,
                        kSysIndexColumns_IndexId_ColumnName,
                        kSysIndexColumns_ColumnDefinitionId_ColumnName,
                        kSysIndexColumns_SortDesc_ColumnName,
                }},
        // Only system database
        {kSysUsersTableName,
                {
                        kMasterColumnName,
                        kSysUsers_Name_ColumnName,
                        kSysUsers_RealName_ColumnName,
                        kSysUsers_State_ColumnName,
                }},
        {kSysUserAccessKeysTableName,
                {
                        kMasterColumnName,
                        kSysUserAccessKeys_UserId_ColumnName,
                        kSysUserAccessKeys_Name_ColumnName,
                        kSysUserAccessKeys_State_ColumnName,
                        kSysUserAccessKeys_Text_ColumnName,
                        kSysUserAccessKeys_Description_ColumnName,
                }},
        {kSysDatabasesTableName,
                {
                        kMasterColumnName,
                        kSysDatabases_Uuid_ColumnName,
                        kSysDatabases_Name_ColumnName,
                        kSysDatabases_CipherId_ColumnName,
                        kSysDatabases_Description_ColumnName,
                        kSysDatabases_MaxTables_ColumnName,
                }},
        {kSysUserPermissionsTableName,
                {
                        kMasterColumnName,
                        kSysUserPermissions_UserId_ColumnName,
                        kSysUserPermissions_DatabaseId_ColumnName,
                        kSysUserPermissions_ObjectType_ColumnName,
                        kSysUserPermissions_ObjectId_ColumnName,
                        kSysUserPermissions_Permissions_ColumnName,
                        kSysUserPermissions_GrantOptions_ColumnName,
                }},
        {kSysUserTokensTableName,
                {
                        kMasterColumnName,
                        kSysUserTokens_UserId_ColumnName,
                        kSysUserTokens_Name_ColumnName,
                        kSysUserTokens_Value_ColumnName,
                        kSysUserTokens_Description_ColumnName,
                        kSysUserTokens_ExpirationTimestamp_ColumnName,
                }},
};

const std::unordered_set<std::string> Database::m_systemDatabaseOnlySystemTables {
        kSysDatabasesTableName,
        kSysUsersTableName,
        kSysUserAccessKeysTableName,
        kSysUserPermissionsTableName,
        kSysUserTokensTableName,
};

// New database
Database::Database(Instance& instance, std::string&& name, const std::string& cipherId,
        BinaryValue&& cipherKey, std::optional<std::string>&& description,
        std::uint32_t maxTableCount)
    : m_instance(instance)
    , m_uuid(computeDatabaseUuid(
              name, name == kSystemDatabaseName ? kSystemDatabaseCreationTime : std::time(nullptr)))
    , m_name(validateDatabaseName(std::move(name)))
    , m_description(std::move(description))
    , m_id(instance.generateNextDatabaseId(name == kSystemDatabaseName))
    , m_dataDir(ensureDataDir(true))
    , m_cipher(crypto::getCipher(cipherId))
    , m_cipherKey(std::move(cipherKey))
    , m_encryptionContext(m_cipher ? m_cipher->createEncryptionContext(m_cipherKey) : nullptr)
    , m_decryptionContext(m_cipher ? m_cipher->createDecryptionContext(m_cipherKey) : nullptr)
    , m_maxTableCount(maxTableCount)
    , m_metadataFile(createMetadataFile())
    , m_metadata(static_cast<DatabaseMetadata*>(m_metadataFile->getMappingAddress()))
    , m_createTransactionParams(User::kSuperUserId, generateNextTransactionId())
    , m_useCount(0)
    , m_systemNotNullConstraintDefinition(createSystemConstraintDefinitionUnlocked(
              ConstraintType::kNotNull, std::make_unique<requests::ConstantExpression>(true)))
    , m_systenDefaultZeroConstraintDefinition(createSystemConstraintDefinitionUnlocked(
              ConstraintType::kDefaultValue, std::make_unique<requests::ConstantExpression>(0)))
{
    createSystemTables();
    saveCurrentCipherKey();
}

// Init existing database
Database::Database(Instance& instance, const DatabaseRecord& dbRecord)
    : m_instance(instance)
    , m_uuid(dbRecord.m_uuid)
    , m_name(dbRecord.m_name)
    , m_description(dbRecord.m_description)
    , m_id(dbRecord.m_id)
    , m_dataDir(ensureDataDir(false))
    , m_cipher(crypto::getCipher(dbRecord.m_cipherId))
    , m_cipherKey(loadCipherKey())
    , m_encryptionContext(m_cipher ? m_cipher->createEncryptionContext(m_cipherKey) : nullptr)
    , m_decryptionContext(m_cipher ? m_cipher->createDecryptionContext(m_cipherKey) : nullptr)
    , m_maxTableCount(dbRecord.m_maxTableCount)
    , m_metadataFile(openMetadataFile())
    , m_metadata(static_cast<DatabaseMetadata*>(m_metadataFile->getMappingAddress()))
    , m_useCount(0)
    , m_sysTablesTable(loadSystemTable(kSysTablesTableName))
    , m_sysDummyTable(loadSystemTable(kSysDummyTableName))
    , m_sysColumnSetsTable(loadSystemTable(kSysColumnSetsTableName))
    , m_sysColumnsTable(loadSystemTable(kSysColumnsTableName))
    , m_sysColumnDefsTable(loadSystemTable(kSysColumnDefsTableName))
    , m_sysColumnSetColumnsTable(loadSystemTable(kSysColumnSetColumnsTableName))
    , m_sysConstraintDefsTable(loadSystemTable(kSysConstraintDefsTableName))
    , m_sysConstraintsTable(loadSystemTable(kSysConstraintsTableName))
    , m_sysColumnDefConstraintsTable(loadSystemTable(kSysColumnDefConstraintsTableName))
    , m_sysIndicesTable(loadSystemTable(kSysIndicesTableName))
    , m_sysIndexColumnsTable(loadSystemTable(kSysIndexColumnsTableName))
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
    allColumns.reserve(std::accumulate(m_allSystemTables.cbegin(), m_allSystemTables.cend(),
            static_cast<std::size_t>(0), [](std::size_t currentSize, const auto& e) noexcept {
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
    m_sysTablesTable = createTableUnlocked(
            kSysTablesTableName, TableType::kDisk, kFirstUserTableId, kSysTablesTableDescription);
    m_sysTablesTable->generateNextSystemTrid();
    allTables.push_back(m_sysTablesTable);

    // Create table SYS_DUMMY
    m_sysDummyTable =
            createTableUnlocked(kSysDummyTableName, TableType::kDisk, 0, kSysDummyTableDescription);
    allTables.push_back(m_sysDummyTable);

    // Create table SYS_COLUMN_SETS
    m_sysColumnSetsTable = createTableUnlocked(kSysColumnSetsTableName, TableType::kDisk,
            kFirstUserTableColumnSetId, kSysColumnSetsTableDescription);
    allTables.push_back(m_sysColumnSetsTable);
    m_sysColumnSetsTable->setLastSystemTrid(m_tmpTridCounters.m_lastColumnSetId);

    // Create table SYS_COLUMNS
    m_sysColumnsTable = createTableUnlocked(kSysColumnsTableName, TableType::kDisk,
            kFirstUserTableColumnId, kSysColumnsTableDescription);
    allTables.push_back(m_sysColumnsTable);
    m_sysColumnsTable->setLastSystemTrid(m_tmpTridCounters.m_lastColumnId);

    // Create table SYS_COLUMN_DEFS
    m_sysColumnDefsTable = createTableUnlocked(kSysColumnDefsTableName, TableType::kDisk,
            kFirstUserTableColumnDefinitionId, kSysColumnDefsTableDescription);
    allTables.push_back(m_sysColumnDefsTable);
    m_sysColumnDefsTable->setLastSystemTrid(m_tmpTridCounters.m_lastColumnDefinitionId);

    // Create table SYS_COLUMN_SET_COLUMNS
    m_sysColumnSetColumnsTable =
            createTableUnlocked(kSysColumnSetColumnsTableName, TableType::kDisk,
                    kFirstUserTableColumnSetColumnId, kSysColumnSetColumnsTableDescription);
    allTables.push_back(m_sysColumnSetColumnsTable);
    m_sysColumnSetColumnsTable->setLastSystemTrid(m_tmpTridCounters.m_lastColumnSetColumnId);

    // Create table SYS_CONSTRAINT_DEFS
    m_sysConstraintDefsTable = createTableUnlocked(kSysConstraintDefsTableName, TableType::kDisk,
            kFirstUserTableConstraintDefinitionId, kSysConstraintDefsTableDescription);
    allTables.push_back(m_sysConstraintDefsTable);
    m_sysConstraintDefsTable->setLastSystemTrid(m_tmpTridCounters.m_lastConstraintDefinitionId);

    // Create table SYS_CONSTRAINTS
    m_sysConstraintsTable = createTableUnlocked(kSysConstraintsTableName, TableType::kDisk,
            kFirstUserTableConstraintId, kSysConstraintsTableDescription);
    allTables.push_back(m_sysConstraintsTable);
    m_sysConstraintsTable->setLastSystemTrid(m_tmpTridCounters.m_lastConstraintId);

    // Create table SYS_COLUMN_DEF_CONSTRAINTS
    m_sysColumnDefConstraintsTable = createTableUnlocked(kSysColumnDefConstraintsTableName,
            TableType::kDisk, kFirstUserTableColumnDefinitionConstraintId,
            kSysColumnDefConstraintsTableDescription);
    allTables.push_back(m_sysColumnDefConstraintsTable);
    m_sysColumnDefConstraintsTable->setLastSystemTrid(
            m_tmpTridCounters.m_lastColumnDefinitionConstraintId);

    // Create table SYS_INDICES
    m_sysIndicesTable = createTableUnlocked(kSysIndicesTableName, TableType::kDisk,
            kFirstUserTableIndexId, kSysIndicesTableDescription);
    allTables.push_back(m_sysIndicesTable);
    m_sysIndicesTable->setLastSystemTrid(m_tmpTridCounters.m_lastIndexId);

    // Create table SYS_INDEX_COLUMNS
    m_sysIndexColumnsTable = createTableUnlocked(kSysIndexColumnsTableName, TableType::kDisk,
            kFirstUserTableIndexColumnId, kSysIndexColumnsTableDescription);
    allTables.push_back(m_sysIndexColumnsTable);
    m_sysIndexColumnsTable->setLastSystemTrid(m_tmpTridCounters.m_lastIndexColumnId);

    // Empty constraint set
    const ColumnConstraintSpecificationList noConstraintsSpec;

    // Column constraint specification list with single "NOT NULL" constraint
    // and empty name, which will cause automatic constraint name generation.
    const ColumnConstraintSpecificationList notNullConstraintSpec {
            ColumnConstraintSpecification(std::string(), ConstraintType::kNotNull,
                    requests::ExpressionPtr(
                            m_systemNotNullConstraintDefinition->getExpression().clone()),
                    kSystemNotNullConstraintDescription),
    };

    // Create columns of the table SYS_TABLES
    allColumns.push_back(m_sysTablesTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysTablesTable->createColumn(ColumnSpecification(
            kSysTables_Type_ColumnName, COLUMN_DATA_TYPE_INT8, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysTables_Type_ColumnDescription)));

    allColumns.push_back(m_sysTablesTable->createColumn(ColumnSpecification(
            kSysTables_Name_ColumnName, COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysTables_Name_ColumnDescription)));

    allColumns.push_back(m_sysTablesTable->createColumn(
            ColumnSpecification(kSysTables_FirstUserTrid_ColumnName, COLUMN_DATA_TYPE_UINT64,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysTables_FirstUserTrid_ColumnDescription)));

    allColumns.push_back(m_sysTablesTable->createColumn(ColumnSpecification(
            kSysTables_CurrentColumnSetId_ColumnName, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
            kSysTables_CurrentColumnSetId_ColumnDescription)));

    allColumns.push_back(
            m_sysTablesTable->createColumn(ColumnSpecification(kSysTables_Description_ColumnName,
                    COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
                    stdext::copy(noConstraintsSpec), kSysTables_Description_ColumnDescription)));

    // Create columns of the table SYS_DUMMY
    allColumns.push_back(m_sysDummyTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysDummyTable->createColumn(ColumnSpecification(
            kSysDummy_Dummy_ColumnName, COLUMN_DATA_TYPE_INT32, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysDummy_Dummy_ColumnDescription)));

    // Create columns of the table SYS_COLUMN_SETS
    allColumns.push_back(m_sysColumnSetsTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysColumnSetsTable->createColumn(
            ColumnSpecification(kSysColumnSets_TableId_ColumnName, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysColumnSets_TableId_ColumnDescription)));

    allColumns.push_back(m_sysColumnSetsTable->createColumn(
            ColumnSpecification(kSysColumnSets_ColumnCount_ColumnName, COLUMN_DATA_TYPE_UINT32,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysColumnSets_ColumnCount_ColumnDescription)));

    // Create columns of the table SYS_COLUMNS
    allColumns.push_back(m_sysColumnsTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(
            m_sysColumnsTable->createColumn(ColumnSpecification(kSysColumns_TableId_ColumnName,
                    Column::kMasterColumnDataType, kSystemTableDataFileDataAreaSize,
                    stdext::copy(notNullConstraintSpec), kSysColumns_TableId_ColumnDescription)));

    allColumns.push_back(
            m_sysColumnsTable->createColumn(ColumnSpecification(kSysColumns_DataType_ColumnName,
                    COLUMN_DATA_TYPE_INT8, kSystemTableDataFileDataAreaSize,
                    stdext::copy(notNullConstraintSpec), kSysColumns_DataType_ColumnDescription)));

    allColumns.push_back(m_sysColumnsTable->createColumn(ColumnSpecification(
            kSysColumns_Name_ColumnName, COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysColumns_Name_ColumnDescription)));

    allColumns.push_back(m_sysColumnsTable->createColumn(ColumnSpecification(
            kSysColumns_State_ColumnName, COLUMN_DATA_TYPE_INT8, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysColumns_State_ColumnDescription)));

    allColumns.push_back(m_sysColumnsTable->createColumn(
            ColumnSpecification(kSysColumns_BlockDataAreaSize_ColumnName, COLUMN_DATA_TYPE_UINT32,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysColumns_BlockDataAreaSize_ColumnDescription)));

    allColumns.push_back(
            m_sysColumnsTable->createColumn(ColumnSpecification(kSysColumns_Description_ColumnName,
                    COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
                    stdext::copy(noConstraintsSpec), kSysColumns_Description_ColumnDescription)));

    // Create columns of the table SYS_COLUMN_DEFS
    allColumns.push_back(m_sysColumnDefsTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysColumnDefsTable->createColumn(
            ColumnSpecification(kSysColumnDefs_ColumnId_ColumnName, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysColumnDefs_ColumnId_ColumnDescription)));

    allColumns.push_back(m_sysColumnDefsTable->createColumn(
            ColumnSpecification(kSysColumnDefs_ConstraintCount_ColumnName, COLUMN_DATA_TYPE_UINT32,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysColumnDefs_ConstraintCount_ColumnDescription)));

    // Create columns of the table SYS_COLUMN_SET_COLUMNS
    allColumns.push_back(m_sysColumnSetColumnsTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysColumnSetColumnsTable->createColumn(ColumnSpecification(
            kSysColumnSetColumns_ColumnSetId_ColumnName, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
            kSysColumnSetColumns_ColumnSetId_ColumnDescription)));

    allColumns.push_back(m_sysColumnSetColumnsTable->createColumn(ColumnSpecification(
            kSysColumnSetColumns_ColumnDefinitionId_ColumnName, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
            kSysColumnSetColumns_ColumnDefinitionId_ColumnDescription)));

    // Create columns of the table SYS_CONSTRAINT_DEFS
    allColumns.push_back(m_sysConstraintDefsTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysConstraintDefsTable->createColumn(
            ColumnSpecification(kSysConstraintDefs_Type_ColumnName, COLUMN_DATA_TYPE_INT8,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysConstraintDefs_Type_ColumnDescription)));

    allColumns.push_back(m_sysConstraintDefsTable->createColumn(
            ColumnSpecification(kSysConstraintDefs_Expr_ColumnName, COLUMN_DATA_TYPE_BINARY,
                    kSystemTableDataFileDataAreaSize, stdext::copy(noConstraintsSpec),
                    kSysConstraintDefs_Expr_ColumnDescription)));

    // Create columns of the table SYS_CONSTRAINTS
    allColumns.push_back(m_sysConstraintsTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(
            m_sysConstraintsTable->createColumn(ColumnSpecification(kSysConstraints_Name_ColumnName,
                    COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
                    stdext::copy(notNullConstraintSpec), kSysConstraints_Name_ColumnDescription)));

    allColumns.push_back(m_sysConstraintsTable->createColumn(
            ColumnSpecification(kSysConstraints_State_ColumnName, COLUMN_DATA_TYPE_INT8,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysConstraints_State_ColumnDescription)));

    allColumns.push_back(m_sysConstraintsTable->createColumn(
            ColumnSpecification(kSysConstraints_TableId_ColumnName, COLUMN_DATA_TYPE_UINT32,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysConstraints_TableId_ColumnDescription)));

    allColumns.push_back(m_sysConstraintsTable->createColumn(
            ColumnSpecification(kSysConstraints_ColumnId_ColumnName, COLUMN_DATA_TYPE_UINT64,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysConstraints_ColumnId_ColumnDescription)));

    allColumns.push_back(m_sysConstraintsTable->createColumn(ColumnSpecification(
            kSysConstraints_DefinitionId_ColumnName, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
            kSysConstraints_DefinitionId_ColumnDescription)));

    allColumns.push_back(m_sysConstraintsTable->createColumn(
            ColumnSpecification(kSysConstraints_Description_ColumnName, COLUMN_DATA_TYPE_TEXT,
                    kSystemTableDataFileDataAreaSize, stdext::copy(noConstraintsSpec),
                    kSysConstraints_Description_ColumnDescription)));

    // Create columns of the table SYS_COLUMN_DEF_CONSTRAINTS
    allColumns.push_back(m_sysColumnDefConstraintsTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysColumnDefConstraintsTable->createColumn(
            ColumnSpecification(kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnName,
                    Column::kMasterColumnDataType, kSystemTableDataFileDataAreaSize,
                    stdext::copy(notNullConstraintSpec),
                    kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnDescription)));

    allColumns.push_back(m_sysColumnDefConstraintsTable->createColumn(
            ColumnSpecification(kSysColumnDefinitionConstraintList_ConstraintId_ColumnName,
                    Column::kMasterColumnDataType, kSystemTableDataFileDataAreaSize,
                    stdext::copy(notNullConstraintSpec),
                    kSysColumnDefinitionConstraintList_ConstraintId_ColumnDescription)));

    // Create columns of the table SYS_INDICES
    allColumns.push_back(m_sysIndicesTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysIndicesTable->createColumn(ColumnSpecification(
            kSysIndices_Type_ColumnName, COLUMN_DATA_TYPE_INT16, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysIndices_Type_ColumnDescription)));

    allColumns.push_back(m_sysIndicesTable->createColumn(ColumnSpecification(
            kSysIndices_Unique_ColumnName, COLUMN_DATA_TYPE_BOOL, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysIndices_Unique_ColumnDescription)));

    allColumns.push_back(m_sysIndicesTable->createColumn(ColumnSpecification(
            kSysIndices_Name_ColumnName, COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysIndices_Name_ColumnDescription)));

    allColumns.push_back(
            m_sysIndicesTable->createColumn(ColumnSpecification(kSysIndices_TableId_ColumnName,
                    Column::kMasterColumnDataType, kSystemTableDataFileDataAreaSize,
                    stdext::copy(notNullConstraintSpec), kSysIndices_TableId_ColumnDescription)));

    allColumns.push_back(m_sysIndicesTable->createColumn(
            ColumnSpecification(kSysIndices_DataFileSize_ColumnName, COLUMN_DATA_TYPE_UINT32,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysIndices_DataFileSize_ColumnDescription)));

    allColumns.push_back(
            m_sysIndicesTable->createColumn(ColumnSpecification(kSysIndices_Description_ColumnName,
                    COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
                    stdext::copy(noConstraintsSpec), kSysIndices_Description_ColumnDescription)));

    // Create columns of the table SYS_INDEX_COLUMNS
    allColumns.push_back(m_sysIndexColumnsTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysIndexColumnsTable->createColumn(
            ColumnSpecification(kSysIndexColumns_IndexId_ColumnName, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysIndexColumns_IndexId_ColumnDescription)));

    allColumns.push_back(m_sysIndexColumnsTable->createColumn(ColumnSpecification(
            kSysIndexColumns_ColumnDefinitionId_ColumnName, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
            kSysIndexColumns_ColumnDefinitionId_ColumnDescription)));

    allColumns.push_back(m_sysIndexColumnsTable->createColumn(
            ColumnSpecification(kSysIndexColumns_SortDesc_ColumnName, COLUMN_DATA_TYPE_BOOL,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysIndexColumns_SortDesc_ColumnDescription)));

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
