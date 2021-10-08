// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandlerTest_TestEnv.h"

// Project headers
#include "dbengine/DatabaseError.h"
#include "dbengine/Table.h"
#include "dbengine/User.h"
#include "dbengine/handlers/RequestHandler.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"

// Common project headers
#include <siodb/common/io/FDStream.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/stl_ext/sstream_ext.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/MessageCatalog.h>
#include <siodb/common/utils/StartupActions.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/ConstantExpression.h>

// CRT headers
#include <ctime>

// System headers
#include <unistd.h>

// Boost
#include <boost/algorithm/string/case_conv.hpp>

// Google Test
#include <gtest/gtest.h>

TestEnvironment* TestEnvironment::m_env;
std::string TestEnvironment::m_testDatabaseName;
std::string TestEnvironment::m_testDatabaseNameLowerCase;
std::array<std::string, TestEnvironment::kTestUserCount> TestEnvironment::m_testUserNames;
std::array<std::uint32_t, TestEnvironment::kTestUserCount> TestEnvironment::m_testUserIds;

TestEnvironment::TestEnvironment(const char* argv0)
    : m_argv0(argv0)
{
    m_env = this;
}

std::unique_ptr<dbengine::RequestHandler> TestEnvironment::makeRequestHandlerForNormalUser(
        std::size_t testUserIndex)
{
    return makeRequestHandler(m_testUserIds[testUserIndex]);
}

std::unique_ptr<dbengine::RequestHandler> TestEnvironment::makeRequestHandlerForUser(
        const std::string& userName)
{
    const auto user = getInstance()->findUserChecked(userName);
    return makeRequestHandler(user->getId());
}

std::unique_ptr<dbengine::RequestHandler> TestEnvironment::makeRequestHandlerForSuperUser()
{
    return makeRequestHandler(dbengine::User::kSuperUserId);
}

std::unique_ptr<dbengine::RequestHandler> TestEnvironment::makeRequestHandler(std::uint32_t userId)
{
    return std::make_unique<dbengine::RequestHandler>(*m_env->m_instance, *m_env->m_output, userId);
}

void TestEnvironment::SetUp()
{
    std::cout << "Filling database instance options..." << std::endl;

    // Create options object
    siodb::config::SiodbOptions instanceOptions;

    // Fill executable path
    std::vector<char> executableFullPath(PATH_MAX);
    if (::realpath(m_argv0, executableFullPath.data()) == nullptr)
        throw std::runtime_error("Failed to obtain full path of the current executable.");
    instanceOptions.m_generalOptions.m_executablePath = executableFullPath.data();

    // Fill general options
    const auto home = ::getenv("HOME");
    const auto baseDir = stdext::concat(home, "/tmp/siodb_", std::time(nullptr), '_', ::getpid());
    std::cout << "Base directory: " << baseDir << std::endl;

    m_instanceFolder = baseDir;
    instanceOptions.m_generalOptions.m_dataDirectory = baseDir + "/data";
    instanceOptions.m_generalOptions.m_allowCreatingUserTablesInSystemDatabase = true;
    instanceOptions.m_generalOptions.m_superUserInitialAccessKey =
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIMiRClOWfWD4kC6cy5IvxscUm17g5ECaXDUe5KVuIFEz "
            "root@siodb";

    // Fill encryption options
    //using siodb::config::defaults::kDefaultCipherId;
    instanceOptions.m_encryptionOptions.m_defaultCipherId = "none";
    instanceOptions.m_encryptionOptions.m_masterCipherId = "none";
    instanceOptions.m_encryptionOptions.m_systemDbCipherId = "none";

    // Fill log options
    instanceOptions.m_logOptions.m_logFileBaseName = "iomgr";
    {
        siodb::config::LogChannelOptions channel;

#if 1
        // Uncomment if required for debug
        channel.m_name = "console";
        channel.m_type = siodb::config::LogChannelType::kConsole;
        channel.m_destination = "stdout";
        channel.m_severity = boost::log::trivial::info;
        instanceOptions.m_logOptions.m_logChannels.push_back(channel);
#endif

        channel.m_name = "file";
        channel.m_type = siodb::config::LogChannelType::kFile;
        channel.m_destination = baseDir + "/log";
        channel.m_severity = boost::log::trivial::debug;
        instanceOptions.m_logOptions.m_logChannels.push_back(channel);
    }

    // Initialize logging
    siodb::log::initLogging(instanceOptions.m_logOptions);

    LOG_INFO << "Base directory: " << baseDir;

    // Initialize DB message catalog.
    LOG_INFO << "Initializing database message catalog...";
    siodb::utils::MessageCatalog::initDefaultCatalog(
            siodb::utils::constructPath(instanceOptions.getExecutableDir(), "iomgr_messages.txt"));

    // Initialize ciphers
    LOG_INFO << "Initializing built-in ciphers...";
    dbengine::crypto::initializeBuiltInCiphers();
    LOG_INFO << "Initializing external ciphers...";
    dbengine::crypto::initializeExternalCiphers(
            instanceOptions.m_encryptionOptions.m_externalCipherOptions);

    // Create instance
    try {
        LOG_INFO << "Creating database instance...";
        m_instance = std::make_shared<dbengine::Instance>(instanceOptions);
        std::cout << "Instance " << m_instance->getUuid() << " initialized." << std::endl;
    } catch (dbengine::DatabaseError& ex) {
        LOG_ERROR << '[' << ex.getErrorCode() << "] " << ex.what() << '\n'
                  << ex.getStackTraceAsString();
        throw std::runtime_error("Instance initialization failed, DatabaseError");
    } catch (std::exception& ex) {
        LOG_ERROR << ex.what();
        throw std::runtime_error("Instance initialization failed, other std error");
    }

    ASSERT_EQ(::pipe(m_pipes), 0);

    // Make pipe larger than maximal data chunk
    constexpr int kPipeSize = 1024 * 1024;
    LOG_INFO << "Expanding pipe buffer to the " << kPipeSize << " bytes";
    ASSERT_EQ(::fcntl(m_pipes[1], F_SETPIPE_SZ, kPipeSize), kPipeSize);

    m_input = std::make_unique<siodb::io::FDStream>(m_pipes[0], true);
    m_output = std::make_unique<siodb::io::FDStream>(m_pipes[1], true);

    // User name must be in UPPERCASE
    for (std::size_t i = 0; i < kTestUserCount; ++i) {
        m_testUserNames[i] = stdext::concat("TEST_USER_", std::to_string(std::time(nullptr)), '_',
                std::to_string(::getpid()), '_', i);
        m_testUserIds[i] =
                m_instance
                        ->createUser(m_testUserNames[i], {}, {}, true, dbengine::User::kSuperUserId)
                        ->getId();
    }

    // Database name must be in UPPERCASE
    m_testDatabaseName =
            "TEST_DB_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(::getpid());
    m_testDatabaseNameLowerCase = boost::to_upper_copy(m_testDatabaseName);
    siodb::BinaryValue key(16, 0xAB);
    const auto database = m_instance->createDatabase(stdext::copy(m_testDatabaseName),
            std::string("aes128"), std::move(key), {},
            std::numeric_limits<std::uint32_t>::max() / 2, {}, false, dbengine::User::kSuperUserId);
    const auto sysTables = database->findTableChecked(dbengine::kSysTablesTableName);

    // Grant permissions to users
    const int usersToGrantPermissionsTo[] = {0, 2};
    for (const int userIndex : usersToGrantPermissionsTo) {
        m_instance->grantObjectPermissionsToUser(m_testUserIds[userIndex], 0,
                dbengine::DatabaseObjectType::kDatabase, database->getId(),
                dbengine::kShowPermissionMask, false, dbengine::User::kSuperUserId);
        m_instance->grantObjectPermissionsToUser(m_testUserIds[userIndex], database->getId(),
                dbengine::DatabaseObjectType::kTable, 0, dbengine::kCreatePermissionMask, false,
                dbengine::User::kSuperUserId);
        m_instance->grantObjectPermissionsToUser(m_testUserIds[userIndex], database->getId(),
                dbengine::DatabaseObjectType::kTable, sysTables->getId(),
                dbengine::kSelectSystemPermissionMask, false, dbengine::User::kSuperUserId);
    }
}

void TestEnvironment::TearDown()
{
    ::close(m_pipes[0]);
    ::close(m_pipes[1]);

    // instance destructor prints data to the log
    m_instance.reset();
    siodb::log::shutdownLogging();

    // keep resoures only if at least one test is failed.
    if (testing::UnitTest::GetInstance()->Passed()) fs::remove_all(m_instanceFolder);
}
