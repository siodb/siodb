// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SystemDatabase.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Column.h"
#include "Index.h"
#include "ThrowDatabaseError.h"
#include "UserAccessKey.h"
#include "UserPermission.h"
#include "UserToken.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/ConstantExpression.h>

// STL headers
#include <numeric>

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

void SystemDatabase::readAllUsers(UserRegistry& userRegistry)
{
    auto userAccessKeyRegistries = readAllUserAccessKeys();
    auto userTokenRegistries = readAllUserTokens();

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
                    m_sysUsersTable->getName(), m_uuid, m_sysUsersTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

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
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue);
        realNameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), realNameValue);
        stateColumn->readRecord(columnRecords.at(colIndex++).getAddress(), stateValue);
        descriptionColumn->readRecord(columnRecords.at(colIndex++).getAddress(), descriptionValue);

        const auto userId = static_cast<std::uint32_t>(mcr.getTableRowId());

        UserAccessKeyRegistry accessKeys;
        const auto it = userAccessKeyRegistries.find(userId);
        if (it != userAccessKeyRegistries.end()) accessKeys.swap(it->second);

        UserTokenRegistry tokens;
        const auto it2 = userTokenRegistries.find(userId);
        if (it2 != userTokenRegistries.end()) tokens.swap(it2->second);

        UserRecord userRecord(userId, std::move(*nameValue.asString()),
                realNameValue.asOptionalString(), descriptionValue.asOptionalString(),
                stateValue.asUInt8() != 0, std::move(accessKeys), std::move(tokens));

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
    const auto descriptionColumn =
            m_sysDatabasesTable->findColumnChecked(kSysDatabases_Description_ColumnName);
    const auto maxTablesColumn =
            m_sysDatabasesTable->findColumnChecked(kSysDatabases_MaxTables_ColumnName);

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
    IndexValue indexValue;
    std::uint8_t* currentKey = key + 8;
    std::uint8_t* nextKey = key;
    do {
        // Obtain master column record address
        std::swap(currentKey, nextKey);
        std::uint64_t trid = 0;
        ::pbeDecodeUInt64(currentKey, &trid);
        LOG_DEBUG << "readAllDatabases: Next key: " << trid;
        if (index->find(currentKey, indexValue.m_data, 1) != 1) {
            throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                    m_sysDatabasesTable->getName(), m_uuid, m_sysDatabasesTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

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
        Variant uuidValue, nameValue, cipherIdValue, descriptionValue, maxTablesValue;
        std::size_t colIndex = 0;
        uuidColumn->readRecord(columnRecords.at(colIndex++).getAddress(), uuidValue);
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue);
        cipherIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), cipherIdValue);
        descriptionColumn->readRecord(columnRecords.at(colIndex++).getAddress(), descriptionValue);
        maxTablesColumn->readRecord(columnRecords.at(colIndex++).getAddress(), maxTablesValue);
        DatabaseRecord databaseRecord(static_cast<std::uint32_t>(mcr.getTableRowId()),
                boost::lexical_cast<Uuid>(*uuidValue.asString()), std::move(*nameValue.asString()),
                std::move(*cipherIdValue.asString()), descriptionValue.asOptionalString(),
                maxTablesValue.asUInt32());
        LOG_DEBUG << "Database " << m_name << ": readAllDatabases: Database #" << trid << " '"
                  << databaseRecord.m_name << '\'';
        reg.insert(std::move(databaseRecord));
    } while (index->findNextKey(currentKey, nextKey));

    // Finally swap registries
    databaseRegistry.swap(reg);
    LOG_DEBUG << "Read " << databaseRegistry.size() << " databases.";
}

// ----- internals -----

SystemDatabase::UserAccessKeyRegistries SystemDatabase::readAllUserAccessKeys()
{
    LOG_DEBUG << "Reading all user access keys.";

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
        LOG_DEBUG << "There are no user access keys.";
        return UserAccessKeyRegistries();
    }

    // Scan table and fill temporary registry
    const auto expectedColumnCount = m_sysUserAccessKeysTable->getColumnCount() - 1;
    UserAccessKeyRegistries registries;
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
                    m_sysUserAccessKeysTable->getName(), m_uuid, m_sysUserAccessKeysTable->getId(),
                    2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

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
        Variant userIdValue, nameValue, textValue, stateValue, descriptionValue;
        std::size_t colIndex = 0;
        // Column order: Id, name, text, state
        userIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), userIdValue);
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue);
        textColumn->readRecord(columnRecords.at(colIndex++).getAddress(), textValue);
        stateColumn->readRecord(columnRecords.at(colIndex++).getAddress(), stateValue);
        descriptionColumn->readRecord(columnRecords.at(colIndex++).getAddress(), descriptionValue);

        UserAccessKeyRecord accessKeyRecord(mcr.getTableRowId(), userIdValue.asUInt32(),
                std::move(*nameValue.asString()), std::move(*textValue.asString()),
                descriptionValue.asOptionalString(), stateValue.asUInt8() != 0);

        LOG_DEBUG << "Database " << m_name << ": readAllUserAccessKeys: User access key #" << trid
                  << " '" << accessKeyRecord.m_name << '\'';
        registries[accessKeyRecord.m_userId].insert(std::move(accessKeyRecord));
    } while (index->findNextKey(currentKey, nextKey));

    const auto keyCount = std::accumulate(registries.begin(), registries.end(), std::size_t(0),
            [](std::size_t a, const UserAccessKeyRegistries::value_type& b) noexcept {
                return a + b.second.size();
            });
    LOG_DEBUG << "Read " << keyCount << " user access keys.";
    return registries;
}

SystemDatabase::UserTokenRegistries SystemDatabase::readAllUserTokens()
{
    LOG_DEBUG << "Reading all user tokens.";

    // Obtain columns
    const auto masterColumn = m_sysUserTokensTable->getMasterColumn();
    const auto userIdColumn =
            m_sysUserTokensTable->findColumnChecked(kSysUserTokens_UserId_ColumnName);
    const auto nameColumn = m_sysUserTokensTable->findColumnChecked(kSysUserTokens_Name_ColumnName);
    const auto valueColumn =
            m_sysUserTokensTable->findColumnChecked(kSysUserTokens_Value_ColumnName);
    const auto expirationDateColumn =
            m_sysUserTokensTable->findColumnChecked(kSysUserTokens_ExpirationTimestamp_ColumnName);
    const auto descriptionColumn =
            m_sysUserTokensTable->findColumnChecked(kSysUserTokens_Description_ColumnName);

    // Obtain min and max TRID
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[16];
    std::uint64_t minTrid = 0, maxTrid = 0;
    if (index->getMinKey(key) && index->getMaxKey(key + 8)) {
        ::pbeDecodeUInt64(key, &minTrid);
        ::pbeDecodeUInt64(key + 8, &maxTrid);
        LOG_DEBUG << "readAllUserTokens: Decoded MinTRID=" << minTrid << " MaxTRID=" << maxTrid;
    }

    // Check min and max TRID
    if (minTrid > maxTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted, m_name,
                m_sysUserAccessKeysTable->getName(), m_uuid, m_sysUserAccessKeysTable->getId(), 1);
    }
    if (maxTrid == 0) {
        LOG_DEBUG << "There are no user tokens.";
        return UserTokenRegistries();
    }

    // Scan table and fill temporary registry
    const auto expectedColumnCount = m_sysUserTokensTable->getColumnCount() - 1;
    UserTokenRegistries registries;
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
                    m_sysUserTokensTable->getName(), m_uuid, m_sysUserTokensTable->getId(), 2);
        }
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));

        // Read and validate master column record
        MasterColumnRecord mcr;
        masterColumn->readMasterColumnRecord(mcrAddr, mcr);
        if (mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    m_name, m_sysUserTokensTable->getName(), m_uuid, m_sysUserTokensTable->getId(),
                    mcrAddr.getBlockId(), mcrAddr.getOffset(), expectedColumnCount,
                    mcr.getColumnCount());
        }

        // Read data from columns
        const auto& columnRecords = mcr.getColumnRecords();
        Variant userIdValue, nameValue, valueValue, expirationDateValue, descriptionValue;
        std::size_t colIndex = 0;
        userIdColumn->readRecord(columnRecords.at(colIndex++).getAddress(), userIdValue);
        nameColumn->readRecord(columnRecords.at(colIndex++).getAddress(), nameValue);
        valueColumn->readRecord(columnRecords.at(colIndex++).getAddress(), valueValue);
        expirationDateColumn->readRecord(
                columnRecords.at(colIndex++).getAddress(), expirationDateValue);
        descriptionColumn->readRecord(columnRecords.at(colIndex++).getAddress(), descriptionValue);
        UserTokenRecord tokenRecord(mcr.getTableRowId(), userIdValue.asUInt32(),
                std::move(*nameValue.asString()), std::move(*valueValue.asBinary()),
                expirationDateValue.isNull()
                        ? std::nullopt
                        : std::optional<std::time_t>(
                                expirationDateValue.asDateTime().toEpochTimestamp()),
                descriptionValue.asOptionalString());
        LOG_DEBUG << "Database " << m_name << ": readAllUserTokens: User token #" << trid << " '"
                  << tokenRecord.m_name << '\'' << " userId=" << tokenRecord.m_userId;
        registries[tokenRecord.m_userId].insert(std::move(tokenRecord));
    } while (index->findNextKey(currentKey, nextKey));

    const auto tokenCount = std::accumulate(registries.begin(), registries.end(), std::size_t(0),
            [](std::size_t a, const UserTokenRegistries::value_type& b) noexcept {
                return a + b.second.size();
            });
    LOG_DEBUG << "Read " << tokenCount << " user tokens.";
    return registries;
}

}  // namespace siodb::iomgr::dbengine
