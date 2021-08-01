// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Instance.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "SystemDatabase.h"
#include "ThrowDatabaseError.h"
#include "User.h"
#include "UserDatabase.h"

// Common project headers
#include <siodb/common/stl_ext/algorithm_ext.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>

namespace siodb::iomgr::dbengine {

std::size_t Instance::getDatabaseCount() const
{
    std::lock_guard lock(m_mutex);
    return m_databaseRegistry.size();
}

std::vector<DatabaseRecord> Instance::getDatabaseRecordsOrderedByName() const
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_databaseRegistry.byName();
    std::vector<DatabaseRecord> databaseRecords(index.cbegin(), index.cend());
    std::sort(databaseRecords.begin(), databaseRecords.end(),
            [](const auto& left, const auto& right) noexcept {
                return left.m_name < right.m_name;
            });
    return databaseRecords;
}

DatabasePtr Instance::findDatabaseChecked(const std::string& databaseName)
{
    if (auto database = findDatabase(databaseName)) return database;
    throwDatabaseError(IOManagerMessageId::kErrorDatabaseDoesNotExist, databaseName);
}

DatabasePtr Instance::findDatabase(const std::string& databaseName)
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_databaseRegistry.byName();
    const auto it = index.find(databaseName);
    if (it == index.end()) return nullptr;

    const auto itdb = m_databases.find(it->m_id);
    if (itdb != m_databases.end()) return itdb->second;

    auto database = std::make_shared<UserDatabase>(*this, *it);
    m_databases.emplace(database->getId(), database);
    return database;
}

DatabasePtr Instance::createDatabase(std::string&& name, const std::string& cipherId,
        BinaryValue&& cipherKey, std::optional<std::string>&& description,
        std::uint32_t maxTableCount, const std::optional<Uuid>& uuid, bool dataDirectoryMustExist,
        std::uint32_t currentUserId)
{
    const auto user = findUserChecked(currentUserId);

    // User must have permission to create database
    user->checkHasPermissions(
            0, DatabaseObjectType::kDatabase, 0, getSinglePermissionMask(PermissionType::kCreate));

    DatabasePtr database;
    {
        std::lock_guard lock(m_cacheMutex);

        if (m_databaseRegistry.size() >= m_maxDatabases)
            throwDatabaseError(IOManagerMessageId::kErrorTooManyDatabases);

        if (m_databaseRegistry.byName().count(name) > 0)
            throwDatabaseError(IOManagerMessageId::kErrorDatabaseAlreadyExists, name);

        const auto& databasesByUuid = m_databaseRegistry.byUuid();
        Uuid realUuid;
        if (uuid.has_value()) {
            if (databasesByUuid.count(*uuid) > 0)
                throwDatabaseError(IOManagerMessageId::kErrorDatabaseUuidAlreadyExists, *uuid);
            realUuid = *uuid;
        } else {
            auto t = std::time(nullptr);
            do {
                realUuid = Database::computeDatabaseUuid(name.c_str(), t++);
            } while (databasesByUuid.count(realUuid) > 0);
        }

        if (maxTableCount == 0) maxTableCount = m_maxTableCountPerDatabase;

        database = std::make_shared<UserDatabase>(*this, realUuid, std::move(name), cipherId,
                std::move(cipherKey), std::move(description), maxTableCount,
                dataDirectoryMustExist);
        m_databaseRegistry.emplace(*database);

        const TransactionParameters tp(
                currentUserId, m_systemDatabase->generateNextTransactionId());
        m_systemDatabase->recordDatabase(*database, tp);
        m_databases.emplace(database->getId(), database);
    }

    // Auto-grant permissions to database creator
    const auto databasePermissions =
            removeMultiplePermissionsFromMask<PermissionType::kCreate, PermissionType::kAttach>(
                    *Instance::getAllObjectTypePermissions(DatabaseObjectType::kDatabase));
    grantObjectPermissionsToUser(*user, 0, DatabaseObjectType::kDatabase, database->getId(),
            databasePermissions, true, User::kSuperUserId);
    grantObjectPermissionsToUser(*user, database->getId(), DatabaseObjectType::kTable, 0,
            *Instance::getAllObjectTypePermissions(DatabaseObjectType::kTable), true,
            User::kSuperUserId);

    return database;
}

bool Instance::dropDatabase(
        const std::string& name, bool databaseMustExist, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto database = findDatabase(name);
    if (!database) {
        if (databaseMustExist)
            throwDatabaseError(IOManagerMessageId::kErrorDatabaseDoesNotExist, name);
        else
            return false;
    }

    const auto user = findUserChecked(currentUserId);

    // User must have permission to drop this ot any database
    if (!user->hasPermissions(
                0, DatabaseObjectType::kDatabase, database->getId(), kDropPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kDatabase, 0, kDropPermissionMask)) {
        if (databaseMustExist)
            throwDatabaseError(IOManagerMessageId::kErrorDatabaseDoesNotExist, name);
        else
            return false;
    }

    const auto id = database->getId();
    if (id == Database::kSystemDatabaseId)
        throwDatabaseError(IOManagerMessageId::kErrorCannotDropSystemDatabase);

    // If any connection now uses the database
    if (database->isUsed())
        throwDatabaseError(IOManagerMessageId::kErrorCannotDropUsedDatabase, database->getName());

    const auto uuid = database->getUuid();
    const auto dataDir = database->getDataDir();
    m_databases.erase(id);
    m_databaseRegistry.byId().erase(id);
    m_systemDatabase->deleteDatabase(id, currentUserId);
    revokeAllObjectPermissionsFromAllUsers(
            0, DatabaseObjectType::kDatabase, database->getId(), User::kSuperUserId);
    system_error_code ec;
    if (fs::remove_all(dataDir, ec) == static_cast<std::uintmax_t>(-1)) {
        throwDatabaseError(IOManagerMessageId::kWarningCannotRemoveDatabaseDataDirectory,
                database->getName(), uuid, ec.value(), ec.message());
    }
    return true;
}

std::uint32_t Instance::generateNextDatabaseId(bool system)
{
    return m_systemDatabase ? m_systemDatabase->generateNextDatabaseId(system) : 1;
}

// --- internals ---

DatabasePtr Instance::findDatabaseUnlocked(const std::string& databaseName)
{
    const auto& databasesByName = m_databaseRegistry.byName();
    const auto it = databasesByName.find(databaseName);
    if (it == databasesByName.end()) return nullptr;

    const auto itdb = m_databases.find(it->m_id);
    if (itdb != m_databases.end()) return itdb->second;

    auto database = std::make_shared<UserDatabase>(*this, *it);
    m_databases.emplace(database->getId(), database);
    return database;
}

}  // namespace siodb::iomgr::dbengine
