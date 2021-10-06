// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "AuthenticationResult.h"
#include "ClientSession.h"
#include "DatabasePtr.h"
#include "InstancePtr.h"
#include "UpdateUserAccessKeyParameters.h"
#include "UpdateUserParameters.h"
#include "UpdateUserTokenParameters.h"
#include "UserAccessKeyPtr.h"
#include "UserPtr.h"
#include "UserTokenPtr.h"
#include "reg/DatabaseRegistry.h"
#include "reg/UserRegistry.h"

// Common project headers
#include <siodb/common/utils/FDGuard.h>
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/CipherContextPtr.h>
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/CipherPtr.h>

// STL headers
#include <mutex>
#include <optional>

namespace siodb::config {

struct SiodbOptions;

}  // namespace siodb::config

namespace siodb::iomgr::dbengine {

class SystemDatabase;

/** DBMS instance */
class Instance {
public:
    /**
     * Initializes object of class Instance.
     * Reads existing on-disk instance definition or creates new ones.
     * @param options Instance options.
     */
    explicit Instance(const config::SiodbOptions& options);

    DECLARE_NONCOPYABLE(Instance);

    /**
     * Returns instance name.
     * @return instance name.
     */
    const auto& getName() const noexcept
    {
        return m_name;
    }

    /**
     * Returns instance UUID.
     * @return Instance UUID.
     */
    const Uuid& getUuid() const noexcept
    {
        return m_uuid;
    }

    /**
     * Returns instance data directory path.
     * @return data directory path.
     */
    const auto& getDataDir() const noexcept
    {
        return m_dataDir;
    }

    /**
     * Returns display name of the instance.
     * @return Display name.
     */
    std::string makeDisplayName() const;

    /**
     * Returns display code of the instance.
     * @return Display code.
     */
    std::string makeDisplayCode() const
    {
        return boost::uuids::to_string(m_uuid);
    }

    /**
     * Returns block cache capacity.
     * @return Block cache capacity.
     */
    auto getBlockCacheCapacity() const noexcept
    {
        return m_blockCacheCapacity;
    }

    /**
     * Returns default database cipher.
     * @return Default database cipher.
     */
    const auto& getDefaultDatabaseCipherId() const noexcept
    {
        return m_defaultDatabaseCipherId;
    }

    /**
     * Returns indication that it is allowed to create user tables in the system database.
     * @return true if it is allowed to create user tables in the system database,
     *         false otherwise.
     */
    bool canCreateUserTablesInSystemDatabase() const noexcept
    {
        return m_allowCreatingUserTablesInSystemDatabase;
    }

    /**
     * Returns system database object.
     * @return System database.
     */
    auto& getSystemDatabase() noexcept
    {
        return *m_systemDatabase;
    }

    /**
     * Retuns number of known databases.
     * @return Number of databases.
     */
    std::size_t getDatabaseCount() const;

    /**
     * Returns list of database records which can be listed by current user, ordered by name.
     * @param currentUserId Current user ID.
     * @return List of database records.
     */
    std::vector<DatabaseRecord> getDatabaseRecordsOrderedByName(std::uint32_t currentUserId);

    /**
     * Returns existing database object.
     * @param databaseName Database name.
     * @return Corresponding database object.
     * @throw DatabaseError if database doesn't exist
     */
    DatabasePtr findDatabaseChecked(const std::string& databaseName);

    /**
     * Returns existing database object.
     * @param databaseId Database ID.
     * @return Corresponding database object.
     * @throw DatabaseError if database doesn't exist
     */
    DatabasePtr findDatabaseChecked(const std::uint32_t databaseId);

    /**
     * Returns existing database object.
     * @param databaseName Database name.
     * @return Corresponding database object or nullptr if it doesn't exist.
     */
    DatabasePtr findDatabase(const std::string& databaseName);

    /**
     * Returns existing database object.
     * @param databaseId Database ID.
     * @return Corresponding database object or nullptr if it doesn't exist.
     */
    DatabasePtr findDatabase(const std::uint32_t databaseId);

    /**
     * Creates new database object and writes all necessary on-disk data structures.
     * @param name Database name.
     * @param cipherId Cipher ID used for encryption of this database.
     * @param cipherKey Key used for encryption of this database.
     * @param description Database description.
     * @param maxTableCount Maximum table count.
     * @param uuid Optional explicit database UUID.
     * @param dataDirectoryMustExist Indicates that data directory must exist.
     * @param currentUserId Current user ID.
     * @return New database object.
     * @throw DatabaseError if some error has occurrred.
     */
    DatabasePtr createDatabase(std::string&& name, const std::string& cipherId,
            BinaryValue&& cipherKey, std::optional<std::string>&& description,
            std::uint32_t maxTableCount, const std::optional<Uuid>& uuid,
            bool dataDirectoryMustExist, std::uint32_t currentUserId);

    /**
     * Deletes existing database.
     * @param name Database name.
     * @param databaseMustExist Indicates that database must exist.
     * @param currentUserId Current user ID.
     * @return true if database dropped, false if database didn't exists
     *         and databaseMustExists is false.
     * @throw DatabaseError if some error has occurrred.
     */
    bool dropDatabase(const std::string& name, bool databaseMustExist, std::uint32_t currentIserId);

    /**
     * Returns existing user object.
     * @param userName User name.
     * @return Corresponding user object.
     * @throw DatabaseError if user doesn't exist
     */
    UserPtr findUserChecked(const std::string& userName);

    /**
     * Returns existing user object.
     * @param userId User ID.
     * @return Corresponding database object.
     * @throw DatabaseError if user doesn't exist
     */
    UserPtr findUserChecked(std::uint32_t userId);

    /**
     * Creates new user.
     * @param name User name.
     * @param realName Real name.
     * @param description User description.
     * @param active Initial state of user.
     * @param currentUserId Current user ID.
     * @return User ID.
     * @throw DatabaseError if some error has occurrred.
     */
    std::uint32_t createUser(const std::string& name, const std::optional<std::string>& realName,
            const std::optional<std::string>& description, bool active,
            std::uint32_t currentUserId);

    /**
     * Deletes existing user.
     * @param name User name.
     * @param userMustExist Indicates that user must exist.
     * @param currentUserId Current user ID.
     * @throw DatabaseError if some error has occurrred.
     */
    void dropUser(const std::string& name, bool userMustExist, std::uint32_t currentUserId);

    /**
     * Updates existing user.
     * @param name User name.
     * @param params Update parameters.
     * @param currentUserId Current user ID.
     * @throw DatabaseError if some error has occurrred.
     */
    void updateUser(const std::string& name, const UpdateUserParameters& params,
            std::uint32_t currentUserId);

    /**
     * Get all permissions for particular object type.
     * @param objectType Object type.
     * @return Bitmask of all permission types are applicable for the given object type,
     *         or empty optional if object type is not supported.
     */
    static std::optional<std::uint64_t> getAllObjectTypePermissions(
            DatabaseObjectType objectType) noexcept;

    /**
     * Grants permissions for the specified table to the specified user.
     * @param userName User name to grant permissions to.
     * @param databaseName Database name to locate the table in.
     * @param tableName Table name to grant permissions for.
     * @param permissions Bitmask of permissions.
     * @param withGrantOption Inidicates that user will be able
     *                        to grant same persmissions to other users.
     * @param currentUserId Current user ID.
     */
    void grantTablePermissionsToUser(const std::string& userName, const std::string& databaseName,
            const std::string& tableName, std::uint64_t permissions, bool withGrantOption,
            std::uint32_t currentUserId);

    /**
     * Performs actual changes in the permission registry in order to grant permissions.
     * @param userId User ID of the user to grant permissions to.
     * @param databaseId Database ID to grant permissions for.
     *                   Zero value may have special meaning "whole instance".
     * @param objectType Object type to grant permissions for.
     * @param objectId Object ID to grant permissions for.
     *                 Zero value may have special meaning "all objects".
     * @param permissions Bitmask of permissions.
     * @param withGrantOption Inidication that user will be able
     *                        to grant same persmissions to others.
     * @param currentUserId Current user ID.
     */
    void grantObjectPermissionsToUser(std::uint32_t userId, std::uint32_t databaseId,
            DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
            bool withGrantOption, std::uint32_t currentUserId);

    /**
     * Performs actual changes in the permission registry in order to grant permissions.
     * @param user User to grant permissions to.
     * @param databaseId Database ID to grant permissions for.
     *                   Zero value may have special meaning "whole instance".
     * @param objectType Object type to grant permissions for.
     * @param objectId Object ID to grant permissions for.
     *                 Zero value may have special meaning "all objects".
     * @param permissions Bitmask of permissions.
     * @param withGrantOption Inidication that user will be able
     *                        to grant same persmissions to others.
     * @param currentUserId Current user ID.
     */
    void grantObjectPermissionsToUser(User& user, std::uint32_t databaseId,
            DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
            bool withGrantOption, std::uint32_t currentUserId);

    /**
     * Revokes permissions from the specified user.
     * @param userName User name to grant permissions to.
     * @param databaseName Database name to locate the table in.
     * @param tableName Table name to grant permissions for.
     * @param permissions Bitmask of permissions.
     * @param currentUserId Current user ID.
     */
    void revokeTablePermissionsFromUser(const std::string& userName,
            const std::string& databaseName, const std::string& tableName,
            std::uint64_t permissions, std::uint32_t currentUserId);

    /**
     * Performs actual changes in the permission registry in order to revoke permissions.
     * @param databaseId Database ID to revoke permissions for.
     *                   Zero value may have special meaning "whole instance".
     * @param objectType Object type to revoke permissions for.
     * @param objectId Object ID to revoke permissions for.
     *                 Zero value may have special meaning "all objects".
     * @param currentUserId Current user ID.
     */
    void revokeAllObjectPermissionsFromAllUsers(std::uint32_t databaseId,
            DatabaseObjectType objectType, std::uint64_t objectId, std::uint32_t currentUserId);

    /**
     * Performs actual changes in the permission registry in order to revoke permissions.
     * @param user User to revoke permissions from.
     * @param databaseId Database ID to revoke permissions for.
     *                   Zero value may have special meaning "whole instance".
     * @param objectType Object type to revoke permissions for.
     * @param objectId Object ID to revoke permissions for.
     *                 Zero value may have special meaning "all objects".
     * @param permissions Bitmask of permissions.
     * @param currentUserId Current user ID.
     */
    void revokeObjectPermissionsFromUser(User& user, std::uint32_t databaseId,
            DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
            std::uint32_t currentUserId);

    /**
     * Returns existing user access key object.
     * @param userName User name.
     * @param userAccessKeyName User access key name.
     * @return Pair of user access key object and corresponding user object.
     * @throw DatabaseError if user access key doesn't exist
     */
    std::pair<UserPtr, UserAccessKeyPtr> findUserAccessKeyChecked(
            const std::string& userName, const std::string& userAccessKeyName);

    /**
     * Returns existing user access key object.
     * @param userAccessKeyId User access key ID.
     * @return Pair of user access key object and corresponding user object.
     * @throw DatabaseError if user access key doesn't exist
     */
    std::pair<UserPtr, UserAccessKeyPtr> findUserAccessKeyChecked(std::uint64_t userAccessKeyId);

    /**
     * Creates new user access key.
     * @param userName User name.
     * @param keyName User access key name.
     * @param text User access key text.
     * @param description User access key description.
     * @param active Initial state
     * @param currentUserId Current user ID.
     * @return User access key ID.
     * @throw DatabaseError if some error has occurrred.
     */
    std::uint64_t createUserAccessKey(const std::string& userName, const std::string& keyName,
            const std::string& text, const std::optional<std::string>& description, bool active,
            std::uint32_t currentUserId);

    /**
     * Deletes existing user access key.
     * @param userName User name.
     * @param keyName Access key name.
     * @param currentUserId Current user ID.
     * @param mustExist Indicates that access key must exist.
     * @throw DatabaseError if some error has occurrred.
     */
    void dropUserAccessKey(const std::string& userName, const std::string& keyName, bool mustExist,
            std::uint32_t currentUserId);

    /**
     * Updates user access key.
     * @param userName User name.
     * @param keyName Access key name.
     * @param params Update parameters.
     * @param currentUserId Current user ID.
     * @throw DatabaseError if some error has occurrred.
     */
    void updateUserAccessKey(const std::string& userName, const std::string& keyName,
            const UpdateUserAccessKeyParameters& params, std::uint32_t currentUserId);

    /**
     * Returns existing user token object.
     * @param userName User name.
     * @param userTokenName User token name.
     * @return Pair of user token object and corresponding user object.
     * @throw DatabaseError if user token doesn't exist
     */
    std::pair<UserPtr, UserTokenPtr> findUserTokenChecked(
            const std::string& userName, const std::string& userTokenName);

    /**
     * Returns existing user token object.
     * @param userTokenId User token ID.
     * @return Pair of user token object and corresponding user object.
     * @throw DatabaseError if user token doesn't exist
     */
    std::pair<UserPtr, UserTokenPtr> findUserTokenChecked(std::uint64_t userTokenId);

    /**
     * Creates new user token.
     * @param userName User name.
     * @param tokenName User token name.
     * @param value User token value. If this is nullptr, value is generated.
     * @param description User access key description.
     * @param expirationTimestamp User token expiration timestamp.
     * @param currentUserId Current user ID.
     * @return Pair of user token ID and token value if it was generated.
     * @throw DatabaseError if some error has occurrred.
     */
    std::pair<std::uint64_t, BinaryValue> createUserToken(const std::string& userName,
            const std::string& tokenName, const std::optional<BinaryValue>& value,
            const std::optional<std::string>& description,
            const std::optional<std::time_t>& expirationTimestamp, std::uint32_t currentUserId);

    /**
     * Deletes existing user token.
     * @param userName User name.
     * @param tokenName Token name.
     * @param currentUserId Current user ID.
     * @param mustExist Indicates that access key must exist.
     * @throw DatabaseError if some error has occurrred.
     */
    void dropUserToken(const std::string& userName, const std::string& tokenName, bool mustExist,
            std::uint32_t currentUserId);

    /**
     * Updates user token.
     * @param userName User name.
     * @param tokenName Token name.
     * @param params Update parameters.
     * @param currentUserId Current user ID.
     * @throw DatabaseError if some error has occurrred.
     */
    void updateUserToken(const std::string& userName, const std::string& tokenName,
            const UpdateUserTokenParameters& params, std::uint32_t currentUserId);

    /**
     * Checks user token.
     * @param userName User name.
     * @param tokenName Token name.
     * @param tokenValue Token value.
     * @param currentUserId Current user ID.
     * @throw DatabaseError if token mismatched.
     */
    void checkUserToken(const std::string& userName, const std::string& tokenName,
            const BinaryValue& tokenValue, std::uint32_t currentUserId);

    /**
     * Begins user Authentication.
     * @param userName User name.
     * @throw DatabaseError if some error has occurrred.
     */
    void beginUserAuthentication(const std::string& userName);

    /**
     * Authenticates user.
     * @param userName User name.
     * @param signature Signed challenge with user private key.
     * @param challenge Random challenge.
     * @return Pair (user ID, new session UUID).
     * @throw DatabaseError if some error has occurrred.
     */
    AuthenticationResult authenticateUser(const std::string& userName, const std::string& signature,
            const std::string& challenge);

    /**
     * Authenticates user.
     * @param userName User name.
     * @param token User token.
     * @return User ID.
     * @throw DatabaseError if some error has occurrred.
     */
    std::uint32_t authenticateUser(const std::string& userName, const std::string& token);

    /**
     * Begins new session.
     * @return New session UUID.
     */
    Uuid beginSession();

    /**
     * Closes session.
     * @param sessionUuid Session UUID.
     * @throw DatabaseError if session does not exist.
     */
    void endSession(const Uuid& sessionUuid);

    /**
     * Generates next database ID.
     * @param system Indicates that resulting database ID must be in the system range.
     * @return Next database ID.
     */
    std::uint32_t generateNextDatabaseId(bool system);

    /**
     * Encrypts data with master encryption.
     * @param data Data buffer address.
     * @param size Data size.
     * @return Buffer with encrypted data. Buffer size is multiple of the master cipher block size.
     */
    BinaryValue encryptWithMasterEncryption(const void* data, std::size_t size) const;

    /**
     * Decrypts data with master encryption.
     * @param data Data buffer address.
     * @param size Data size. Must be multiple of the master cipher block size.
     * @return Buffer with encrypted data.
     */
    BinaryValue decryptWithMasterEncryption(const void* data, std::size_t size) const;

private:
    /** Creates new instance data */
    void createInstanceData();

    /** Loads existing instance data */
    void loadInstanceData();

    /** Ensures data directory exists */
    void ensureDataDir() const;

    /** Creates system database */
    void createSystemDatabase();

    /** Loads system database */
    void loadSystemDatabase();

    /** Loads users */
    void loadUsers();

    /** Creates superuser */
    void createSuperUser();

    /** Records superuser into the system database. */
    void recordSuperUser();

    /**
     * Loads master encryption key.
     * @param keyPath Key path.
     * @return Master encryption key.
     * @throw DatabaseError In the following cases:
     *                      - If file doesn't not exist or could not be open.
     *                      - If Read error happens.
     *                      - If key data is invalid.
     */
    BinaryValue loadMasterCipherKey(const std::string& keyPath) const;

    /**
     * Loads initial super-user access key.
     * @return Access key text
     * @throw DatabaseError If file does not exist or could not be read
     */
    std::string loadSuperUserInitialAccessKey() const;

    /**
     * Returns existing database object. Does not acquire database cache access synchronization lock.
     * @param databaseName Database name.
     * @return Corresponding database object or nullptr if it doesn't exist.
     */
    DatabasePtr findDatabaseUnlocked(const std::string& databaseName);

    /**
     * Returns existing database object. Does not acquire database cache access synchronization lock.
     * @param databaseId Database ID.
     * @return Corresponding database object or nullptr if it doesn't exist.
     */
    DatabasePtr findDatabaseUnlocked(std::uint32_t databaseId);

    /** Checks instance data consistency */
    void checkDataConsistency();

    /**
     * Opens or creates new metadata file.
     * @return Metadata file descriptor.
     */
    int openMetadataFile() const;

    /**
     * Loads existing instance metadata. Doesn't perform metadata access synchronization.
     * @throw DatabaseError if metadata could not be loaded.
     */
    void loadMetadata();

    /**
     * Saves instance metadata. Doesn't perform metadata access synchronization.
     * @throw DatabaseError if metadata could not be loaded.
     */
    void saveMetadata() const;

    /**
     * Constructs instance metadata file path.
     * @return Instance metadata file path.
     */
    std::string makeMetadataFilePath() const;

    /**
     * Serializes instance metadata into memory buffer.
     * @param buffer Memory buffer address.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serializeMetadata(std::uint8_t* buffer) const noexcept;

    /**
     * De-serializes instance metadata from memory buffer.
     * @param buffer Memory buffer address.
     * @return Address of byte after last read byte on success, nullptr otherwise.
     */
    const std::uint8_t* deserializeMetadata(const std::uint8_t* buffer) noexcept;

    /** Creates instance initialization success indicator file. */
    void createInitializationFlagFile() const;

    /** Checks instance initialization success indicator file. */
    void checkInitializationFlagFile() const;

    /**
     * Returns existing user object. Assumes lock already acquired.
     * @param userName User name.
     * @return Corresponding user object.
     * @throw DatabaseError if user not found.
     */
    UserPtr findUserCheckedUnlocked(const std::string& userName);

    /**
     * Returns existing user object. Assumes lock already acquired.
     * @param userId User ID.
     * @return Corresponding database object.
     * @throw DatabaseError if user not found.
     */
    UserPtr findUserCheckedUnlocked(std::uint32_t userId);

    /**
     * Returns existing user object. Assumes lock already acquired.
     * @param userName User name.
     * @return Corresponding user object or nullptr if it doesn't exist.
     */
    UserPtr findUserUnlocked(const std::string& userName);

    /**
     * Returns existing user object. Assumes lock already acquired.
     * @param userId User ID.
     * @return Corresponding database object or nullptr if it doesn't exist.
     */
    UserPtr findUserUnlocked(std::uint32_t userId);

    /**
     * Returns cached user object or creates new one.  Assumes lock already acquired.
     * @param userRecord User record.
     * @return Corresponding user object.
     */
    UserPtr findUserUnlocked(const UserRecord& userRecord);

    /**
     * Returns existing user access key object. Assumes lock already acquired.
     * @param userName User name.
     * @param userAccessKeyName User access key name.
     * @return Pair of user access key object and corresponding user object.
     * @throw DatabaseError if user access key doesn't exist.
     */
    std::pair<UserPtr, UserAccessKeyPtr> findUserAccessKeyCheckedUnlocked(
            const std::string& userName, const std::string& userAccessKeyName);

    /**
     * Returns existing user access key object. Assumes lock already acquired.
     * @param userAccessKeyId User access key ID.
     * @return Pair of user access key object and corresponding user object.
     * @throw DatabaseError if user access key doesn't exist.
     */
    std::pair<UserPtr, UserAccessKeyPtr> findUserAccessKeyCheckedUnlocked(
            std::uint64_t userAccessKeyId);

    /**
     * Returns existing user token object. Assumes lock already acquired.
     * @param userName User name.
     * @param userTokenName User token name.
     * @return Pair of user token object and corresponding user object.
     * @throw DatabaseError if user access key doesn't exist.
     */
    std::pair<UserPtr, UserTokenPtr> findUserTokenCheckedUnlocked(
            const std::string& userName, const std::string& userTokenName);

    /**
     * Returns existing user token object. Assumes lock already acquired.
     * @param userTokenId User token ID.
     * @return Pair of user token object and corresponding user object.
     * @throw DatabaseError if user access key doesn't exist.
     */
    std::pair<UserPtr, UserTokenPtr> findUserTokenCheckedUnlocked(std::uint64_t userTokenId);

    /**
     * Performs actual changes in the permission registry in order to grant permissions.
     * @param user User to grant permissions to.
     * @param databaseId Database ID to grant permissions for.
     *                   Zero value may have special meaning "whole instance".
     * @param objectType Object type to grant permissions for.
     * @param objectId Object ID to grant permissions for.
     *                 Zero value may have special meaning "all objects".
     * @param permissions Bitmask of permissions.
     * @param withGrantOption Inidication that user will be able
     *                        to grant same persmissions to others.
     * @param currentUserId Current user ID.
     */
    void grantObjectPermissionsToUserUnlocked(User& user, std::uint32_t databaseId,
            DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
            bool withGrantOption, std::uint32_t currentUserId);

    /**
     * Performs actual changes in the permission registry in order to revoke permissions.
     * @param user User to revoke permissions from.
     * @param databaseId Database ID to revoke permissions for.
     *                   Zero value may have special meaning "whole instance".
     * @param objectType Object type to revoke permissions for.
     * @param objectId Object ID to revoke permissions for.
     *                 Zero value may have special meaning "all objects".
     * @param permissions Bitmask of permissions.
     * @param currentUserId Current user ID.
     */
    void revokeObjectPermissionsFromUserUnlocked(User& user, std::uint32_t databaseId,
            DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
            std::uint32_t currentUserId);

    /**
     * Checks applicability of permission to the particular object type.
     * @param objectType Object type.
     * @param permissions Permission bitmask.
     * @throws DatabaseError if at least one permission types is not applicable
     *         for the given object type.
     */
    static void validatePermissions(DatabaseObjectType objectType, std::uint64_t permissions);

    /**
     * Checks applicability of permission to the particular object type.
     * @param objectType Object type.
     * @param permissions Permission bitmask.
     * @return true if all permission types are applicable for the given object type,
     *         false if at least one permission type is not applicable.
     */
    static bool isValidPermissions(
            DatabaseObjectType objectType, std::uint64_t permissions) noexcept;

private:
    /** Instance identifier */
    const Uuid m_uuid;

    /** Instance name */
    const std::string m_name;

    /** Instance data directory */
    const std::string m_dataDir;

    /** Default database cipher */
    const std::string m_defaultDatabaseCipher;

    /** System database cipher */
    const std::string m_defaultDatabaseCipherId;

    /** Cipher object */
    crypto::CipherPtr m_masterCipher;

    /** Master encryption key */
    BinaryValue m_masterCipherKey;

    /** Master encryption context */
    crypto::CipherContextPtr m_masterEncryptionContext;

    /** Master decryption context */
    crypto::CipherContextPtr m_masterDecryptionContext;

    /** System database cipher */
    const std::string m_systemDatabaseCipherId;

    /** Superuser's initial access key */
    const std::string m_superUserInitialAccessKey;

    /** Maximum number of users */
    const std::size_t m_maxUsers;

    /** Maximum number of databases */
    const std::size_t m_maxDatabases;

    /** Maximum tables per database */
    const std::size_t m_maxTableCountPerDatabase;

    /** Block cache capacity */
    const std::size_t m_blockCacheCapacity;

    /** Metadata access synchronization object */
    mutable std::mutex m_mutex;

    /** Cache and registries access synchronization object */
    mutable std::mutex m_cacheMutex;

    /** User registry. Contains information about all known users. */
    UserRegistry m_userRegistry;

    /** User objects. */
    std::unordered_map<std::uint32_t, UserPtr> m_users;

    /** Mapping from user access key IDs to user IDs. */
    std::unordered_map<std::uint64_t, std::uint32_t> m_userAccessKeyIdToUserId;

    /** Mapping from user token IDs to user IDs. */
    std::unordered_map<std::uint64_t, std::uint32_t> m_userTokenIdToUserId;

    /** Database registry. Contains information about all known databases. */
    DatabaseRegistry m_databaseRegistry;

    /** Database objects. */
    std::unordered_map<std::uint32_t, DatabasePtr> m_databases;

    /** Superuser */
    UserPtr m_superUser;

    /** System database */
    std::shared_ptr<SystemDatabase> m_systemDatabase;

    /* Metadata file descriptor */
    FDGuard m_metadataFile;

    /** Flag which allows creating user tables in the system database */
    const bool m_allowCreatingUserTablesInSystemDatabase;

    /** Session mutex */
    mutable std::mutex m_sessionMutex;

    /** Session UUID generator */
    boost::uuids::random_generator m_sessionUuidGenerator;

    /** Active sessions */
    std::unordered_map<Uuid, std::shared_ptr<ClientSession>> m_activeSessions;

    /** Allowed permission map */
    static const std::unordered_map<DatabaseObjectType, std::uint64_t> s_allowedPermissions;

    /** Metadata version */
    static constexpr std::uint32_t kCurrentMetadataVersion = 1;

    /** Serialized metadata size */
    static constexpr std::size_t kSerializedMetadataSize = sizeof(kCurrentMetadataVersion);

    /** Metadata file name */
    static constexpr const char* kMetadataFileName = "instance_metadata";

    /** Generated token length */
    static constexpr std::size_t kGeneratedTokenLength = 64;

    /** All premissions constant for REVOKE */
    static constexpr std::uint64_t kAllPermissionsForRevoke =
            std::numeric_limits<std::uint64_t>::max();
};

}  // namespace siodb::iomgr::dbengine
