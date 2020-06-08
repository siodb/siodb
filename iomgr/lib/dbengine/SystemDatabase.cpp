// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SystemDatabase.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDataBlock.h"
#include "ColumnDefinitionConstraint.h"
#include "ColumnSetColumn.h"
#include "Index.h"
#include "MasterColumnRecord.h"
#include "ThrowDatabaseError.h"
#include "UserAccessKey.h"
#include "UserPermission.h"
#include "parser/expr/ConstantExpression.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_ext/utility_ext.h>
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// STL headers
#include <numeric>

// Boost headers
#include <boost/lexical_cast.hpp>

#define CREATE_DEMO_TABLES

namespace siodb::iomgr::dbengine {

SystemDatabase::SystemDatabase(Instance& instance, const std::string& cipherId,
        BinaryValue&& cipherKey, [[maybe_unused]] const std::string& superUserInitialAccessKey)
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

    // Create table SYS_DATABASES
    m_sysDatabasesTable = createTableUnlocked(kSysDatabasesTableName, TableType::kDisk,
            kFirstUserDatabaseId, kSysDatabasesTableDescription);
    allTables.push_back(m_sysDatabasesTable);
    m_sysDatabasesTable->setLastSystemTrid(m_id);

    // Create table SYS_USER_PERMISSIONS
    m_sysUserPermissionsTable = createTableUnlocked(
            kSysUserPermissionsTableName, TableType::kDisk, 0, kSysUserPermissionsTableDescription);
    allTables.push_back(m_sysUserPermissionsTable);

    // Create columns of the table SYS_USER_ACCESS_KEYS
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
            ColumnSpecification(kSysDatabases_CipherKey_ColumnName, COLUMN_DATA_TYPE_BINARY,
                    kSystemTableDataFileDataAreaSize, stdext::copy(notNullConstraintSpec),
                    kSysDatabases_CipherKey_ColumnDescription)));

    allColumns.push_back(m_sysDatabasesTable->createColumn(
            ColumnSpecification(kSysDatabases_Description_ColumnName, COLUMN_DATA_TYPE_TEXT,
                    kSystemTableDataFileDataAreaSize, stdext::copy(noConstraintsSpec),
                    kSysDatabases_Description_ColumnDescription)));

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

    // Demo stuff
    createDemoTables(User::kSuperUserId);

    // Indicate that database is initialized
    createInitializationFlagFile();
}

SystemDatabase::SystemDatabase(
        Instance& instance, const std::string& cipherId, BinaryValue&& cipherKey)
    : Database(instance,
            DatabaseRecord(kSystemDatabaseId, kSystemDatabaseUuid, kSystemDatabaseName,
                    std::string(cipherId), std::move(cipherKey), kSystemDatabaseDescription),
            m_allSystemTables.size() * 2)
    , m_sysUsersTable(loadSystemTable(kSysUsersTableName))
    , m_sysUserAccessKeysTable(loadSystemTable(kSysUserAccessKeysTableName))
    , m_sysDatabasesTable(loadSystemTable(kSysDatabasesTableName))
    , m_sysUserPermissionsTable(loadSystemTable(kSysUserPermissionsTableName))
{
}

bool SystemDatabase::isSystemDatabase() const noexcept
{
    return true;
}

void SystemDatabase::readAllUsers(UserRegistry& userRegistry)
{
    std::unordered_map<std::uint32_t, UserAccessKeyRegistry> userAccessKeyRegistries;
    readAllUserAccessKeys(userAccessKeyRegistries);

    LOG_DEBUG << "Reading all users.";

    // Obtain columns
    const auto masterColumn = m_sysUsersTable->getMasterColumn();
    const auto nameColumn = m_sysUsersTable->findColumnChecked(kSysUsers_Name_ColumnName);
    const auto realNameColumn = m_sysUsersTable->findColumnChecked(kSysUsers_RealName_ColumnName);
    const auto stateColumn = m_sysUsersTable->findColumnChecked(kSysUsers_State_ColumnName);
    const auto descriptionColumn =
            m_sysUsersTable->findColumnChecked(kSysUsers_Description_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "readAllUsers: Decoded MinTRID=" << minTrid << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysUsersTable->getName(), m_uuid, m_sysUsersTable->getId(), 1);
    }
    if (maxTrid == 0) {
        userRegistry.clear();
        LOG_DEBUG << "There are no users.";
        return;
    }

    // Scan table and fill temporary registry
    const auto expectedColumnCount = m_sysUsersTable->getColumnCount() - 1;
    UserRegistry reg;
    std::uint8_t value[12];
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        if (index->findValue(currentKey, value, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysUsersTable->getName(), m_uuid, m_sysUsersTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(value, sizeof(value));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysUsersTable->getName(), m_uuid, m_sysUsersTable->getId(),
                    mcrAddr.getBlockId(), mcrAddr.getOffset(), expectedColumnCount,
                    mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant nameValue, realNameValue, stateValue, descriptionValue;
        std::size_t colIndex = 0;
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue, false);
        realNameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), realNameValue, false);
        stateColumn->readRecord(columnRecords.at(colIndex++).getAddress(), stateValue, false);
        descriptionColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), descriptionValue, false);
        const auto userId = static_cast<std::uint32_t>(mcr.getTableRowId());
        UserAccessKeyRegistry accessKeys;
        const auto it = userAccessKeyRegistries.find(userId);
        if (it != userAccessKeyRegistries.end()) accessKeys.swap(it->second);
        UserRecord userRecord(userId, std::move(*nameValue.asString()),
                realNameValue.asOptionalString(), descriptionValue.asOptionalString(),
                stateValue.asUInt8() != 0, std::move(accessKeys));
        LOG_DEBUG << "Database " << m_name << ": readAllUsers: User #" << trid << " '"
                  << userRecord.m_name << '\'';
        reg.insert(std::move(userRecord));
    } while (index->findNextKey(currentKey, nextKey));

    // Finally swap registries
    userRegistry.swap(reg);
    LOG_DEBUG << "Read " << userRegistry.size() << " users.";
}

void SystemDatabase::readAllDatabases(DatabaseRegistry& databaseRegistry)
{
    LOG_DEBUG << "Reading all databases.";

    // Obtain columns
    const auto masterColumn = m_sysDatabasesTable->getMasterColumn();
    const auto uuidColumn = m_sysDatabasesTable->findColumnChecked(kSysDatabases_Uuid_ColumnName);
    const auto nameColumn = m_sysDatabasesTable->findColumnChecked(kSysDatabases_Name_ColumnName);
    const auto cipherIdColumn =
            m_sysDatabasesTable->findColumnChecked(kSysDatabases_CipherId_ColumnName);
    const auto cipherKeyColumn =
            m_sysDatabasesTable->findColumnChecked(kSysDatabases_CipherKey_ColumnName);
    const auto descriptionColumn =
            m_sysDatabasesTable->findColumnChecked(kSysDatabases_Description_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "readAllDatabases: Decoded MinTRID=" << minTrid << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysDatabasesTable->getName(), m_uuid, m_sysDatabasesTable->getId(), 1);
    }
    if (maxTrid == 0) {
        databaseRegistry.clear();
        LOG_DEBUG << "There are no databases.";
        return;
    }

    // Scan table and fill temporary registry
    const auto expectedColumnCount = m_sysDatabasesTable->getColumnCount() - 1;
    DatabaseRegistry reg;
    std::uint8_t value[12];
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        LOG_DEBUG << "readAllDatabases: Next key: " << trid;
        if (index->findValue(currentKey, value, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysDatabasesTable->getName(), m_uuid, m_sysDatabasesTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(value, sizeof(value));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysDatabasesTable->getName(), m_uuid, m_sysDatabasesTable->getId(),
                    mcrAddr.getBlockId(), mcrAddr.getOffset(), expectedColumnCount,
                    mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant uuidValue, nameValue, cipherIdValue, cipherKeyValue, descriptionValue;
        std::size_t colIndex = 0;
        uuidColumn->readRecord(columnRecords.at(colIndex++).getAddress(), uuidValue, false);
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue, false);
        cipherIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), cipherIdValue, false);
        cipherKeyColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), cipherKeyValue, false);
        descriptionColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), descriptionValue, false);
        DatabaseRecord databaseRecord(static_cast<std::uint32_t>(mcr.getTableRowId()),
                boost::lexical_cast<Uuid>(*uuidValue.asString()), std::move(*nameValue.asString()),
                std::move(*cipherIdValue.asString()), std::move(*cipherKeyValue.asBinary()),
                descriptionValue.asOptionalString());
        LOG_DEBUG << "Database " << m_name << ": readAllDatabases: Database #" << trid << " '"
                  << databaseRecord.m_name << '\'';
        reg.insert(std::move(databaseRecord));
    } while (index->findNextKey(currentKey, nextKey));

    // Finally swap registries
    databaseRegistry.swap(reg);
    LOG_DEBUG << "Read " << databaseRegistry.size() << " databases.";
}

std::uint32_t SystemDatabase::generateNextUserId()
{
    return static_cast<std::uint32_t>(m_sysUsersTable->generateNextUserTrid());
}

std::uint64_t SystemDatabase::generateNextUserAccessKeyId()
{
    return m_sysUserAccessKeysTable->generateNextUserTrid();
}

std::uint32_t SystemDatabase::generateNextDatabaseId(bool system)
{
    const auto databaseId =
            system ? (m_sysDatabasesTable ? m_sysDatabasesTable->generateNextSystemTrid()
                                          : kSystemDatabaseId)
                   : m_sysDatabasesTable->generateNextUserTrid();
    if (databaseId >= std::numeric_limits<std::uint32_t>::max())
        throwDatabaseError(IOManagerMessageId::kErrorInstanceResourceExhausted, "Database ID");
    return static_cast<std::uint32_t>(databaseId);
}

std::uint64_t SystemDatabase::generateNextUserPermissionId()
{
    return m_sysUserPermissionsTable->generateNextUserTrid();
}

void SystemDatabase::recordUser(const User& user, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording user #" << user.getId() << ' '
              << user.getName();
    std::vector<Variant> values(m_sysUsersTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = user.getName();
    values.at(i++) = user.getRealName();
    values.at(i++) = static_cast<std::uint8_t>(user.isActive() ? 1 : 0);
    values.at(i++) = user.getDescription();
    m_sysUsersTable->insertRow(values, tp, user.getId());
}

void SystemDatabase::recordUserAccessKey(
        const UserAccessKey& accessKey, const TransactionParameters& tp)
{
    const auto& user = accessKey.getUser();
    LOG_DEBUG << "Database " << m_name << ": Recording user access key #" << accessKey.getId()
              << ' ' << accessKey.getName() << "for the user #" << user.getId() << ' '
              << user.getName();
    std::vector<Variant> values(m_sysUserAccessKeysTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = accessKey.getUserId();
    values.at(i++) = accessKey.getName();
    values.at(i++) = accessKey.getText();
    values.at(i++) = (std::uint8_t(accessKey.isActive() ? 1 : 0));
    values.at(i++) = accessKey.getDescription();
    m_sysUserAccessKeysTable->insertRow(values, tp, accessKey.getId());
}

void SystemDatabase::recordDatabase(const Database& database, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording database #" << database.getId() << ' '
              << database.getName();
    std::vector<Variant> values(m_sysDatabasesTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = boost::uuids::to_string(database.getUuid());
    values.at(i++) = database.getName();
    values.at(i++) = database.getCipherId();
    values.at(i++) = database.getCipherKey();
    values.at(i++) = database.getDescription();
    m_sysDatabasesTable->insertRow(values, tp, database.getId());
}

void SystemDatabase::recordUserPermission(
        const UserPermission& permission, const TransactionParameters& tp)
{
    LOG_DEBUG << "Database " << m_name << ": Recording user permission record #"
              << permission.getId() << " for the user #" << permission.getUserId();
    std::vector<Variant> values(m_sysUserPermissionsTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = permission.getUserId();
    values.at(i++) = permission.getDatabaseId();
    values.at(i++) = static_cast<std::uint8_t>(permission.getObjectType());
    values.at(i++) = permission.getObjectId();
    values.at(i++) = permission.getPermissions();
    values.at(i++) = permission.getGrantOptions();
    m_sysUserPermissionsTable->insertRow(values, tp, permission.getId());
}

void SystemDatabase::deleteDatabase(std::uint32_t databaseId, std::uint32_t currentUserId)
{
    const TransactionParameters tp(currentUserId, generateNextTransactionId());
    m_sysDatabasesTable->deleteRow(databaseId, tp);
}

void SystemDatabase::deleteUser(std::uint32_t userId, std::uint32_t currentUserId)
{
    const TransactionParameters tp(currentUserId, generateNextTransactionId());
    m_sysUsersTable->deleteRow(userId, tp);
}

void SystemDatabase::deleteUserAccessKey(std::uint64_t accessKeyId, std::uint32_t currentUserId)
{
    const TransactionParameters tp(currentUserId, generateNextTransactionId());
    m_sysUserAccessKeysTable->deleteRow(accessKeyId, tp);
}

void SystemDatabase::updateUser(
        std::uint32_t userId, const UpdateUserParameters& params, std::uint32_t currentUserId)
{
    const TransactionParameters tp(currentUserId, generateNextTransactionId());
    std::vector<Variant> columnValues;
    std::vector<std::size_t> columnPositions;

    constexpr std::size_t kMaxNumberOfUpdatedColumns = 3;
    columnValues.reserve(kMaxNumberOfUpdatedColumns);
    columnPositions.reserve(kMaxNumberOfUpdatedColumns);

    if (params.m_realName) {
        columnValues.emplace_back(*params.m_realName);
        columnPositions.push_back(m_sysUsersTable->findColumnChecked(kSysUsers_RealName_ColumnName)
                                          ->getCurrentPosition());
    }

    if (params.m_description) {
        columnValues.emplace_back(*params.m_description);
        columnPositions.push_back(
                m_sysUsersTable->findColumnChecked(kSysUsers_Description_ColumnName)
                        ->getCurrentPosition());
    }

    if (params.m_active) {
        columnValues.emplace_back(std::uint8_t(*params.m_active ? 1 : 0));
        columnPositions.push_back(m_sysUsersTable->findColumnChecked(kSysUsers_State_ColumnName)
                                          ->getCurrentPosition());
    }

    if (columnValues.empty()) return;

    if (!m_sysUsersTable->updateRow(userId, std::move(columnValues), columnPositions, tp))
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userId);
}

void SystemDatabase::updateUserAccessKey(std::uint64_t accessKeyId,
        const UpdateUserAccessKeyParameters& params, std::uint32_t currentUserId)
{
    const TransactionParameters tp(currentUserId, generateNextTransactionId());
    std::vector<Variant> columnValues;
    std::vector<std::size_t> columnPositions;

    constexpr std::size_t kMaxNumberOfUpdatedColumns = 2;
    columnValues.reserve(kMaxNumberOfUpdatedColumns);
    columnPositions.reserve(kMaxNumberOfUpdatedColumns);

    if (params.m_description) {
        columnValues.emplace_back(*params.m_description);
        columnPositions.push_back(
                m_sysUserAccessKeysTable
                        ->findColumnChecked(kSysUserAccessKeys_Description_ColumnName)
                        ->getCurrentPosition());
    }

    if (params.m_active) {
        columnValues.emplace_back(std::uint8_t(*params.m_active ? 1 : 0));
        columnPositions.push_back(
                m_sysUserAccessKeysTable->findColumnChecked(kSysUserAccessKeys_State_ColumnName)
                        ->getCurrentPosition());
    }

    if (columnValues.empty()) return;
    if (!m_sysUserAccessKeysTable->updateRow(
                accessKeyId, std::move(columnValues), columnPositions, tp)) {
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyDoesNotExist, accessKeyId);
    }
}

void SystemDatabase::createDemoTables([[maybe_unused]] std::uint32_t currentUserId)
{
#ifdef CREATE_DEMO_TABLES
    LOG_DEBUG << "Creating demo tables.";

    const std::vector<SimpleColumnSpecification> customersTableColumnDefs {
            {"FIRST_NAME", COLUMN_DATA_TYPE_TEXT, true},
            {"LAST_NAME", COLUMN_DATA_TYPE_TEXT, true},
    };

    const std::vector<SimpleColumnSpecification> itemsTableColumnDefs {
            {"NAME", COLUMN_DATA_TYPE_TEXT, true},
            {"PRICE", COLUMN_DATA_TYPE_DOUBLE, true},
    };

    const std::vector<SimpleColumnSpecification> ordersTableColumnDefs {
            {"CUSTOMER_ID", COLUMN_DATA_TYPE_UINT64, true},
            {"BILLING_ADDR", COLUMN_DATA_TYPE_TEXT, true},
            {"SHIPPING_ADDR", COLUMN_DATA_TYPE_TEXT, true},
    };

    const std::vector<SimpleColumnSpecification> orderItemsTableColumnDefs {
            {"ORDER_ID", COLUMN_DATA_TYPE_UINT64, true},
            {"ITEM_ID", COLUMN_DATA_TYPE_UINT64, true},
            {"QTY", COLUMN_DATA_TYPE_INT32, true},
            {"PRICE", COLUMN_DATA_TYPE_DOUBLE, true},
            {"DISCOUNT_PCT", COLUMN_DATA_TYPE_DOUBLE, true, 0.0},
    };

    const std::vector<SimpleColumnSpecification> digitalBooksColumnDefs {
            {"DIGITAL_SIGNATURE", COLUMN_DATA_TYPE_BINARY, true},
            {"BOOK_TEXT", COLUMN_DATA_TYPE_TEXT, true},
    };

    const std::vector<SimpleColumnSpecification> contractsColumnDefs {
            {"START_DATE", COLUMN_DATA_TYPE_TIMESTAMP, true, "CURRENT_TIMESTAMP"},
            {"FINISH_DATE", COLUMN_DATA_TYPE_TIMESTAMP, false},
    };

    createUserTable("CUSTOMERS", TableType::kDisk, customersTableColumnDefs, currentUserId, {});
    createUserTable("ITEMS", TableType::kDisk, itemsTableColumnDefs, currentUserId, {});
    createUserTable("ORDERS", TableType::kDisk, ordersTableColumnDefs, currentUserId, {});
    createUserTable("ORDER_ITEMS", TableType::kDisk, orderItemsTableColumnDefs, currentUserId, {});
    createUserTable("DIGITAL_BOOKS", TableType::kDisk, digitalBooksColumnDefs, currentUserId, {});
    createUserTable("CONTRACTS", TableType::kDisk, contractsColumnDefs, currentUserId, {});
#endif
}

// ----- internals -----

std::size_t SystemDatabase::readAllUserAccessKeys(UserAccessKeyRegistries& userAccessKeyRegistries)
{
    LOG_DEBUG << "Reading all users access keys.";

    // Obtain columns
    const auto masterColumn = m_sysUserAccessKeysTable->getMasterColumn();
    const auto userIdColumn =
            m_sysUserAccessKeysTable->findColumnChecked(kSysUserAccessKeys_UserId_ColumnName);
    const auto nameColumn =
            m_sysUserAccessKeysTable->findColumnChecked(kSysUserAccessKeys_Name_ColumnName);
    const auto textColumn =
            m_sysUserAccessKeysTable->findColumnChecked(kSysUserAccessKeys_Text_ColumnName);
    const auto stateColumn =
            m_sysUserAccessKeysTable->findColumnChecked(kSysUserAccessKeys_State_ColumnName);
    const auto descriptionColumn =
            m_sysUserAccessKeysTable->findColumnChecked(kSysUserAccessKeys_Description_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "readAllUserAccessKeys: Decoded MinTRID=" << minTrid << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysUserAccessKeysTable->getName(), m_uuid, m_sysUserAccessKeysTable->getId(), 1);
    }
    if (maxTrid == 0) {
        userAccessKeyRegistries.clear();
        LOG_DEBUG << "There are no user access keys.";
        return 0;
    }

    // Scan table and fill temporary registry
    const auto expectedColumnCount = m_sysUserAccessKeysTable->getColumnCount() - 1;
    std::unordered_map<std::uint32_t, UserAccessKeyRegistry> regs;
    std::uint8_t value[12];
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        if (index->findValue(currentKey, value, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysUserAccessKeysTable->getName(), m_uuid, m_sysUserAccessKeysTable->getId(),
                    2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(value, sizeof(value));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysUserAccessKeysTable->getName(), m_uuid,
                    m_sysUserAccessKeysTable->getId(), mcrAddr.getBlockId(), mcrAddr.getOffset(),
                    expectedColumnCount, mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant userIdValue, nameValue, stateValue, textValue, descriptionValue;
        std::size_t colIndex = 0;
        // Column order: Id, name, text, state
        userIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), userIdValue, false);
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue, false);
        textColumn->readRecord(columnRecords.at(colIndex++).getAddress(), textValue, false);
        stateColumn->readRecord(columnRecords.at(colIndex++).getAddress(), stateValue, false);
        descriptionColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), descriptionValue, false);
        UserAccessKeyRecord accessKeyRecord(mcr.getTableRowId(), userIdValue.asUInt32(),
                std::move(*nameValue.asString()), std::move(*textValue.asString()),
                descriptionValue.asOptionalString(), stateValue.asUInt8() != 0);
        LOG_DEBUG << "Database " << m_name << ": readAllUserAccessKeys: User access key #" << trid
                  << " '" << accessKeyRecord.m_name << '\'';
        regs[accessKeyRecord.m_userId].insert(std::move(accessKeyRecord));
    } while (index->findNextKey(currentKey, nextKey));

    // Finally swap registries
    userAccessKeyRegistries.swap(regs);

    // Count keys
    const auto keyCount = std::accumulate(userAccessKeyRegistries.begin(),
            userAccessKeyRegistries.end(), std::size_t(0),
            [](std::size_t a, const UserAccessKeyRegistries::value_type& b) noexcept {
                return a + b.second.size();
            });
    LOG_DEBUG << "Read " << keyCount << " user access keys.";
    return keyCount;
}

}  // namespace siodb::iomgr::dbengine
