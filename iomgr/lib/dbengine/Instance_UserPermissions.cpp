// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Instance.h"

// Project headers
#include "SystemDatabase.h"
#include "Table.h"
#include "ThrowDatabaseError.h"
#include "UserAccessKey.h"

// Common project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include <siodb/common/log/Log.h>
#include <siodb/iomgr/shared/dbengine/PermissionType.h>
#include <siodb/iomgr/shared/dbengine/parser/CommonConstants.h>

namespace siodb::iomgr::dbengine {

const std::unordered_map<DatabaseObjectType, std::uint64_t> Instance::s_allowedPermissions {
        {DatabaseObjectType::kInstance, buildMultiPermissionMask<PermissionType::kShutdown>()},
        {DatabaseObjectType::kDatabase,
                buildMultiPermissionMask<PermissionType::kShow, PermissionType::kCreate,
                        PermissionType::kDrop, PermissionType::kAlter, PermissionType::kAttach,
                        PermissionType::kDetach>()},
        {DatabaseObjectType::kTable,
                buildMultiPermissionMask<PermissionType::kSelect, PermissionType::kSelectSystem,
                        PermissionType::kInsert, PermissionType::kUpdate, PermissionType::kDelete,
                        PermissionType::kShow, PermissionType::kShowSystem, PermissionType::kCreate,
                        PermissionType::kDrop, PermissionType::kAlter>()},
};

std::optional<std::uint64_t> Instance::getAllObjectTypePermissions(
        DatabaseObjectType objectType) noexcept
{
    const auto it = s_allowedPermissions.find(objectType);
    return it != s_allowedPermissions.end() ? it->second : std::optional<std::uint64_t>();
}

void Instance::grantTablePermissionsToUser(const std::string& userName,
        const std::string& databaseName, const std::string& tableName, std::uint64_t permissions,
        bool withGrantOption, std::uint32_t currentUserId)
{
    const auto user = findUserChecked(userName);
    DatabasePtr database;
    std::unique_ptr<UseDatabaseGuard> databaseGuard;
    if (databaseName != requests::kAllObjectsName) {
        database = findDatabaseChecked(databaseName);
        databaseGuard = std::make_unique<UseDatabaseGuard>(*database);
    }
    TablePtr table;
    if (tableName != requests::kAllObjectsName) {
        if (!databaseGuard)
            throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, tableName);
        table = database->findTableChecked(tableName);
    }
    grantObjectPermissionsToUser(*user, database ? database->getId() : 0,
            DatabaseObjectType::kTable, table ? table->getId() : 0, permissions, withGrantOption,
            currentUserId);
}

void Instance::grantObjectPermissionsToUser(std::uint32_t userId, std::uint32_t databaseId,
        DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
        bool withGrantOption, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_mutex);
    const auto user = findUserCheckedUnlocked(userId);
    grantObjectPermissionsToUserUnlocked(
            *user, databaseId, objectType, objectId, permissions, withGrantOption, currentUserId);
}

void Instance::grantObjectPermissionsToUser(User& user, std::uint32_t databaseId,
        DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
        bool withGrantOption, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_mutex);
    grantObjectPermissionsToUserUnlocked(
            user, databaseId, objectType, objectId, permissions, withGrantOption, currentUserId);
}

void Instance::revokeTablePermissionsFromUser(const std::string& userName,
        const std::string& databaseName, const std::string& tableName, std::uint64_t permissions,
        std::uint32_t currentUserId)
{
    const auto user = findUserChecked(userName);
    DatabasePtr database;
    std::unique_ptr<UseDatabaseGuard> databaseGuard;
    if (databaseName != requests::kAllObjectsName) {
        database = findDatabaseChecked(databaseName);
        databaseGuard = std::make_unique<UseDatabaseGuard>(*database);
    }
    TablePtr table;
    if (tableName != requests::kAllObjectsName) {
        if (!databaseGuard)
            throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, tableName);
        table = database->findTableChecked(tableName);
    }
    revokeObjectPermissionsFromUser(*user, database ? database->getId() : 0,
            DatabaseObjectType::kTable, table ? table->getId() : 0, permissions, currentUserId);
}

void Instance::revokeAllObjectPermissionsFromAllUsers(std::uint32_t databaseId,
        DatabaseObjectType objectType, std::uint64_t objectId, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_mutex);
    for (const auto& u : m_users) {
        revokeObjectPermissionsFromUserUnlocked(*u.second, databaseId, objectType, objectId,
                kAllPermissionsForRevoke, currentUserId);
    }
}

void Instance::revokeObjectPermissionsFromUser(User& user, std::uint32_t databaseId,
        DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
        std::uint32_t currentUserId)
{
    std::lock_guard lock(m_mutex);
    revokeObjectPermissionsFromUserUnlocked(
            user, databaseId, objectType, objectId, permissions, currentUserId);
}

// --- internals ---

void Instance::grantObjectPermissionsToUserUnlocked(User& user, std::uint32_t databaseId,
        DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
        bool withGrantOption, std::uint32_t currentUserId)
{
    validatePermissions(objectType, permissions);

    const auto currentUser = findUserCheckedUnlocked(currentUserId);
    const UserPermissionKey permissionKey(databaseId, objectType, objectId);
    if (!currentUser->hasPermissions(permissionKey, permissions, true))
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);

    const auto& usersById = m_userRegistry.byId();
    const auto it = usersById.find(user.getId());
    if (it == usersById.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, user.getId());

    auto& userRecord = stdext::as_mutable(*it);
    auto& permissionsById = userRecord.m_permissions.byId();
    auto permissionData = user.grantPermissions(permissionKey, permissions, withGrantOption);
    if (permissionData.getId() != 0) {
        m_systemDatabase->updateUserPermission(permissionData, currentUserId);
        const auto it2 = permissionsById.find(permissionData.getId());
        if (it2 == permissionsById.end()) {
            LOG_ERROR << "Permission record #" << permissionData.getId()
                      << " not found for the user #" << user.getId();
            const UserPermissionRecord record(user.getId(), permissionKey, permissionData);
            permissionsById.insert(record);
        } else {
            auto& permissionRecord = stdext::as_mutable(*it2);
            permissionRecord.m_permissions = permissionData.getPermissions();
            permissionRecord.m_grantOptions = permissionData.getGrantOptions();
        }
    } else {
        const TransactionParameters tp(
                currentUserId, m_systemDatabase->generateNextTransactionId());
        const auto permissionId = m_systemDatabase->recordUserPermission(
                user.getId(), permissionKey, permissionData, tp);
        user.setPermissionRecordId(permissionKey, permissionId);
        permissionData.setId(permissionId);
        const UserPermissionRecord permissionRecord(user.getId(), permissionKey, permissionData);
        userRecord.m_permissions.insert(permissionRecord);
    }
}

void Instance::revokeObjectPermissionsFromUserUnlocked(User& user, std::uint32_t databaseId,
        DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
        std::uint32_t currentUserId)
{
    if (permissions != kAllPermissionsForRevoke) validatePermissions(objectType, permissions);

    const auto currentUser = findUserCheckedUnlocked(currentUserId);
    const UserPermissionKey permissionKey(databaseId, objectType, objectId);
    if (!currentUser->hasPermissions(permissionKey, permissions, true))
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);

    const auto& usersById = m_userRegistry.byId();
    const auto it = usersById.find(user.getId());
    if (it == usersById.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, user.getId());
    auto& userRecord = stdext::as_mutable(*it);
    auto& permissionsById = userRecord.m_permissions.byId();
    const auto result = user.revokePermissions(permissionKey, permissions);
    if (result) {
        const auto permissionId = result->first.getId();
        if (result->second) {
            m_systemDatabase->deleteUserPermission(permissionId, currentUserId);
            permissionsById.erase(permissionId);
        } else {
            m_systemDatabase->updateUserPermission(result->first, currentUserId);
            const auto it2 = permissionsById.find(permissionId);
            if (it2 == permissionsById.end()) {
                LOG_ERROR << "Permission record #" << permissionId << " not found for the user #"
                          << user.getId();
                const UserPermissionRecord permissionRecord(
                        user.getId(), permissionKey, result->first);
                userRecord.m_permissions.insert(permissionRecord);
            } else {
                auto& permissionRecord = stdext::as_mutable(*it2);
                permissionRecord.m_permissions = result->first.getPermissions();
                permissionRecord.m_grantOptions = result->first.getGrantOptions();
            }
        }
    }
}

void Instance::validatePermissions(DatabaseObjectType objectType, std::uint64_t permissions)
{
    if (!isValidPermissions(objectType, permissions))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidPermissionSpecification);
}

bool Instance::isValidPermissions(DatabaseObjectType objectType, std::uint64_t permissions) noexcept
{
    const auto it = s_allowedPermissions.find(objectType);
    return it != s_allowedPermissions.end() && (permissions & it->second) == permissions;
}

}  // namespace siodb::iomgr::dbengine
