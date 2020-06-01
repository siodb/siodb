// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandlerTest_TestEnv.h"

// Project headers
#include "dbengine/DatabaseError.h"
#include "dbengine/Table.h"
#include "dbengine/handlers/RequestHandler.h"
#include "dbengine/parser/DBEngineRequestFactory.h"
#include "dbengine/parser/expr/ConstantExpression.h"

// Common project headers
#include <siodb/common/io/FdIo.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/options/InstanceOptions.h>
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FsUtils.h>
#include <siodb/common/utils/MessageCatalog.h>
#include <siodb/common/utils/StartupActions.h>

// CRT headers
#include <ctime>

// System headers
#include <unistd.h>

// Google Test
#include <gtest/gtest.h>

TestEnvironment* TestEnvironment::m_env;

std::unique_ptr<dbengine::RequestHandler> TestEnvironment::makeRequestHandler()
{
    return std::make_unique<dbengine::RequestHandler>(
            *m_env->m_instance, *m_env->m_output, dbengine::User::kSuperUserId);
}

void TestEnvironment::SetUp()
{
    std::cout << "Filling database instance options..." << std::endl;

    // Create options object
    siodb::config::InstanceOptions instanceOptions;

    // Fill executable path
    std::vector<char> executableFullPath(PATH_MAX);
    if (::realpath(m_argv0, executableFullPath.data()) == nullptr)
        throw std::runtime_error("Failed to obtain full path of the current executable.");
    instanceOptions.m_generalOptions.m_executablePath = executableFullPath.data();

    // Fill general options
    const auto home = ::getenv("HOME");
    const std::string baseDir = stdext::string_builder()
                                << home << "/tmp/siodb_" << std::time(nullptr) << '_' << ::getpid();

    m_instanceFolder = baseDir;
    instanceOptions.m_generalOptions.m_dataDirectory = baseDir + "/data";
    instanceOptions.m_generalOptions.m_allowCreatingUserTablesInSystemDatabase = true;
    instanceOptions.m_generalOptions.m_superUserInitialAccessKey =
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIMiRClOWfWD4kC6cy5IvxscUm17g5ECaXDUe5KVuIFEz "
            "root@siodb";

    // Fill encryption options
    //using siodb::config::defaults::kDefaultCipherId;
    instanceOptions.m_encryptionOptions.m_defaultCipherId = "none";
    instanceOptions.m_encryptionOptions.m_systemDbCipherId = "none";

    // Fill log options
    instanceOptions.m_logOptions.m_logFileBaseName = "iomgr";
    {
        siodb::config::LogChannelOptions channel;

        channel.m_name = "console";
        channel.m_type = siodb::config::LogChannelType::kConsole;
        channel.m_destination = "stdout";
        channel.m_severity = boost::log::trivial::debug;
        instanceOptions.m_logOptions.m_logChannels.push_back(channel);

        channel.m_name = "file";
        channel.m_type = siodb::config::LogChannelType::kFile;
        channel.m_destination = baseDir + "/log";
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

    m_input = std::make_unique<siodb::io::FdIo>(m_pipes[0], true);
    m_output = std::make_unique<siodb::io::FdIo>(m_pipes[1], true);
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
