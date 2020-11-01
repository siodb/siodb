// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "InvalidConfigurationError.h"
#include "LogOptions.h"
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
constexpr const char* kGeneralOptionDeadConnectionCleanupInterval =
        "dead_connection_cleanup_interval";
constexpr const char* kGeneralOptionAllowGroupPermissionsOnConfigFiles =
        "allow_group_permissions_on_config_files";
constexpr const char* kGeneralOptionIgnorePermissionsOnConfigFiles =
        "ignore_permissions_on_config_files";
constexpr const char* kGeneralOptionEnableRestServer = "enable_rest_server";

// IO Manager options
constexpr const char* kIOManagerOptionIpv4SqlPort = "iomgr.ipv4_port";
constexpr const char* kIOManagerOptionIpv6SqlPort = "iomgr.ipv6_port";
constexpr const char* kIOManagerOptionIpv4RestPort = "iomgr.rest.ipv4_port";
constexpr const char* kIOManagerOptionIpv6RestPort = "iomgr.rest.ipv6_port";
constexpr const char* kIOManagerOptionWorkerThreadNumber = "iomgr.worker_thread_number";
constexpr const char* kIOManagerOptionWriterThreadNumber = "iomgr.writer_thread_number";
constexpr const char* kIOManagerOptionMaxUsers = "iomgr.max_users";
constexpr const char* kIOManagerOptionMaxDatabases = "iomgr.max_databases";
constexpr const char* kIOManagerOptionMaxTablesPerDatabase = "iomgr.max_tables_per_db";
constexpr const char* kIOManagerOptionBlockCacheCapacity = "iomgr.block_cache_capacity";
constexpr const char* kIOManagerOptionDeadConnectionCleanupInterval =
        "iomgr.dead_connection_cleanup_interval";
constexpr const char* kIOManagerOptionMaxJsonPayloadSize = "iomgr.max_json_payload_size";

// Encryption options
constexpr const char* kEncryptionOptionDefaultCipherId = "encryption.default_cipher_id";
constexpr const char* kEncryptionOptionMasterCipherId = "encryption.master_cipher_id";
constexpr const char* kEncryptionOptionMasterKey = "encryption.master_key";
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

// Siodb dead connection cleanup period in seconds
constexpr unsigned kMinOptionDeadConnectionCleanupInterval = 3;
constexpr unsigned kMaxOptionDeadConnectionCleanupInterval = 3600;
constexpr unsigned kDefaultOptionDeadConnectionCleanupInterval = 15;

// Allow group permission of config files
constexpr bool kDefaultOptionAllowGroupPermissionsOnConfigFiles = true;

// Ignore permissions of config files
constexpr bool kDefaultOptionIgnorePermissionsOnConfigFiles = false;

// Whether REST Server is enabled by default
constexpr bool kDefaultOptionEnableRestServer = false;

// Default number of IO Manager worker threads
constexpr const unsigned kDefaultIOManagerWorkerThreadNumber = 2;
constexpr const unsigned kDefaultIOManagerWriterThreadNumber = 2;

// Default IO Manager ports
constexpr auto kDefaultIOManagerIpv4SqlPortNumber = 50001;
constexpr auto kDefaultIOManagerIpv6SqlPortNumber = 0;
constexpr auto kDefaultIOManagerIpv4RestPortNumber = 50002;
constexpr auto kDefaultIOManagerIpv6RestPortNumber = 0;

// IO Manager user number limit
constexpr std::size_t kMinIOManagerMaxUsers = 2;
constexpr std::size_t kDefaultIOManagerMaxUsers = 8192;

// IO Manager database number limit
constexpr std::size_t kMinIOManagerMaxDatabases = 2;
constexpr std::size_t kDefaultIOManagerMaxDatabases = 65536;

// IO Manager table number limit
constexpr std::size_t kMaxNumberOfSystemTables = 99;
constexpr std::size_t kMinIOManagerMaxTablesPerDatabase = kMaxNumberOfSystemTables + 1;
constexpr std::size_t kDefaultIOManagerMaxTablesPerDatabase = 65536;

// IO Manager block cache capacity
constexpr std::size_t kMinIOManagerBlockCacheCapacity = 50;
constexpr std::size_t kDefaultIOManagerBlockCacheCapacity = 103;

// IO Manager dead connection cleanup period in seconds
constexpr unsigned kMinIOManagerOptionDeadConnectionCleanupInterval = 3;
constexpr unsigned kMaxIOManagerOptionDeadConnectionCleanupInterval = 3600;
constexpr unsigned kDefaultIOManagerOptionDeadConnectionCleanupInterval = 15;

// Maximum JSON payload size
constexpr std::size_t kDefaultIOManagerOptionMaxJsonPayloadSize = 1024 * 1024;
constexpr std::size_t kMaxIOManagerOptionMaxJsonPayloadSize = 1024 * 1024 * 1024;

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
    int m_ipv4Port = kDefaultIpv4PortNumber;

    /** Instance IPv6 TCP port number */
    int m_ipv6Port = kDefaultIpv6PortNumber;

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

    /** Dead connection cleanup period */
    unsigned m_deadConnectionCleanupInterval = kDefaultOptionDeadConnectionCleanupInterval;

    /** Allow group permission of config files */
    bool m_allowGroupPermissionsOnConfigFiles = kDefaultOptionAllowGroupPermissionsOnConfigFiles;

    /** Ignore permissions of config files */
    bool m_ignorePermissionsOnConfigFiles = kDefaultOptionIgnorePermissionsOnConfigFiles;

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

    /** Enables REST Server service */
    bool m_enableRestServer = kDefaultOptionEnableRestServer;
};

/** IO Manager options */
struct IOManagerOptions {
    /** Worker thread number */
    std::size_t m_workerThreadNumber = kDefaultIOManagerWorkerThreadNumber;

    /** Writer thread number */
    std::size_t m_writerThreadNumber = kDefaultIOManagerWriterThreadNumber;

    /** IPv4 TCP SQL port number */
    int m_ipv4SqlPort = kDefaultIOManagerIpv4SqlPortNumber;

    /** IPv6 TCP SQL port number */
    int m_ipv6SqlPort = kDefaultIOManagerIpv6SqlPortNumber;

    /** IPv4 TCP REST port number */
    int m_ipv4RestPort = kDefaultIOManagerIpv4SqlPortNumber;

    /** IPv6 TCP REST port number */
    int m_ipv6RestPort = kDefaultIOManagerIpv6SqlPortNumber;

    /** Maximum number of users */
    std::uint32_t m_maxUsers = kDefaultIOManagerMaxUsers;

    /** Maximum number of databases */
    std::uint32_t m_maxDatabases = kDefaultIOManagerMaxDatabases;

    /** Maximum number of tables per database */
    std::uint32_t m_maxTableCountPerDatabase = kDefaultIOManagerMaxTablesPerDatabase;

    /** Block cache capacity */
    std::size_t m_blockCacheCapacity = kDefaultIOManagerBlockCacheCapacity;

    /** Dead connection cleanup period */
    unsigned m_deadConnectionCleanupInterval = kDefaultIOManagerOptionDeadConnectionCleanupInterval;

    /** Maximum JSON payload size */
    std::size_t m_maxJsonPayloadSize = kDefaultIOManagerOptionMaxJsonPayloadSize;
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

    /** Master cipher ID */
    std::string m_masterCipherId;

    /** Master key */
    std::string m_masterCipherKeyPath;

    /** System database cipher ID */
    std::string m_systemDbCipherId;

    /** Explicit master cipher key. Use this one only for unit tests. */
    BinaryValue m_masterCipherKey;

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
struct SiodbOptions {
    /** Initializes object of class SiodbOptions */
    SiodbOptions() noexcept
    {
        // Make GCC-8 happy
    }

    /**
     * Initializes object of class SiodbOptions.
     * @param instanceName Siodb instance name.
     */
    SiodbOptions(const std::string& instanceName)
    {
        load(instanceName);
    }

    /**
     * Initializes object of class SiodbOptions.
     * @param instanceName Siodb instance name.
     * @param configPath Configuration file path.
     */
    SiodbOptions(const std::string& instanceName, const std::string& configPath)
    {
        load(instanceName, configPath);
    }

    /**
     * Returns executable directory path.
     * @return Directory path.
     */
    std::string getExecutableDir() const;

    /**
     * Reads options for given instance.
     * @param instanceName Siodb instance name.
     * @throw InvalidConfigurationError
     *        - If configuration file can't be read.
     *        - If value of some configuration options is invalid.
     */
    void load(const std::string& instanceName);

    /**
     * Reads options for given instance.
     * @param instanceName Siodb instance name.
     * @param configPath Configuration file path.
     * @throw InvalidConfigurationError
     *        - If configuration file can't be read.
     *        - If value of some configuration options is invalid.
     */
    void load(const std::string& instanceName, const std::string& configPath);

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
};

/** Instance options shared pointer shortcut type */
using InstaceOptionsPtr = std::shared_ptr<SiodbOptions>;

/** Constant instance options shared pointer shortcut type */
using ConstInstaceOptionsPtr = std::shared_ptr<const SiodbOptions>;

}  // namespace siodb::config
