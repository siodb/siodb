// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SystemDatabase.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Column.h"
#include "ThrowDatabaseError.h"
#include "UserAccessKey.h"
#include "UserPermission.h"
#include "UserToken.h"

// Common project headers
#include <siodb/common/log/Log.h>

namespace siodb::iomgr::dbengine {

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
    m_sysUsersTable->insertRow(std::move(values), tp, user.getId());
}

void SystemDatabase::recordUserAccessKey(
        const UserAccessKey& accessKey, const TransactionParameters& tp)
{
    const auto& user = accessKey.getUser();
    LOG_DEBUG << "Database " << m_name << ": Recording user access key #" << accessKey.getId()
              << " '" << accessKey.getName() << "' for the user #" << user.getId() << ' '
              << user.getName();
    std::vector<Variant> values(m_sysUserAccessKeysTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = accessKey.getUserId();
    values.at(i++) = accessKey.getName();
    values.at(i++) = accessKey.getText();
    values.at(i++) = (std::uint8_t(accessKey.isActive() ? 1 : 0));
    values.at(i++) = accessKey.getDescription();
    m_sysUserAccessKeysTable->insertRow(std::move(values), tp, accessKey.getId());
}

void SystemDatabase::recordUserToken(const UserToken& token, const TransactionParameters& tp)
{
    const auto& user = token.getUser();
    LOG_DEBUG << "Database " << m_name << ": Recording user token #" << token.getId() << " '"
              << token.getName() << "' for the user #" << user.getId() << ' ' << user.getName();
    std::vector<Variant> values(m_sysUserTokensTable->getColumnCount() - 1);
    std::size_t i = 0;
    values.at(i++) = token.getUserId();
    values.at(i++) = token.getName();
    values.at(i++) = token.getValue();
    if (token.getExpirationTimestamp()) values.at(i) = RawDateTime(*token.getExpirationTimestamp());
    i++;
    values.at(i++) = token.getDescription();
    m_sysUserTokensTable->insertRow(std::move(values), tp, token.getId());
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
    m_sysDatabasesTable->insertRow(std::move(values), tp, database.getId());
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
    m_sysUserPermissionsTable->insertRow(std::move(values), tp, permission.getId());
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

void SystemDatabase::deleteUserToken(std::uint64_t tokenId, std::uint32_t currentUserId)
{
    const TransactionParameters tp(currentUserId, generateNextTransactionId());
    m_sysUserTokensTable->deleteRow(tokenId, tp);
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
        columnPositions.push_back(m_sysUsersTable->findColumnChecked(kSysUsers_RealName_ColumnName)
                                          ->getCurrentPosition());
        columnValues.emplace_back(*params.m_realName);
    }

    if (params.m_description) {
        columnPositions.push_back(
                m_sysUsersTable->findColumnChecked(kSysUsers_Description_ColumnName)
                        ->getCurrentPosition());
        columnValues.emplace_back(*params.m_description);
    }

    if (params.m_active) {
        columnPositions.push_back(m_sysUsersTable->findColumnChecked(kSysUsers_State_ColumnName)
                                          ->getCurrentPosition());
        columnValues.emplace_back(std::uint8_t(*params.m_active ? 1 : 0));
    }

    if (columnValues.empty()) return;

    const auto result =
            m_sysUsersTable->updateRow(userId, columnPositions, std::move(columnValues), tp);
    if (!std::get<0>(result))
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

    if (params.m_active) {
        columnPositions.push_back(
                m_sysUserAccessKeysTable->findColumnChecked(kSysUserAccessKeys_State_ColumnName)
                        ->getCurrentPosition());
        columnValues.emplace_back(std::uint8_t(*params.m_active ? 1 : 0));
    }

    if (params.m_description) {
        columnPositions.push_back(
                m_sysUserAccessKeysTable
                        ->findColumnChecked(kSysUserAccessKeys_Description_ColumnName)
                        ->getCurrentPosition());
        columnValues.emplace_back(*params.m_description);
    }

    if (columnValues.empty()) return;

    const auto result = m_sysUserAccessKeysTable->updateRow(
            accessKeyId, columnPositions, std::move(columnValues), tp);
    if (!std::get<0>(result))
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyIdDoesNotExist, accessKeyId);
}

void SystemDatabase::updateUserToken(
        std::uint64_t tokenId, const UpdateUserTokenParameters& params, std::uint32_t currentUserId)
{
    const TransactionParameters tp(currentUserId, generateNextTransactionId());
    std::vector<Variant> columnValues;
    std::vector<std::size_t> columnPositions;

    constexpr std::size_t kMaxNumberOfUpdatedColumns = 2;
    columnValues.reserve(kMaxNumberOfUpdatedColumns);
    columnPositions.reserve(kMaxNumberOfUpdatedColumns);

    if (params.m_expirationTimestamp) {
        columnPositions.push_back(
                m_sysUserTokensTable
                        ->findColumnChecked(kSysUserTokens_ExpirationTimestamp_ColumnName)
                        ->getCurrentPosition());

        if (*params.m_expirationTimestamp)
            columnValues.emplace_back(RawDateTime(**params.m_expirationTimestamp));
        else
            columnValues.emplace_back();
    }

    if (params.m_description) {
        columnPositions.push_back(
                m_sysUserTokensTable->findColumnChecked(kSysUserTokens_Description_ColumnName)
                        ->getCurrentPosition());
        columnValues.emplace_back(*params.m_description);
    }

    if (columnValues.empty()) return;

    const auto result =
            m_sysUserTokensTable->updateRow(tokenId, columnPositions, std::move(columnValues), tp);
    if (!std::get<0>(result))
        throwDatabaseError(IOManagerMessageId::kErrorUserTokenIdDoesNotExist, tokenId);
}

}  // namespace siodb::iomgr::dbengine
