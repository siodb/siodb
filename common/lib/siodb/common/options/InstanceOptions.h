// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "InvalidConfigurationOptionError.h"
#include "LogOptions.h"
#include "../config/SiodbDefs.h"
#include "../utils/BinaryValue.h"

// STL headers
#include <stdexcept>

namespace siodb::config {

// Instance configuration file directory
constexpr const char* kSiodbInstanceConfigDirectory = "/etc/siodb/instances";

///////////////////////////// Options names ////////////////////////////////////

// Instance options
constexpr const char* kGeneralOptionExecutablePath = "executable_path";
constexpr const char* kGeneralOptionIpv4Port = "ipv4_port";
constexpr const char* kGeneralOptionIpv6Port = "ipv6_port";
constexpr const char* kGeneralOptionDataDirectory = "data_dir";
constexpr const char* kGeneralOptionLogChannels = "log_channels";
constexpr const char* kGeneralOptionAdminConnectionListenerBacklog =
        "admin_connection_listener_backlog";
constexpr const char* kGeneralOptionMaxAdminConnections = "max_admin_connections";
constexpr const char* kGeneralOptionUserConnectionListenerBacklog =
        "user_connection_listener_backlog";
constexpr const char* kGeneralOptionMaxUserConnections = "max_user_connections";

// IO Manager options
constexpr const char* kIOManagerOptionIpv4Port = "iomgr.ipv4_port";
constexpr const char* kIOManagerOptionIpv6Port = "iomgr.ipv6_port";
constexpr const char* kIOManagerOptionWorkerThreadNumber = "iomgr.worker_thread_number";
constexpr const char* kIOManagerOptionWriterThreadNumber = "iomgr.writer_thread_number";
constexpr const char* kIOManagerOptionUserCacheCapacity = "iomgr.user_cache_capacity";
constexpr const char* kIOManagerOptionDatabaseCacheCapacity = "iomgr.database_cache_capacity";
constexpr const char* kIOManagerOptionTableCacheCapacity = "iomgr.table_cache_capacity";
constexpr const char* kIOManagerOptionBlockCacheCapacity = "iomgr.block_cache_capacity";

// Encryption options
constexpr const char* kEncryptionOptionDefaultCipherId = "encryption.default_cipher_id";
constexpr const char* kEncryptionOptionSystemDbCipherId = "encryption.system_db_cipher_id";

// Client options
constexpr const char* kClientOptionEnableEncryption = "client.enable_encryption";
constexpr const char* kClientOptionTlsCertificate = "client.tls_certificate";
constexpr const char* kClientOptionTlsCertificateChain = "client.tls_certificate_chain";
constexpr const char* kClientOptionTlsPrivateKey = "client.tls_private_key";

// Log channel options
constexpr const char* kLogChannelOptionType = "type";
constexpr const char* kLogChannelOptionDestination = "destination";
constexpr const char* kLogChannelOptionMaxFileSize = "max_file_size";
constexpr const char* kLogChannelOptionMaxFiles = "max_files";
constexpr const char* kLogChannelOptionExpirationTime = "exp_time";
constexpr const char* kLogChannelOptionSeverity = "severity";

///////////////////////////// Default values ///////////////////////////////////

// Port
constexpr int kDefaultIpv4PortNumber = 50000;
constexpr int kDefaultIpv6PortNumber = 0;

// Admin connection listener backlog
constexpr int kDefaultAdminConnectionListenerBacklog = 10;
constexpr int kMaxAdminConnectionListenerBacklog = 10;

// Max. number of admin connections
constexpr unsigned kDefaultMaxAdminConnections = 10;
constexpr unsigned kMaxMaxAdminConnections = 100;

// User connection listener backlog
constexpr int kDefaultUserConnectionListenerBacklog = 10;
constexpr int kMaxUserConnectionListenerBacklog = 32768;

// Max. number of user connections
constexpr unsigned kDefaultMaxUserConnections = 10;
constexpr unsigned kMaxMaxUserConnections = 32768;

// Default number of IO Manager worker threads
constexpr const unsigned kDefaultIOManagerWorkerThreadNumber = 2;
constexpr const unsigned kDefaultIOManagerWriterThreadNumber = 2;

// Default IOManager ports
constexpr auto kDefaultIOManagerIpv4PortNumber = 50001;
constexpr auto kDefaultIOManagerIpv6PortNumber = 0;

// IOManager user cache capacity
constexpr std::size_t kMinIOManagerUserCacheCapacity = 2;
constexpr std::size_t kDefaultIOManagerUserCacheCapacity = 100;

// IOManager database cache capacity
constexpr std::size_t kMinIOManagerDatabaseCacheCapacity = 2;
constexpr std::size_t kDefaultIOManagerDatabaseCacheCapacity = 100;

// IOManager table cache capacity
constexpr std::size_t kMaxNumberOfSystemTables = 4;
constexpr std::size_t kMinIOManagerTableCacheCapacity = kMaxNumberOfSystemTables + 1;
constexpr std::size_t kDefaultIOManagerTableCacheCapacity = 100;

// IOManager block cache capacity
constexpr std::size_t kMinIOManagerBlockCacheCapacity = 50;
constexpr std::size_t kDefaultIOManagerBlockCacheCapacity = 103;

/** Default cipher */
constexpr const char* kDefaultCipherId = "aes128";

/** Default client enable encryption */
constexpr bool kDefaultClientEnableEncryption = true;

/** Default admin client enable encryption */
constexpr bool kDefaultAdminClientEnableEncryption = false;

/** General instance options */
struct GeneralOptions {
    /** Instance name */
    std::string m_name;

    /** Path to instance executable */
    std::string m_executablePath;

    /** Instance IPv4 TCP port number */
    int m_ipv4port = kDefaultIpv4PortNumber;

    /** Instance IPv6 TCP port number */
    int m_ipv6port = kDefaultIpv6PortNumber;

    /** Path of the data directory */
    std::string m_dataDirectory;

    /** Admin connection listener backlog */
    unsigned m_adminConnectionListenerBacklog = kDefaultAdminConnectionListenerBacklog;

    /** Maximum number of admin connections */
    unsigned m_maxAdminConnections = kDefaultMaxAdminConnections;

    /** User connection listener backlog */
    unsigned m_userConnectionListenerBacklog = kDefaultUserConnectionListenerBacklog;

    /** Maximum number of user connections */
    unsigned m_maxUserConnections = kDefaultMaxUserConnections;

    /**
     * Explicit superuser's initial access key. Needed only when creating new instance.
     * Use this one only for unit tests.
     */
    std::string m_superUserInitialAccessKey;

    /**
     * Allow creating user table in the system database.
     * Use this one only for unit tests.
     */
    bool m_allowCreatingUserTablesInSystemDatabase = false;
};

/** IO Manager options */
struct IOManagerOptions {
    /** Worker thread number */
    std::size_t m_workerThreadNumber = kDefaultIOManagerWorkerThreadNumber;

    /** Writer thread number */
    std::size_t m_writerThreadNumber = kDefaultIOManagerWriterThreadNumber;

    /** IPv4 TCP port number */
    int m_ipv4port = kDefaultIOManagerIpv4PortNumber;

    /** IPv6 TCP port number */
    int m_ipv6port = kDefaultIOManagerIpv6PortNumber;

    /** User cache capacity */
    std::size_t m_userCacheCapacity = kDefaultIOManagerUserCacheCapacity;

    /** Database cache capacity */
    std::size_t m_databaseCacheCapacity = kDefaultIOManagerDatabaseCacheCapacity;

    /** Table cache capacity */
    std::size_t m_tableCacheCapacity = kDefaultIOManagerTableCacheCapacity;

    /** Block cache capacity */
    std::size_t m_blockCacheCapacity = kDefaultIOManagerBlockCacheCapacity;
};

/** Extenal cipher options */
struct ExternalCipherOptions {
    /** Just something for now */
    int m_dummy = 0;
};

/** Encryption options */
struct EncryptionOptions {
    /** Default cipher ID */
    std::string m_defaultCipherId;

    /** System database cipher ID */
    std::string m_systemDbCipherId;

    /** Explicit system database cipher key. Use this one only for unit tests. */
    BinaryValue m_systemDbCipherKey;

    /** External cipher options */
    ExternalCipherOptions m_externalCipherOptions;
};

/** Client options */
struct ClientOptions {
    /** Indication that encryption is enabled */
    bool m_enableEncryption = false;

    /** TLS certificate */
    std::string m_tlsCertificate;

    /** TLS certificate chain */
    std::string m_tlsCertificateChain;

    /** TLS private key */
    std::string m_tlsPrivateKey;
};

/** Whole database options */
struct InstanceOptions {
    /** Instance options */
    GeneralOptions m_generalOptions;

    /** IO Manager options */
    IOManagerOptions m_ioManagerOptions;

    /** Log options */
    LogOptions m_logOptions;

    /** Encryption options */
    EncryptionOptions m_encryptionOptions;

    /** Client options */
    ClientOptions m_clientOptions;

    /**
     * Returns executable directory path.
     * @return Directory path.
     */
    std::string getExecutableDir() const;

    /**
     * Reads options for given instance.
     * @param instanceName instance name.
     * @throw InvalidConfigurationOptionError
     *         - if database instance name is invalid or not known.
     *         - if value of some configuration options is invalid.
     */
    void load(const std::string& instanceName);
};

/** Instance options shared pointer shortcut type */
using InstaceOptionsPtr = std::shared_ptr<InstanceOptions>;

/** Constant instance options shared pointer shortcut type */
using ConstInstaceOptionsPtr = std::shared_ptr<const InstanceOptions>;

}  // namespace siodb::config
