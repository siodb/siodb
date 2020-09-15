// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/DatabaseError.h"
#include "dbengine/Instance.h"
#include "dbengine/User.h"
#include "dbengine/crypto/GetCipher.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/MessageCatalog.h>
#include <siodb/common/utils/StartupActions.h>
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/Cipher.h>

// CRT headers
#include <ctime>

// STL headers
#include <iostream>
#include <random>

// System headers
#include <unistd.h>

// Boost headers
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>

// Google Test
#include <gtest/gtest.h>

namespace {

const char* argv0;
std::mutex m_commonInitializationMutex;
bool m_commonInitializationDone;

void CreateAndLoadInstance(const char* cipherId)
{
    using namespace siodb;
    using namespace siodb::iomgr::dbengine;
    std::cout << "Filling database instance options..." << std::endl;

    // Create options object
    config::SiodbOptions instanceOptions;

    std::cout << "Filling general options..." << std::endl;

    // Fill executable path
    std::vector<char> executableFullPath(PATH_MAX);
    if (::realpath(argv0, executableFullPath.data()) == nullptr) {
        throw std::runtime_error("Failed to obtain full path of the current executable.");
    }
    instanceOptions.m_generalOptions.m_executablePath = executableFullPath.data();

    // Fill general options
    const auto home = ::getenv("HOME");
    const std::string baseDir = stdext::string_builder()
                                << home << "/tmp/siodb_" << std::time(nullptr) << '_' << ::getpid();
    instanceOptions.m_generalOptions.m_dataDirectory = baseDir + "/data";
    instanceOptions.m_generalOptions.m_superUserInitialAccessKey =
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIMiRClOWfWD4kC6cy5IvxscUm17g5ECaXDUe5KVuIFEz "
            "root@siodb";

    // Fill log options
    std::cout << "Filling log options..." << std::endl;
    instanceOptions.m_logOptions.m_logFileBaseName = "iomgr";
    {
        config::LogChannelOptions channel;

        channel.m_name = "console";
        channel.m_type = config::LogChannelType::kConsole;
        channel.m_destination = "stdout";
        channel.m_severity = boost::log::trivial::debug;
        instanceOptions.m_logOptions.m_logChannels.push_back(channel);

        channel.m_name = "file";
        channel.m_type = config::LogChannelType::kFile;
        channel.m_destination = baseDir + "/log";
        instanceOptions.m_logOptions.m_logChannels.push_back(channel);
    }

    // Initialize logging
    log::LogSubsystemGuard logGuard(instanceOptions.m_logOptions);

    LOG_INFO << "Base directory: " << baseDir;

    do {
        // Guard this part of initialization
        std::lock_guard lock(m_commonInitializationMutex);
        if (m_commonInitializationDone) break;

        // Initialize DB message catalog.
        LOG_INFO << "Initializing database message catalog...";
        utils::MessageCatalog::initDefaultCatalog(siodb::utils::constructPath(
                instanceOptions.getExecutableDir(), "iomgr_messages.txt"));

        // Initialize ciphers
        LOG_INFO << "Initializing built-in ciphers...";
        crypto::initializeBuiltInCiphers();
        LOG_INFO << "Initializing external ciphers...";
        crypto::initializeExternalCiphers(
                instanceOptions.m_encryptionOptions.m_externalCipherOptions);

        m_commonInitializationDone = true;
    } while (false);

    instanceOptions.m_encryptionOptions.m_defaultCipherId = cipherId;
    instanceOptions.m_encryptionOptions.m_masterCipherId = cipherId;
    instanceOptions.m_encryptionOptions.m_systemDbCipherId = cipherId;

    const auto cipher = crypto::getCipher(instanceOptions.m_encryptionOptions.m_systemDbCipherId);
    if (cipher) {
        LOG_INFO << "Filling encryption key...";
        const auto keyLength = cipher->getKeySizeInBits() / 8;
        auto& key = instanceOptions.m_encryptionOptions.m_masterCipherKey;
        key.resize(keyLength);
        std::fill_n(key.begin(), key.size(), 0xEF);
    }

    // Create instance
    const std::string userName("USER1");
    const std::string tokenName("TOKEN1");
    std::string tokenStr;
    std::uint32_t userId;
    try {
        LOG_INFO << "========================================";
        LOG_INFO << "= ";
        LOG_INFO << "Creating database instance...";
        LOG_INFO << "= ";
        LOG_INFO << "========================================";
        auto instance = std::make_unique<Instance>(instanceOptions);
        LOG_INFO << "Instance " << instance->getUuid() << " created." << std::endl;
        userId = instance->createUser(
                userName, std::nullopt, std::nullopt, true, User::kSuperUserId);
        LOG_INFO << "Created user #" << userId;
        const auto userToken = instance->createUserToken(
                userName, tokenName, std::nullopt, std::nullopt, std::nullopt, User::kSuperUserId);
        tokenStr.resize(userToken.second.size() * 2);
        boost::algorithm::hex_lower(
                userToken.second.cbegin(), userToken.second.cend(), tokenStr.begin());
        const auto authenticatedUserId = instance->authenticateUser(userName, tokenStr);
        EXPECT_EQ(authenticatedUserId, userId);
    } catch (DatabaseError& ex) {
        LOG_ERROR << '[' << ex.getErrorCode() << "] " << ex.what() << '\n'
                  << ex.getStackTraceAsString();
        throw std::runtime_error("Instance initialization failed, DatabaseError");
    } catch (std::exception& ex) {
        LOG_ERROR << ex.what();
        throw std::runtime_error("Instance initialization failed, other std error");
    }

    // Load existing instance
    try {
        LOG_INFO << "========================================";
        LOG_INFO << "= ";
        LOG_INFO << "= LOADING DATABASE INSTANCE";
        LOG_INFO << "= ";
        LOG_INFO << "========================================";
        auto instance = std::make_unique<Instance>(instanceOptions);
        LOG_INFO << "Instance " << instance->getUuid() << " loaded." << std::endl;
        const auto authenticatedUserId = instance->authenticateUser(userName, tokenStr);
        EXPECT_EQ(authenticatedUserId, userId);
    } catch (DatabaseError& ex) {
        LOG_ERROR << '[' << ex.getErrorCode() << "] " << ex.what() << '\n'
                  << ex.getStackTraceAsString();
        throw std::runtime_error("Instance initialization failed, DatabaseError");
    } catch (std::exception& ex) {
        LOG_ERROR << ex.what();
        throw std::runtime_error("Instance initialization failed, other std error");
    }
}

}  // anonymous namespace

TEST(TokenAuthenticationTest, AuthenticateWithTokenAfterInstanceReload)
{
    CreateAndLoadInstance(siodb::config::kDefaultCipherId);
}

int main(int argc, char** argv)
{
    // Must be called very first!
    siodb::utils::performCommonStartupActions();

    // Save executable
    argv0 = argv[0];

    // Run tests
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
