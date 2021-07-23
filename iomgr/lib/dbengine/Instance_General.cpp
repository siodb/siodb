// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Instance.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "SystemDatabase.h"
#include "ThrowDatabaseError.h"
#include "User.h"
#include "UserAccessKey.h"
#include "UserDatabase.h"
#include "crypto/GetCipher.h"

// Common project headers
#include <siodb/common/config/SiodbDataFileDefs.h>
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/iomgr/shared/dbengine/crypto/KeyGenerator.h>
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/Cipher.h>
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/CipherContext.h>

namespace siodb::iomgr::dbengine {

Instance::Instance(const config::SiodbOptions& options)
    : m_uuid(utils::getZeroUuid())
    , m_name(options.m_generalOptions.m_name)
    , m_dataDir(options.m_generalOptions.m_dataDirectory)
    , m_defaultDatabaseCipherId(options.m_encryptionOptions.m_defaultCipherId)
    , m_masterCipher(crypto::getCipher(options.m_encryptionOptions.m_masterCipherId))
    , m_masterCipherKey(
              options.m_encryptionOptions.m_masterCipherKey.empty()
                      ? loadMasterCipherKey(options.m_encryptionOptions.m_masterCipherKeyPath)
                      : options.m_encryptionOptions.m_masterCipherKey)
    , m_masterEncryptionContext(
              m_masterCipher ? m_masterCipher->createEncryptionContext(m_masterCipherKey) : nullptr)
    , m_masterDecryptionContext(
              m_masterCipher ? m_masterCipher->createDecryptionContext(m_masterCipherKey) : nullptr)
    , m_systemDatabaseCipherId(options.m_encryptionOptions.m_systemDbCipherId)
    , m_superUserInitialAccessKey(options.m_generalOptions.m_superUserInitialAccessKey.empty()
                                          ? loadSuperUserInitialAccessKey()
                                          : options.m_generalOptions.m_superUserInitialAccessKey)
    , m_maxUsers(options.m_ioManagerOptions.m_maxUsers)
    , m_maxDatabases(options.m_ioManagerOptions.m_maxDatabases)
    , m_maxTableCountPerDatabase(options.m_ioManagerOptions.m_maxTableCountPerDatabase)
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

std::string Instance::makeDisplayName() const
{
    std::ostringstream oss;
    oss << '\'' << m_name << '\'';
    return oss.str();
}

BinaryValue Instance::encryptWithMasterEncryption(const void* data, std::size_t size) const
{
    BinaryValue buffer;
    if (size > 0) {
        if (m_masterCipher) {
            const auto blockSize = m_masterCipher->getBlockSizeInBits() / 8;
            const auto r = size % blockSize;
            const auto bufferSize = size + (r == 0 ? 0 : blockSize - r);
            buffer.resize(bufferSize);
            m_masterEncryptionContext->transform(data, size / blockSize, buffer.data());
            if (r > 0) {
                // A bit speculative, but should work for the most cases
                std::uint8_t xbuffer[2048];
                std::memcpy(xbuffer, static_cast<const std::uint8_t*>(data) + size - r, r);
                std::memset(xbuffer + r, 0, blockSize - r);
                m_masterEncryptionContext->transform(
                        xbuffer, 1, buffer.data() + buffer.size() - blockSize);
            }
        } else {
            buffer.resize(size);
            std::memcpy(buffer.data(), data, size);
        }
    }
    return buffer;
}

BinaryValue Instance::decryptWithMasterEncryption(const void* data, std::size_t size) const
{
    BinaryValue buffer;
    if (size > 0) {
        if (m_masterCipher) {
            const auto blockSize = m_masterCipher->getBlockSizeInBits() / 8;
            if (size % blockSize != 0) throw std::invalid_argument("Invalid data size");
            buffer.resize(size);
            m_masterDecryptionContext->transform(data, size / blockSize, buffer.data());
        } else {
            buffer.resize(size);
            std::memcpy(buffer.data(), data, size);
        }
    }
    return buffer;
}

// --- internals ---

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
    loadUsers();
    checkDataConsistency();
}

void Instance::ensureDataDir() const
{
    LOG_DEBUG << "Instance: Ensuring data directory.";
    system_error_code ec;
    const fs::path dataDirPath(m_dataDir);
    if (fs::exists(dataDirPath)) {
        if (!fs::is_directory(dataDirPath))
            throwDatabaseError(IOManagerMessageId::kErrorInstanceDataDirIsNotDir, m_dataDir);
    } else {
        if (!fs::create_directories(dataDirPath, ec)) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateInstanceDataDir, m_dataDir,
                    ec.value(), ec.message());
        }
    }
    if (!utils::clearDir(m_dataDir, ec)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotClearInstanceDataDir, m_dataDir,
                ec.value(), ec.message());
    }
}

void Instance::createSystemDatabase()
{
    LOG_DEBUG << "Instance: Creating system database.";
    const auto cipher = crypto::getCipher(m_systemDatabaseCipherId);
    const auto keyLength = cipher ? cipher->getKeySizeInBits() : 0;
    auto cipherKey = cipher ? crypto::generateCipherKey(keyLength, nullptr) : BinaryValue();
    m_systemDatabase =
            std::make_shared<SystemDatabase>(*this, m_systemDatabaseCipherId, std::move(cipherKey));
    m_databaseRegistry.emplace(*m_systemDatabase);
    m_databases.emplace(m_systemDatabase->getId(), m_systemDatabase);
}

void Instance::loadSystemDatabase()
{
    LOG_DEBUG << "Instance: Loading system database.";
    m_systemDatabase = std::make_shared<SystemDatabase>(*this, m_systemDatabaseCipherId);
    m_databaseRegistry.emplace(*m_systemDatabase);
    m_databases.emplace(m_systemDatabase->getId(), m_systemDatabase);
}

void Instance::loadUsers()
{
    LOG_DEBUG << "Instance: Loading users.";
    m_userRegistry = m_systemDatabase->readAllUsers();
    m_superUser = findUserChecked(User::kSuperUserId);
}

void Instance::createSuperUser()
{
    LOG_DEBUG << "Instance: Creating super user.";
    const UserRecord userRecord(User::kSuperUserId, User::kSuperUserName, std::nullopt,
            User::kSuperUserDescription, true, UserAccessKeyRegistry(), UserTokenRegistry(),
            UserPermissionRegistry());
    m_superUser = std::make_shared<User>(userRecord);
    if (!m_superUserInitialAccessKey.empty()) {
        m_superUser->addAccessKey(UserAccessKey::kSuperUserInitialAccessKeyId,
                UserAccessKey::kSuperUserInitialAccessKeyName,
                std::string(m_superUserInitialAccessKey),
                UserAccessKey::kSuperUserInitialAccessKeyDescription, true);
    }
    m_userRegistry.emplace(*m_superUser);
    m_users.emplace(m_superUser->getId(), m_superUser);
}

void Instance::recordSuperUser()
{
    LOG_DEBUG << "Instance: Recording super user.";
    const auto& tp = m_systemDatabase->getCreateTransactionParams();
    m_systemDatabase->recordUser(*m_superUser, tp);
    for (const auto& accessKey : m_superUser->getAccessKeys())
        m_systemDatabase->recordUserAccessKey(*accessKey, tp);
}

BinaryValue Instance::loadMasterCipherKey(const std::string& keyPath) const
{
    LOG_DEBUG << "Instance: Loading master cipher key.";
    if (!m_masterCipher) return BinaryValue();

    BinaryValue key(m_masterCipher->getKeySizeInBits() / 8);
    FDGuard fd(::open(keyPath.c_str(), O_RDONLY | O_CLOEXEC));
    if (!fd.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kFatalCannotOpenMasterEncryptionKey, keyPath,
                errorCode, std::strerror(errorCode));
    }

    struct stat st;
    if (::fstat(fd.getFD(), &st) < 0) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kFatalCannotStatMasterEncryptionKey, keyPath,
                errorCode, std::strerror(errorCode));
    }
    if (st.st_size != static_cast<off_t>(key.size())) {
        throwDatabaseError(IOManagerMessageId::kFatalInvalidMasterEncryptionKey, keyPath,
                key.size(), st.st_size);
    }

    const auto n = ::readExact(fd.getFD(), key.data(), key.size(), kIgnoreSignals);
    if (n != key.size()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kFatalCannotReadMasterEncryptionKey, keyPath,
                errorCode, std::strerror(errorCode), key.size(), n);
    }

    return key;
}

std::string Instance::loadSuperUserInitialAccessKey() const
{
    LOG_DEBUG << "Instance: Loading super user initial access key.";
    const auto fileName = composeInstanceInitialSuperUserAccessKeyFilePath(m_name);
    system_error_code ec;
    const auto fileSize = fs::file_size(fileName, ec);
    if (fileSize == static_cast<boost::uintmax_t>(-1))
        throwDatabaseError(IOManagerMessageId::kFatalCannotStatSuperUserKey, fileName);

    if (fileSize > siodb::kMaxUserAccessKeySize) {
        throwDatabaseError(IOManagerMessageId::kFatalSuperUserAccessKeyIsTooLong, fileSize,
                siodb::kMaxUserAccessKeySize);
    }

    LOG_DEBUG << "Instance: Read " << fileSize << " bytes of access key";
    std::ifstream ifs(fileName);
    if (!ifs.is_open())
        throwDatabaseError(IOManagerMessageId::kFatalCannotOpenSuperUserKey, fileName);

    std::string accessKey(fileSize, '\0');
    ifs.read(accessKey.data(), fileSize);
    ifs.close();

    LOG_DEBUG << "Instance: Read super user initial access key from file.";
    return accessKey;
}

void Instance::checkDataConsistency()
{
    LOG_INFO << "Instance: Checking data consistency.";

    m_databaseRegistry = m_systemDatabase->readAllDatabases();

    const auto& index = m_databaseRegistry.byUuid();
    const auto itSystemDatabase =
            std::find_if(index.cbegin(), index.cend(), [](const auto& record) noexcept {
                return record.m_uuid == Database::kSystemDatabaseUuid;
            });
    if (itSystemDatabase == index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorSystemDatabaseNotFound);

    for (const auto& record : index) {
        if (record.m_uuid == Database::kSystemDatabaseUuid) continue;
        auto database = std::make_shared<UserDatabase>(*this, record);
        m_databases.emplace(database->getId(), database);
    }
}

int Instance::openMetadataFile() const
{
    const auto metadataFilePath = makeMetadataFilePath();
    LOG_DEBUG << "Instance: Opening or creating metadta file " << metadataFilePath;
    const int fd = ::open(metadataFilePath.c_str(), O_CREAT | O_RDWR | O_CLOEXEC | O_NOATIME,
            kDataFileCreationMode);
    if (fd < 0) {
        throwDatabaseError(IOManagerMessageId::kFatalCannotOpenInstanceMetadata, metadataFilePath,
                errno, std::strerror(errno));
    }
    return fd;
}

void Instance::loadMetadata()
{
    LOG_DEBUG << "Instance: Loading metadata";

    // Read metadata file
    std::uint8_t buffer[kSerializedMetadataSize];
    if (::preadExact(m_metadataFile.getFD(), buffer, sizeof(buffer), 0, kIgnoreSignals)
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
    LOG_DEBUG << "Instance: Saving metadata";

    // Serialize metadata to buffer
    std::uint8_t buffer[kSerializedMetadataSize];
    serializeMetadata(buffer);

    // Write metadata to the file
    if (::pwriteExact(m_metadataFile.getFD(), buffer, sizeof(buffer), 0, kIgnoreSignals)
            != sizeof(buffer)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotSaveInstanceMetadata, errorCode,
                std::strerror(errorCode));
    }
}

std::string Instance::makeMetadataFilePath() const
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
        ofs << '\"' << m_name << "\"\n"
            << m_uuid << '\n'
            << std::time(nullptr) << '\n'
            << std::flush;
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
    if (instanceName.size() < 2 || instanceName.front() != '\"' || instanceName.back() != '\"'
            || instanceName.substr(1, instanceName.length() - 2) != m_name) {
        throwDatabaseError(IOManagerMessageId::kFatalInstanceNameMismatch, instanceName, m_name);
    }
}

}  // namespace siodb::iomgr::dbengine
