// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "AuthenticationResult.h"
#include "DatabaseCache.h"
#include "InstancePtr.h"
#include "UpdateUserAccessKeyParameters.h"
#include "UpdateUserParameters.h"
#include "UpdateUserTokenParameters.h"
#include "UserCache.h"
#include "reg/DatabaseRegistry.h"
#include "reg/UserRegistry.h"
#include "../main/ClientSession.h"

// Common project headers
#include <siodb/common/utils/FdGuard.h>
#include <siodb/common/utils/HelperMacros.h>

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
     * Return table cache capacity.
     * @return Table cache capacity.
     */
    auto getTableCacheCapacity() const noexcept
    {
        return m_tableCacheCapacity;
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
    std::size_t getDatbaseCount() const;

    /**
     * Returns list of database records ordered by name.
     * @return List of database records.
     */
    std::vector<DatabaseRecord> getDatabaseRecordsOrderedByName() const;

    /**
     * Returns list of databasesordered by name.
     * @return List of databasess.
     */
    std::vector<std::string> getDatabaseNames() const;

    /**
     * Returns existing database object.
     * @param databaseName database name.
     * @return Corresponding database object.
     * @throw DatabaseError if database doesn't exist
     */
    DatabasePtr findDatabaseChecked(const std::string& databaseName);

    /**
     * Returns existing database object.
     * @param databaseName database name.
     * @return Corresponding database object or nullptr if it doesn't exist.
     */
    DatabasePtr findDatabase(const std::string& databaseName);

    /**
     * Creates new database object and writes all necessary on-disk data structures.
     * @param name Database name.
     * @param cipherId Cipher ID used for encryption of this database.
     * @param cipherKey Key used for encryption of this database.
     * @param currentUserId Current user ID.
     * @param description Database description.
     * @return New database object.
     * @throw DatabaseError if some error has occurrred.
     */
    DatabasePtr createDatabase(std::string&& name, const std::string& cipherId,
            BinaryValue&& cipherKey, std::uint32_t currentUserId,
            std::optional<std::string>&& description);

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

    /** Creates superuser */
    void createSuperUser();

    /** Loads superuser */
    void loadSuperUser();

    /** Records superuser into the system database. */
    void recordSuperUser();

    /** 
     * Loads system database encryption key.
     * @return System database encryption key.
     * @throw DatabaseError 1) If file doesn't not exist or could not be open.
     * 2) If Read error happens.
     * 3) If key data is invalid.
     */
    BinaryValue loadSystemDatabaseCipherKey() const;

    /**
     * Loads initial super-user access key.
     * @return Access key text 
     * @throw DatabaseError If file does not exist or could not be read
     */
    std::string loadSuperUserInitialAccessKey() const;

    /**
     * Returns existing database object. Does not acquire database cache access synchronization lock.
     * @param databaseName database name.
     * @return Corresponding database object or nullptr if it doesn't exist.
     */
    DatabasePtr findDatabaseUnlocked(const std::string& databaseName);

    /** Checks instance data consistency */
    void checkDataConsistency();

    /**
     * Opens or creates new metadata file.
     * @return Metadata file descriptor.
     */
    int openMetadataFile() const;

    /**
     * Loads existing instance metadata. Performs metadata access synchronization.
     * @throw DatabaseError if metadata could not be loaded.
     */
    void syncLoadMetadata()
    {
        std::lock_guard lock(m_mutex);
        loadMetadata();
    }

    /**
     * Loads existing instance metadata. Doesn't perform metadata access synchronization.
     * @throw DatabaseError if metadata could not be loaded.
     */
    void loadMetadata();

    /**
     * Saves instance metadata. Performs metadata access synchronization.
     * @throw DatabaseError if metadata could not be loaded.
     */
    void syncSaveMetadata() const
    {
        std::lock_guard lock(m_mutex);
        saveMetadata();
    }

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
     * Returns existing user object.
     * @param userName User name.
     * @return Corresponding user object or nullptr if it doesn't exist.
     */
    UserPtr findUserUnlocked(const std::string& userName);

    /**
     * Returns existing user object.
     * @param userId User ID.
     * @return Corresponding database object or nullptr if it doesn't exist.
     */
    UserPtr findUserUnlocked(std::uint32_t userId);

    /**
     * Returns cached user object or creates new one.
     * @param userRecord User record.
     * @return Corresponding user object.
     */
    UserPtr findUserUnlocked(const UserRecord& userRecord);

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

    /** System database cipher */
    const std::string m_systemDatabaseCipherId;

    /** System database cipher key */
    BinaryValue m_systemDatabaseCipherKey;

    /** Superuser's initial access key */
    std::string m_superUserInitialAccessKey;

    /** Metadata access synchronization object */
    mutable std::mutex m_mutex;

    /** Cache and registries access synchronization object */
    mutable std::mutex m_cacheMutex;

    /** User registry. Contains information about all known users. */
    UserRegistry m_userRegistry;

    /** User cache. Contains recently used user objects. */
    UserCache m_userCache;

    /** Database registry. Contains information about all known databases. */
    DatabaseRegistry m_databaseRegistry;

    /** Database cache. Contains recently used database objects. */
    DatabaseCache m_databaseCache;

    /** Superuser */
    UserPtr m_superUser;

    /** System database */
    std::shared_ptr<SystemDatabase> m_systemDatabase;

    /** Table cache capacity */
    const std::size_t m_tableCacheCapacity;

    /** Block cache capacity */
    const std::size_t m_blockCacheCapacity;

    /* Metadata file descriptor */
    FdGuard m_metadataFile;

    /** Flag which allows creating user tables in the system database */
    const bool m_allowCreatingUserTablesInSystemDatabase;

    /** Session mutex */
    mutable std::mutex m_sessionMutex;

    /** Session UUID generator */
    boost::uuids::random_generator m_sessionUuidGenerator;

    /** Active sessions */
    std::unordered_map<Uuid, std::shared_ptr<ClientSession>> m_activeSessions;

    /** Metadata version */
    static constexpr std::uint32_t kCurrentMetadataVersion = 1;

    /** Serialized metadata size */
    static constexpr std::size_t kSerializedMetadataSize = sizeof(kCurrentMetadataVersion);

    /** Instance initialization completion flag file name */
    static constexpr const char* kInitializationFlagFile = "initialized";

    /** Metadata file name */
    static constexpr const char* kMetadataFileName = "instance_metadata";

    /** Generated token length */
    static constexpr std::size_t kGeneratedTokenLength = 64;
};

}  // namespace siodb::iomgr::dbengine
