// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Instance.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "SystemDatabase.h"
#include "ThrowDatabaseError.h"
#include "UserAccessKey.h"
#include "UserDatabase.h"
#include "crypto/ciphers/Cipher.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/crypto/DigitalSignatureKey.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/options/DatabaseInstance.h>
#include <siodb/common/options/InstanceOptions.h>
#include <siodb/common/stl_ext/utility_ext.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FsUtils.h>
#include <siodb/common/utils/MessageCatalog.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// CRT headers
#include <cstdio>
#include <cstdlib>

// STL headers
#include <algorithm>
#include <fstream>

// System headers
#include <sys/stat.h>
#include <sys/wait.h>

// Boost headers
#include <boost/uuid/uuid_generators.hpp>

namespace siodb::iomgr::dbengine {

Instance::Instance(const config::InstanceOptions& options)
    : m_uuid(utils::getZeroUuid())
    , m_name(options.m_generalOptions.m_name)
    , m_dataDir(options.m_generalOptions.m_dataDirectory)
    , m_defaultDatabaseCipherId(options.m_encryptionOptions.m_defaultCipherId)
    , m_systemDatabaseCipherId(options.m_encryptionOptions.m_systemDbCipherId)
    , m_systemDatabaseCipherKey(options.m_encryptionOptions.m_systemDbCipherKey.empty()
                                        ? loadSystemDatabaseCipherKey()
                                        : options.m_encryptionOptions.m_systemDbCipherKey)
    , m_superUserInitialAccessKey(options.m_generalOptions.m_superUserInitialAccessKey.empty()
                                          ? loadSuperUserInitialAccessKey()
                                          : options.m_generalOptions.m_superUserInitialAccessKey)
    , m_userCache(options.m_ioManagerOptions.m_userCacheCapacity)
    , m_databaseCache(options.m_ioManagerOptions.m_databaseCacheCapacity)
    , m_tableCacheCapacity(options.m_ioManagerOptions.m_tableCacheCapacity)
    , m_blockCacheCapacity(options.m_ioManagerOptions.m_blockCacheCapacity)
    , m_metadataFile()
    , m_allowCreatingUserTablesInSystemDatabase(
              options.m_generalOptions.m_allowCreatingUserTablesInSystemDatabase)
{
    if (fs::exists(utils::constructPath(m_dataDir, kInitializationFlagFile)))
        loadInstanceData();
    else
        createInstanceData();
}

std::string Instance::getDisplayName() const
{
    std::ostringstream oss;
    oss << '\'' << m_name << '\'';
    return oss.str();
}

std::uint32_t Instance::generateNextDatabaseId(bool system)
{
    return m_systemDatabase ? m_systemDatabase->generateNextDatabaseId(system) : 1;
}

std::size_t Instance::getDatbaseCount() const
{
    std::lock_guard lock(m_mutex);
    return m_databaseRegistry.size();
}

std::vector<DatabaseRecord> Instance::getDatabaseRecordsOrderedByName() const noexcept
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_databaseRegistry.byName();
    std::vector<DatabaseRecord> dbRecords(index.begin(), index.end());
    std::sort(
            dbRecords.begin(), dbRecords.end(), [
            ](const auto& left, const auto& right) noexcept { return left.m_name < right.m_name; });
    return dbRecords;
}

DatabasePtr Instance::getDatabaseChecked(const std::string& databaseName)
{
    if (auto db = getDatabase(databaseName)) return db;
    throwDatabaseError(IOManagerMessageId::kErrorDatabaseDoesNotExist, databaseName);
}

DatabasePtr Instance::getDatabase(const std::string& databaseName)
{
    std::lock_guard lock(m_mutex);
    const auto& index = m_databaseRegistry.byName();
    const auto it = index.find(databaseName);
    if (it == index.end()) return nullptr;

    auto cachedDatabase = m_databaseCache.get(it->m_id);
    if (cachedDatabase) return *cachedDatabase;

    auto database = std::make_shared<UserDatabase>(*this, *it, m_tableCacheCapacity);
    m_databaseCache.emplace(database->getId(), database);
    return database;
}

DatabasePtr Instance::createDatabase(const std::string& name, const std::string& cipherId,
        const BinaryValue& cipherKey, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    // Check if database already exists
    if (m_databaseRegistry.byName().count(name) > 0)
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseAlreadyExists, name);

    // Create and register database
    auto database =
            std::make_shared<UserDatabase>(*this, name, cipherId, cipherKey, m_tableCacheCapacity);
    m_databaseRegistry.emplace(*database);
    const TransactionParameters tp(currentUserId, m_systemDatabase->generateNextTransactionId());
    m_systemDatabase->recordDatabase(*database, tp);
    m_databaseCache.emplace(database->getId(), database);
    return database;
}

bool Instance::dropDatabase(
        const std::string& name, bool databaseMustExist, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto database = getDatabase(name);
    if (!database) {
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
    m_databaseCache.erase(id);
    m_databaseRegistry.byId().erase(id);
    m_systemDatabase->deleteDatabase(id, currentUserId);
    boost::system::error_code errorCode;
    if (fs::remove_all(dataDir, errorCode) == static_cast<std::uintmax_t>(-1)) {
        throwDatabaseError(IOManagerMessageId::kWarningCannotRemoveDatabaseDataDirectory,
                database->getName(), uuid, errorCode.value(), errorCode.message());
    }
    return true;
}

UserPtr Instance::getUserChecked(const std::string& userName)
{
    std::lock_guard lock(m_mutex);
    if (auto user = getUserUnlocked(userName)) return user;
    throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);
}

UserPtr Instance::getUserChecked(std::uint32_t userId)
{
    std::lock_guard lock(m_mutex);
    if (auto user = getUserUnlocked(userId)) return user;
    throwDatabaseError(IOManagerMessageId::kErrorUserIdDoesNotExist, userId);
}

std::uint32_t Instance::createUser(const std::string& name, const std::string& realName,
        bool active, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    // Check if user already exists
    if (m_userRegistry.byName().count(name) > 0)
        throwDatabaseError(IOManagerMessageId::kErrorUserAlreadyExists, name);

    // Create and register user
    auto user = std::make_shared<User>(*m_systemDatabase, name, realName, active);
    m_userRegistry.emplace(*user);
    const TransactionParameters tp(currentUserId, m_systemDatabase->generateNextTransactionId());
    m_systemDatabase->recordUser(*user, tp);
    m_userCache.emplace(user->getId(), user);
    return user->getId();
}

void Instance::dropUser(const std::string& name, bool userMustExist, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    auto& index = m_userRegistry.byName();
    const auto it = index.find(name);
    if (it == index.end()) {
        // Support DROP USER IF EXISTS
        if (!userMustExist) return;
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, name);
    }

    const auto id = it->m_id;
    if (id == User::kSuperUserId) throwDatabaseError(IOManagerMessageId::kErrorCannotDropSuperUser);

    auto user = getUserUnlocked(*it);
    m_userCache.erase(id);
    index.erase(it);

    for (const auto& accessKey : user->getAccessKeys())
        m_systemDatabase->deleteUserAccessKey(accessKey->getId(), currentUserId);

    m_systemDatabase->deleteUser(id, currentUserId);
}

void Instance::updateUser(const std::string& name, const std::optional<bool>& active,
        const std::optional<std::string>& realName, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);
    const auto& index = m_userRegistry.byName();
    const auto it = index.find(name);
    if (it == index.cend()) throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, name);
    auto& mutableUserRecord = stdext::as_mutable(*it);

    const auto user = getUserUnlocked(*it);
    const auto id = user->getId();

    const bool needUpdateState = active && *active != it->m_active;
    const bool needUpdateRealName = realName && *realName != it->m_realName;
    if (!needUpdateState && !needUpdateRealName) return;

    if (needUpdateState) {
        if (id == User::kSuperUserId)
            throwDatabaseError(IOManagerMessageId::kErrorCannotChangeSuperUserState);
        user->setActive(*active);
        // NOTE: The following one still correct only because we do not index
        // by UserRecord::m_active.
        mutableUserRecord.m_active = *active;
    }

    if (needUpdateRealName) {
        // It's allowed to change real name of a super user
        user->setRealName(*realName);
        // NOTE: The following one still correct only because we do not index
        // by UserRecord::m_realName.
        mutableUserRecord.m_realName = *realName;
    }

    m_systemDatabase->updateUser(id, active, realName, currentUserId);
}

std::uint64_t Instance::createUserAccessKey(const std::string& userName, const std::string& name,
        const std::string& text, bool active, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto& index = m_userRegistry.byName();
    const auto it = index.find(userName);
    if (it == index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);

    // NOTE: We don't index by UserRecord::m_accessKeys.
    auto& mutableUserRecord = stdext::as_mutable(*it);

    const auto user = getUserUnlocked(*it);

    const auto id = m_systemDatabase->generateNextUserAccessKeyId();
    const auto accessKey = user->addUserAccessKey(id, name, text, active);

    // NOTE: We don't index by UserRecord::m_accessKeys.
    mutableUserRecord.m_accessKeys.emplace(*accessKey);

    const TransactionParameters tp(currentUserId, m_systemDatabase->generateNextTransactionId());
    m_systemDatabase->recordUserAccessKey(*accessKey, tp);

    return id;
}

void Instance::dropUserAccessKey(const std::string& userName, const std::string& name,
        bool accessKeyMustExist, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto& userIndex = m_userRegistry.byName();
    const auto itUser = userIndex.find(userName);
    if (itUser == userIndex.end()) {
        if (!accessKeyMustExist) return;
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);
    }

    // NOTE: We don't index by UserRecord::m_accessKeys.
    auto& accessKeyIndex = stdext::as_mutable(*itUser).m_accessKeys.byName();
    const auto itKey = accessKeyIndex.find(name);
    if (itKey == accessKeyIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyDoesNotExist, userName, name);

    const auto user = getUserUnlocked(*itUser);

    const auto accessKeyId = itKey->m_id;
    accessKeyIndex.erase(itKey);
    user->deleteUserAccessKey(name);
    m_systemDatabase->deleteUserAccessKey(accessKeyId, currentUserId);
}

void Instance::updateUserAccessKey(const std::string& userName, const std::string& name,
        const std::optional<bool>& active, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto& userIndex = m_userRegistry.byName();
    const auto itUser = userIndex.find(userName);
    if (itUser == userIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);

    // NOTE: We don't index by UserRecord::m_accessKeys.
    auto& accessKeyIndex = stdext::as_mutable(*itUser).m_accessKeys.byName();
    const auto itKey = accessKeyIndex.find(name);
    if (itKey == accessKeyIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyDoesNotExist, userName, name);

    const auto user = getUserUnlocked(*itUser);
    const auto userAccessKey = user->getUserAccessKeyChecked(name);

    if (!active) return;

    const bool activeNow = userAccessKey->isActive();
    if (activeNow == active) return;

    if (user->isSuperUser() && !active && user->getActiveKeyCount() == 1)
        throwDatabaseError(IOManagerMessageId::kErrorCannotDeactivateLastSuperUserAccessKey, name);

    // NOTE: We do index by UserAccessKeyRecord::m_active, so modify it in a special way.
    if (accessKeyIndex.modify(
                itKey,
                [&active](UserAccessKeyRecord & record) noexcept { record.m_active = *active; },
                [activeNow](
                        UserAccessKeyRecord & record) noexcept { record.m_active = activeNow; })) {
        userAccessKey->setActive(*active);
        m_systemDatabase->updateUserAccessKey(userAccessKey->getId(), *active, currentUserId);
    }
}

void Instance::beginUserAuthentication(const std::string& userName)
{
    const auto user = getUserChecked(userName);
    if (!user->isActive()) throwDatabaseError(IOManagerMessageId::kErrorUserAccessDenied, userName);
    if (user->getActiveKeyCount() == 0)
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessDenied, userName);
}

std::pair<std::uint32_t, Uuid> Instance::authenticateUser(
        const std::string& userName, const std::string& signature, const std::string& challenge)
{
    const auto user = getUserChecked(userName);
    const auto accessKeys = user->getAccessKeys();

    if (!user->isActive()) throwDatabaseError(IOManagerMessageId::kErrorUserAccessDenied, userName);

    const auto itKey =
            std::find_if(accessKeys.begin(), accessKeys.end(), [&](const auto& accessKey) {
                if (!accessKey->isActive()) return false;

                siodb::crypto::DigitalSignatureKey key;
                try {
                    key.parseFromString(accessKey->getText());
                    return key.verifySignature(challenge, signature);
                } catch (std::exception& e) {
                    LOG_WARNING << "Instance: signature verification failed: " << e.what();
                }

                return false;
            });

    if (itKey == accessKeys.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessDenied, userName);

    LOG_INFO << "Instance: User '" << userName << "' authenticated.";

    return std::make_pair(user->getId(), beginSession());
}

void Instance::endSession(const Uuid& sessionUuid)
{
    std::lock_guard lock(m_sessionMutex);
    if (m_activeSessions.erase(sessionUuid) == 0)
        throwDatabaseError(IOManagerMessageId::kErrorSessionDoesNotExist, sessionUuid);
    LOG_INFO << "Session " << sessionUuid << " finished";
}

// ---- internal ----

void Instance::createInstanceData()
{
    LOG_INFO << "Instance: Creating new instance data.";
    ensureDataDir();
    m_metadataFile.reset(openMetadataFile());
    createSuperUser();
    createSystemDatabase();
    recordSuperUser();
    saveMetadata();
    createInitializationFlagFile();
    checkDataConsistency();
}

void Instance::loadInstanceData()
{
    LOG_INFO << "Instance: Loading instance data.";
    checkInitializationFlagFile();
    m_metadataFile.reset(openMetadataFile());
    loadMetadata();
    loadSystemDatabase();
    loadSuperUser();
    checkDataConsistency();
}

void Instance::ensureDataDir() const
{
    LOG_DEBUG << "Instance: Ensuring data directory.";
    boost::system::error_code errorCode;
    const fs::path dataDirPath(m_dataDir);
    if (fs::exists(dataDirPath)) {
        if (!fs::is_directory(dataDirPath))
            throwDatabaseError(IOManagerMessageId::kErrorInstanceDataDirIsNotDir, m_dataDir);
    } else {
        if (!fs::create_directories(dataDirPath, errorCode)) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateInstanceDataDir, m_dataDir,
                    errorCode.value(), errorCode.message());
        }
    }
    if (!utils::clearDir(m_dataDir, errorCode)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotClearInstanceDataDir, m_dataDir,
                errorCode.value(), errorCode.message());
    }
}

void Instance::createSystemDatabase()
{
    LOG_DEBUG << "Instance: Creating system database.";
    m_systemDatabase = std::make_shared<SystemDatabase>(*this, m_systemDatabaseCipherId,
            m_systemDatabaseCipherKey, m_superUserInitialAccessKey);
    m_databaseRegistry.emplace(*m_systemDatabase);
    m_databaseCache.emplace(m_systemDatabase->getId(), m_systemDatabase);
}

void Instance::loadSystemDatabase()
{
    LOG_DEBUG << "Instance: Loading system database.";
    m_systemDatabase = std::make_shared<SystemDatabase>(
            *this, m_systemDatabaseCipherId, m_systemDatabaseCipherKey);
    m_databaseRegistry.emplace(*m_systemDatabase);
    m_databaseCache.emplace(m_systemDatabase->getId(), m_systemDatabase);
}

void Instance::createSuperUser()
{
    LOG_DEBUG << "Instance: Creating super user.";
    m_superUser = std::make_shared<User>(UserRecord(User::kSuperUserId, User::kSuperUserName,
            std::string(), true, UserAccessKeyRegistry()));
    if (!m_superUserInitialAccessKey.empty()) {
        m_superUser->addUserAccessKey(UserAccessKey::kSuperUserInitialAccessKeyId,
                UserAccessKey::kSuperUserInitialAccessKeyName, m_superUserInitialAccessKey, true);
    }
    m_userRegistry.emplace(*m_superUser);
    m_userCache.emplace(m_superUser->getId(), m_superUser);
}

void Instance::loadSuperUser()
{
    LOG_DEBUG << "Instance: Loading super user.";
    m_systemDatabase->readAllUsers(m_userRegistry);
    m_superUser = getUserChecked(User::kSuperUserId);
}

void Instance::recordSuperUser()
{
    LOG_DEBUG << "Instance: Recording super user.";
    const auto& tp = m_systemDatabase->getCreateTransactionParams();
    m_systemDatabase->recordUser(*m_superUser, tp);
    for (const auto& accessKey : m_superUser->getAccessKeys())
        m_systemDatabase->recordUserAccessKey(*accessKey, tp);
}

BinaryValue Instance::loadSystemDatabaseCipherKey() const
{
    LOG_DEBUG << "Instance: Loading system database cipher key.";
    const auto cipher = crypto::getCipher(m_systemDatabaseCipherId);
    if (!cipher) return BinaryValue();
    BinaryValue key(cipher->getKeySize() / 8);
    const auto keyPath = composeInstanceSysDbEncryptionKeyFilePath(m_name);
    FileDescriptorGuard fd(::open(keyPath.c_str(), O_RDONLY));
    if (!fd.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kFatalCannotOpenSystemDatabaseEncryptionKey, keyPath,
                errorCode, std::strerror(errorCode));
    }
    struct stat st;
    if (::fstat(fd.getFd(), &st) < 0) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kFatalCannotStatSystemDatabaseEncryptionKey, keyPath,
                errorCode, std::strerror(errorCode));
    }
    if (st.st_size != static_cast<off_t>(key.size())) {
        throwDatabaseError(IOManagerMessageId::kFatalInvalidSystemDatabaseEncryptionKey, keyPath,
                key.size(), st.st_size);
    }
    if (::readExact(fd.getFd(), key.data(), key.size(), kIgnoreSignals) != key.size()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kFatalCannotReadSystemDatabaseEncryptionKey, keyPath,
                errorCode, std::strerror(errorCode));
    }
    return key;
}

std::string Instance::loadSuperUserInitialAccessKey() const
{
    LOG_DEBUG << "Instance: Loading super user initial access key.";
    const auto fileName = composeInstanceInitialSuperUserAccessKeyFilePath(m_name);
    boost::system::error_code errorCode;
    const auto fileSize = fs::file_size(fileName, errorCode);
    if (fileSize == static_cast<boost::uintmax_t>(-1))
        throwDatabaseError(IOManagerMessageId::kFatalCannotStatSuperUserKey, fileName);

    if (fileSize > siodb::kMaxAccessKeySize)
        throwDatabaseError(IOManagerMessageId::kFatalSuperUserAccessKeyIsLargerThanMax, fileSize,
                siodb::kMaxAccessKeySize);

    LOG_DEBUG << "Instance: Read " << fileSize << " bytes of access key";
    std::ifstream ifs(fileName);
    if (!ifs.is_open())
        throwDatabaseError(IOManagerMessageId::kFatalCannotOpenSuperUserKey, fileName);

    std::string accessKey(fileSize, '\0');

    ifs.read(accessKey.data(), fileSize);

    LOG_DEBUG << "Instance: Read super user initial access key from file.";
    return accessKey;
}

DatabasePtr Instance::getDatabaseUnlocked(const std::string& databaseName)
{
    const auto& databasesByName = m_databaseRegistry.byName();
    const auto it = databasesByName.find(databaseName);
    if (it == databasesByName.end()) return nullptr;

    const auto cachedDatabase = m_databaseCache.get(it->m_id);
    if (cachedDatabase) return *cachedDatabase;

    auto database = std::make_shared<UserDatabase>(*this, *it, m_tableCacheCapacity);
    m_databaseCache.emplace(database->getId(), database);
    return database;
}

void Instance::checkDataConsistency()
{
    LOG_INFO << "Instance: Checking data consistency.";

    m_systemDatabase->readAllDatabases(m_databaseRegistry);

    const auto& index = m_databaseRegistry.byUuid();
    const auto itSystemDatabase = std::find_if(
            index.cbegin(), index.cend(), [](const auto& record) noexcept {
                return record.m_uuid == Database::kSystemDatabaseUuid;
            });
    if (itSystemDatabase == index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorSystemDatabaseNotFound);

    for (const auto& record : index) {
        if (record.m_uuid == Database::kSystemDatabaseUuid) continue;
        auto database = std::make_shared<UserDatabase>(*this, record, m_tableCacheCapacity);
        m_databaseCache.emplace(database->getId(), database);
    }
}

int Instance::openMetadataFile() const
{
    const auto metadataFilePath = getMetadataFilePath();
    int fd = ::open(metadataFilePath.c_str(), O_CREAT | O_RDWR | O_CLOEXEC | O_NOATIME,
            kDataFileCreationMode);
    if (fd < 0) {
        throwDatabaseError(IOManagerMessageId::kFatalCannotOpenInstanceMetadata, metadataFilePath,
                errno, std::strerror(errno));
    }
    return fd;
}

void Instance::loadMetadata()
{
    // Read metadata file
    std::uint8_t buffer[kSerializedMetadataSize];
    if (::preadExact(m_metadataFile.getFd(), buffer, sizeof(buffer), 0, kIgnoreSignals)
            != sizeof(buffer)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kFatalCannotLoadInstanceMetadata, errorCode,
                std::strerror(errorCode));
    }

    // Decode metadata
    if (!deserializeMetadata(buffer)) {
        throwDatabaseError(
                IOManagerMessageId::kFatalCannotLoadInstanceMetadata, -1, "can't decode metadata");
    }
}

void Instance::saveMetadata() const
{
    // Serialize metadata to buffer
    std::uint8_t buffer[kSerializedMetadataSize];
    serializeMetadata(buffer);

    // Write metadata to the file
    if (::pwriteExact(m_metadataFile.getFd(), buffer, sizeof(buffer), 0, kIgnoreSignals)
            != sizeof(buffer)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotSaveInstanceMetadata, errorCode,
                std::strerror(errorCode));
    }
}

std::string Instance::getMetadataFilePath() const
{
    return utils::constructPath(m_dataDir, kMetadataFileName);
}

std::uint8_t* Instance::serializeMetadata(std::uint8_t* buffer) const noexcept
{
    buffer = ::pbeEncodeUInt32(kCurrentMetadataVersion, buffer);
    return buffer;
}

const std::uint8_t* Instance::deserializeMetadata(const std::uint8_t* buffer) noexcept
{
    std::uint32_t version = 0;
    buffer = ::pbeDecodeUInt32(buffer, &version);
    if (version > kCurrentMetadataVersion) return nullptr;
    return buffer;
}

void Instance::createInitializationFlagFile() const
{
    LOG_DEBUG << "Instance: Creating intialization flag file.";
    const auto initFlagFile = utils::constructPath(m_dataDir, kInitializationFlagFile);
    std::ofstream ofs(initFlagFile);
    if (!ofs.is_open()) {
        throwDatabaseError(IOManagerMessageId::kFatalCannotCreateInstanceInitializationFlagFile,
                initFlagFile, "can't create initialization flag file");
    }
    ofs.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        ofs << m_name << '\n' << m_uuid << '\n' << std::time(nullptr) << '\n' << std::flush;
    } catch (std::exception& ex) {
        throwDatabaseError(IOManagerMessageId::kFatalCannotCreateInstanceInitializationFlagFile,
                initFlagFile, "can't write initialization flag file");
    }
}

void Instance::checkInitializationFlagFile() const
{
    LOG_DEBUG << "Instance: Checking intialization flag file.";
    const auto initFlagFile = utils::constructPath(m_dataDir, kInitializationFlagFile);
    std::ifstream ifs(initFlagFile);
    if (!ifs.is_open()) {
        throwDatabaseError(IOManagerMessageId::kFatalCannotOpenInstanceInitializationFlagFile,
                initFlagFile, "can't open initialization flag file for reading");
    }
    ifs.exceptions(std::ios::badbit | std::ios::failbit);
    std::string instanceName;
    std::getline(ifs, instanceName);
    if (instanceName != m_name)
        throwDatabaseError(IOManagerMessageId::kFatalInstanceNameMismatch, instanceName, m_name);
}

Uuid Instance::beginSession()
{
    std::lock_guard lock(m_sessionMutex);

    Uuid sessionUuid;
    do {
        sessionUuid = m_sessionUuidGenerator();
    } while (m_activeSessions.find(sessionUuid) != m_activeSessions.end());

    m_activeSessions.emplace(sessionUuid, std::make_shared<ClientSession>(sessionUuid));
    LOG_INFO << "Session " << sessionUuid << " started";
    return sessionUuid;
}

UserPtr Instance::getUserUnlocked(const std::string& userName)
{
    DBG_LOG_DEBUG("Looking up user '" << userName << "'");
    const auto& index = m_userRegistry.byName();
    const auto it = index.find(userName);
    if (it == index.cend()) return nullptr;
    return getUserUnlocked(*it);
}

UserPtr Instance::getUserUnlocked(std::uint32_t userId)
{
    DBG_LOG_DEBUG("Looking up user #" << userId);
    const auto& index = m_userRegistry.byId();
    const auto it = index.find(userId);
    if (it == index.cend()) return nullptr;
    return getUserUnlocked(*it);
}

UserPtr Instance::getUserUnlocked(const UserRecord& userRecord)
{
    const auto cachedUser = m_userCache.get(userRecord.m_id);
    if (cachedUser) return *cachedUser;
    auto user = std::make_shared<User>(userRecord);
    m_userCache.emplace(user->getId(), user);
    return user;
}

}  // namespace siodb::iomgr::dbengine
