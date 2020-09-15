// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Database.h"

namespace siodb::iomgr::dbengine {

/** System database */
class SystemDatabase : public Database, public UserIdGenerator {
public:
    /**
     * Initializes object of class SystemDatabase for new instance.
     * @param instance Instance object.
     * @param cipherId Cipher used for encryption of this database.
     * @param cipherKey Cipher key.
     */
    SystemDatabase(Instance& instance, const std::string& cipherId, BinaryValue&& cipherKey);

    /**
     * Initializes object of class SystemDatabase for existing instance.
     * @param instance Instance object.
     * @param cipherId Cipher used for encryption of this database.
     * @param cipherKey Key used for encryption of this database.
     */
    SystemDatabase(Instance& instance, const std::string& cipherId);

    /**
     * Gives indication that this is system database.
     * @return true if this is system database, false otherwise.
     */
    bool isSystemDatabase() const noexcept override final;

    /**
     * Reads list of known users from the system table.
     * @param[out] userRegistry User registry.
     */
    void readAllUsers(UserRegistry& userRegistry);

    /**
     * Reads list of known databases from the system table.
     * @param[out] databaseRegistry Database registry.
     */
    void readAllDatabases(DatabaseRegistry& databaseRegistry);

    /**
     * Generates new unique user ID.
     * @return New user ID.
     */
    virtual std::uint32_t generateNextUserId() override;

    /**
     * Generates new unique user key ID.
     * @return New user ID.
     */
    std::uint64_t generateNextUserAccessKeyId();

    /**
     * Generates new unique user token ID.
     * @return New user tokenID.
     */
    std::uint64_t generateNextUserTokenId();

    /**
     * Generates new unique database ID.
     * @param system Indicates that database ID should be in the system range
     * @return New database ID.
     */
    std::uint32_t generateNextDatabaseId(bool system);

    /**
     * Generates new unique user permission ID.
     * @return New user ID.
     */
    std::uint64_t generateNextUserPermissionId();

    /**
     * Records user into the appropriate system table.
     * @param user An user.
     * @param tp Transaction parameters.
     */
    void recordUser(const User& database, const TransactionParameters& tp);

    /**
     * Records user access key into the appropriate system table.
     * @param accessKey An access key.
     * @param tp Transaction parameters.
     */
    void recordUserAccessKey(const UserAccessKey& accessKey, const TransactionParameters& tp);

    /**
     * Records user token into the appropriate system table.
     * @param token A token.
     * @param tp Transaction parameters.
     */
    void recordUserToken(const UserToken& token, const TransactionParameters& tp);

    /**
     * Records database into the appropriate system table.
     * @param database A database.
     * @param tp Transaction parameters.
     */
    void recordDatabase(const Database& database, const TransactionParameters& tp);

    /**
     * Records user permission into the appropriate system table.
     * @param permission A permission object.
     * @param tp Transaction parameters.
     */
    void recordUserPermission(const UserPermission& permission, const TransactionParameters& tp);

    /**
     * Deletes database record.
     * @param databaseId Database ID.
     * @param currentUserId Current user ID.
     */
    void deleteDatabase(std::uint32_t databaseId, std::uint32_t currentUserId);

    /**
     * Deletes user record.
     * @param userId User ID.
     * @param currentUserId Current user ID.
     */
    void deleteUser(std::uint32_t userId, std::uint32_t currentUserId);

    /**
     * Deletes user access key record.
     * @param accessKeyId User access key ID.
     * @param currentUserId Current user ID.
     */
    void deleteUserAccessKey(std::uint64_t accessKeyId, std::uint32_t currentUserId);

    /**
     * Deletes user token record.
     * @param tokenId User token ID.
     * @param currentUserId Current user ID.
     */
    void deleteUserToken(std::uint64_t tokenId, std::uint32_t currentUserId);

    /**
     * Updates existing user.
     * @param userId User ID.
     * @param params Update parameters.
     * @param currentUserId Current user ID.
     * @throw DatabaseError if some error has occurrred.
     */
    void updateUser(
            std::uint32_t userId, const UpdateUserParameters& params, std::uint32_t currentUserId);

    /**
     * Updates user access key.
     * @param accessKeyId User access key ID.
     * @param params Update parameters.
     * @param currentUserId Current user ID.
     */
    void updateUserAccessKey(std::uint64_t accessKeyId, const UpdateUserAccessKeyParameters& params,
            std::uint32_t currentUserId);

    /**
     * Updates user token.
     * @param accessKeyId User token ID.
     * @param params Update parameters.
     * @param currentUserId Current user ID.
     */
    void updateUserToken(std::uint64_t tokenId, const UpdateUserTokenParameters& params,
            std::uint32_t currentUserId);

private:
    /** User access key registry map by user ID. */
    using UserAccessKeyRegistries = std::unordered_map<std::uint32_t, UserAccessKeyRegistry>;

    /** User token registry map by user ID. */
    using UserTokenRegistries = std::unordered_map<std::uint32_t, UserTokenRegistry>;

private:
    /**
     * Reads list of known user access keys from the system table.
     * @return Collection of the user access key registries.
     */
    UserAccessKeyRegistries readAllUserAccessKeys();

    /**
     * Reads list of known user tokens from the system table.
     * @return Collection of the user token registries.
     */
    UserTokenRegistries readAllUserTokens();

private:
    /** Table SYS_USERS */
    TablePtr m_sysUsersTable;

    /** Table SYS_USER_KEYS */
    TablePtr m_sysUserAccessKeysTable;

    /** Table SYS_USER_TOKENS */
    TablePtr m_sysUserTokensTable;

    /** Table SYS_DATABASES */
    TablePtr m_sysDatabasesTable;

    /** Table SYS_USER_PERMISSIONS */
    TablePtr m_sysUserPermissionsTable;
};

}  // namespace siodb::iomgr::dbengine
