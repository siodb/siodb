// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SystemDatabase.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDataBlock.h"
#include "ColumnDefinitionConstraint.h"
#include "ThrowDatabaseError.h"

// STL headers
#include <numeric>

namespace siodb::iomgr::dbengine {

SystemDatabase::SystemDatabase(
        Instance& instance, const std::string& cipherId, BinaryValue&& cipherKey)
    : Database(instance, kSystemDatabaseName, cipherId, std::move(cipherKey),
            m_allSystemTables.size() * 2, kSystemDatabaseDescription)
{
    // Initialize buffers
    std::vector<TablePtr> allTables;
    allTables.reserve(m_systemDatabaseOnlySystemTables.size());

    std::vector<ColumnSetPtr> allColumnSets;
    allColumnSets.reserve(allTables.capacity());

    std::vector<ColumnPtr> allColumns, masterColumns;
    masterColumns.reserve(allTables.capacity());
    allColumns.reserve(std::accumulate(m_systemDatabaseOnlySystemTables.cbegin(),
            m_systemDatabaseOnlySystemTables.cend(), static_cast<std::size_t>(0),
            [](std::size_t currentSize, const auto& tableName) {
                const auto it = m_allSystemTables.find(tableName);
                return currentSize + (it == m_allSystemTables.end() ? 0 : it->second.size());
            }));

    std::vector<ColumnDefinitionPtr> allColumnDefs;
    allColumnDefs.reserve(allColumns.capacity());

    std::vector<ConstraintPtr> allConstraints;
    allConstraints.reserve(allColumnDefs.capacity() * 2);

    std::vector<ColumnDefinitionConstraintPtr> allColumnDefinitionConstraintList;
    allColumnDefinitionConstraintList.reserve(allConstraints.capacity());

    // Empty constraint set
    const std::vector<ColumnConstraintSpecification> noConstraintsSpec;

    // Column constraint specification list with single "NOT NULL" constraint
    // and empty name, which will cause automatic constraint name generation
    const ColumnConstraintSpecificationList notNullConstraintSpec {
            ColumnConstraintSpecification(std::string(), ConstraintType::kNotNull,
                    requests::ExpressionPtr(
                            m_systemNotNullConstraintDefinition->getExpression().clone()),
                    kSystemNotNullConstraintDescription),
    };

    // Create table SYS_USERS
    m_sysUsersTable = createTableUnlocked(
            kSysUsersTableName, TableType::kDisk, kFirstUserUserId, kSysUsersTableDescription);
    allTables.push_back(m_sysUsersTable);

    // Create table SYS_USER_ACCESS_KEYS
    m_sysUserAccessKeysTable = createTableUnlocked(
            kSysUserAccessKeysTableName, TableType::kDisk, 0, kSysUserAccessKeysTableDescription);
    allTables.push_back(m_sysUserAccessKeysTable);
    // Skip one access key TRID for super user initial access key
    m_sysUserAccessKeysTable->generateNextUserTrid();

    // Create table SYS_DATABASES
    m_sysDatabasesTable = createTableUnlocked(kSysDatabasesTableName, TableType::kDisk,
            kFirstUserDatabaseId, kSysDatabasesTableDescription);
    allTables.push_back(m_sysDatabasesTable);
    m_sysDatabasesTable->setLastSystemTrid(m_id);

    // Create table SYS_USER_PERMISSIONS
    m_sysUserPermissionsTable = createTableUnlocked(
            kSysUserPermissionsTableName, TableType::kDisk, 0, kSysUserPermissionsTableDescription);
    allTables.push_back(m_sysUserPermissionsTable);

    // Create table SYS_USER_TOKENS
    m_sysUserTokensTable = createTableUnlocked(
            kSysUserTokensTableName, TableType::kDisk, 0, kSysUserTokensTableDescription);
    allTables.push_back(m_sysUserTokensTable);

    // Create columns of the table SYS_USERS
    allColumns.push_back(m_sysUsersTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysUsersTable->createColumn(ColumnSpecification(
            kSysUsers_Name_ColumnName, COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysUsers_Name_ColumnDescription)));

    allColumns.push_back(m_sysUsersTable->createColumn(ColumnSpecification(
            kSysUsers_RealName_ColumnName, COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
            stdext::copy(noConstraintsSpec), kSysUsers_RealName_ColumnDescription)));

    allColumns.push_back(m_sysUsersTable->createColumn(ColumnSpecification(
            kSysUsers_State_ColumnName, COLUMN_DATA_TYPE_UINT8, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysUsers_State_ColumnDescription)));

    allColumns.push_back(
            m_sysUsersTable->createColumn(ColumnSpecification(kSysUsers_Description_ColumnName,
                    COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
                    stdext::copy(noConstraintsSpec), kSysUsers_Description_ColumnDescription)));

    // Create columns of the table SYS_USER_ACCESS_KEYS
    allColumns.push_back(m_sysUserAccessKeysTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysUserAccessKeysTable->createColumn(
            ColumnSpecification(kSysUserAccessKeys_UserId_ColumnName, Column::kMasterColumnDataType,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysUserAccessKeys_UserId_ColumnDescription)));

    allColumns.push_back(m_sysUserAccessKeysTable->createColumn(
            ColumnSpecification(kSysUserAccessKeys_Name_ColumnName, COLUMN_DATA_TYPE_TEXT,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysUserAccessKeys_Name_ColumnDescription)));

    allColumns.push_back(m_sysUserAccessKeysTable->createColumn(
            ColumnSpecification(kSysUserAccessKeys_Text_ColumnName, COLUMN_DATA_TYPE_TEXT,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysUserAccessKeys_Text_ColumnDescription)));

    allColumns.push_back(m_sysUserAccessKeysTable->createColumn(
            ColumnSpecification(kSysUserAccessKeys_State_ColumnName, COLUMN_DATA_TYPE_UINT8,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysUserAccessKeys_State_ColumnDescription)));

    allColumns.push_back(m_sysUserAccessKeysTable->createColumn(
            ColumnSpecification(kSysUserAccessKeys_Description_ColumnName, COLUMN_DATA_TYPE_TEXT,
                    kSystemTableDataFileDataAreaSize, stdext::copy(noConstraintsSpec),
                    kSysUserAccessKeys_Description_ColumnDescription)));

    // Create columns of the table SYS_DATABASES
    allColumns.push_back(m_sysDatabasesTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysDatabasesTable->createColumn(ColumnSpecification(
            kSysDatabases_Uuid_ColumnName, COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysDatabases_Uuid_ColumnDescription)));

    allColumns.push_back(m_sysDatabasesTable->createColumn(ColumnSpecification(
            kSysDatabases_Name_ColumnName, COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysDatabases_Name_ColumnDescription)));

    allColumns.push_back(m_sysDatabasesTable->createColumn(
            ColumnSpecification(kSysDatabases_CipherId_ColumnName, COLUMN_DATA_TYPE_TEXT,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysDatabases_CipherId_ColumnDescription)));

    allColumns.push_back(m_sysDatabasesTable->createColumn(
            ColumnSpecification(kSysDatabases_Description_ColumnName, COLUMN_DATA_TYPE_TEXT,
                    kSystemTableDataFileDataAreaSize, stdext::copy(noConstraintsSpec),
                    kSysDatabases_Description_ColumnDescription)));

    // Create columns of the table SYS_USER_TOKENS
    allColumns.push_back(m_sysUserTokensTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(
            m_sysUserTokensTable->createColumn(ColumnSpecification(kSysUserTokens_UserId_ColumnName,
                    Column::kMasterColumnDataType, kSystemTableDataFileDataAreaSize,
                    stdext::copy(notNullConstraintSpec), kSysUserTokens_UserId_ColumnDescription)));

    allColumns.push_back(m_sysUserTokensTable->createColumn(ColumnSpecification(
            kSysUserTokens_Name_ColumnName, COLUMN_DATA_TYPE_TEXT, kSystemTableDataFileDataAreaSize,
            stdext::copy(notNullConstraintSpec), kSysUserTokens_Name_ColumnDescription)));

    allColumns.push_back(
            m_sysUserTokensTable->createColumn(ColumnSpecification(kSysUserTokens_Value_ColumnName,
                    COLUMN_DATA_TYPE_BINARY, kSystemTableDataFileDataAreaSize,
                    stdext::copy(notNullConstraintSpec), kSysUserTokens_Value_ColumnDescription)));

    allColumns.push_back(m_sysUserTokensTable->createColumn(ColumnSpecification(
            kSysUserTokens_ExpirationTimestamp_ColumnName, COLUMN_DATA_TYPE_TIMESTAMP,
            kSystemTableDataFileDataAreaSize, stdext::copy(noConstraintsSpec),
            kSysUserTokens_ExpirationTimestamp_ColumnDescription)));

    allColumns.push_back(m_sysUserTokensTable->createColumn(
            ColumnSpecification(kSysUserTokens_Description_ColumnName, COLUMN_DATA_TYPE_TEXT,
                    kSystemTableDataFileDataAreaSize, stdext::copy(noConstraintsSpec),
                    kSysUserTokens_Description_ColumnDescription)));

    // Table columns of the table SYS_USER_PERMISSIONS
    allColumns.push_back(m_sysUserPermissionsTable->getMasterColumn());
    masterColumns.push_back(allColumns.back());

    allColumns.push_back(m_sysUserPermissionsTable->createColumn(ColumnSpecification(
            kSysUserPermissions_UserId_ColumnName, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
            kSysUserPermissions_UserId_ColumnDescription)));

    allColumns.push_back(m_sysUserPermissionsTable->createColumn(ColumnSpecification(
            kSysUserPermissions_DatabaseId_ColumnName, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
            kSysUserPermissions_DatabaseId_ColumnDescription)));

    allColumns.push_back(m_sysUserPermissionsTable->createColumn(
            ColumnSpecification(kSysUserPermissions_ObjectType_ColumnName, COLUMN_DATA_TYPE_UINT8,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysUserPermissions_ObjectType_ColumnDescription)));

    allColumns.push_back(m_sysUserPermissionsTable->createColumn(ColumnSpecification(
            kSysUserPermissions_ObjectId_ColumnName, Column::kMasterColumnDataType,
            kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
            kSysUserPermissions_ObjectId_ColumnDescription)));

    allColumns.push_back(m_sysUserPermissionsTable->createColumn(
            ColumnSpecification(kSysUserPermissions_Permissions_ColumnName, COLUMN_DATA_TYPE_UINT64,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysUserPermissions_Permissions_ColumnDescription)));

    allColumns.push_back(m_sysUserPermissionsTable->createColumn(ColumnSpecification(
            kSysUserPermissions_GrantOptions_ColumnName, COLUMN_DATA_TYPE_UINT64,
            kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
            kSysUserPermissions_GrantOptions_ColumnDescription)));

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

    // Record database
    recordDatabase(*this, tp);

    // Save system table info
    saveSystemObjectsInfo();

    // Indicate that database is initialized
    createInitializationFlagFile();
}

SystemDatabase::SystemDatabase(Instance& instance, const std::string& cipherId)
    : Database(instance,
            DatabaseRecord(kSystemDatabaseId, kSystemDatabaseUuid, kSystemDatabaseName,
                    std::string(cipherId), kSystemDatabaseDescription),
            m_allSystemTables.size() * 2)
    , m_sysUsersTable(loadSystemTable(kSysUsersTableName))
    , m_sysUserAccessKeysTable(loadSystemTable(kSysUserAccessKeysTableName))
    , m_sysUserTokensTable(loadSystemTable(kSysUserTokensTableName))
    , m_sysDatabasesTable(loadSystemTable(kSysDatabasesTableName))
    , m_sysUserPermissionsTable(loadSystemTable(kSysUserPermissionsTableName))
{
}

}  // namespace siodb::iomgr::dbengine
