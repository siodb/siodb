// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/DatabaseError.h"
#include "dbengine/Instance.h"
#include "dbengine/crypto/GetCipher.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FsUtils.h>
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
#include <boost/algorithm/string.hpp>

// Google Test
#include <gtest/gtest.h>

namespace {

const char* argv0;
std::mutex m_commonInitializationMutex;
bool m_commonInitializationDone;

void CreateAndLoadInstance(const char* cipherId)
{
    std::cout << "Filling database instance options..." << std::endl;

    // Create options object
    siodb::config::SiodbOptions instanceOptions;

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
    siodb::log::LogSubsystemGuard logGuard(instanceOptions.m_logOptions);

    LOG_INFO << "Base directory: " << baseDir;

    do {
        // Guard this part of initialization
        std::lock_guard lock(m_commonInitializationMutex);
        if (m_commonInitializationDone) break;

        // Initialize DB message catalog.
        LOG_INFO << "Initializing database message catalog...";
        siodb::utils::MessageCatalog::initDefaultCatalog(siodb::utils::constructPath(
                instanceOptions.getExecutableDir(), "iomgr_messages.txt"));

        // Initialize ciphers
        LOG_INFO << "Initializing built-in ciphers...";
        siodb::iomgr::dbengine::crypto::initializeBuiltInCiphers();
        LOG_INFO << "Initializing external ciphers...";
        siodb::iomgr::dbengine::crypto::initializeExternalCiphers(
                instanceOptions.m_encryptionOptions.m_externalCipherOptions);

        m_commonInitializationDone = true;
    } while (false);

    instanceOptions.m_encryptionOptions.m_defaultCipherId = cipherId;
    instanceOptions.m_encryptionOptions.m_systemDbCipherId = cipherId;

    const auto cipher = siodb::iomgr::dbengine::crypto::getCipher(
            instanceOptions.m_encryptionOptions.m_systemDbCipherId);
    if (cipher) {
        LOG_INFO << "Filling encryption key...";
        const auto keyLength = cipher->getKeySize() / 8;
        auto& key = instanceOptions.m_encryptionOptions.m_systemDbCipherKey;
        key.resize(keyLength);
        std::fill_n(key.begin(), key.size(), 111);
    }

    // Create instance
    try {
        LOG_INFO << "========================================";
        LOG_INFO << "= ";
        LOG_INFO << "Creating database instance...";
        LOG_INFO << "= ";
        LOG_INFO << "========================================";
        auto instance = std::make_unique<siodb::iomgr::dbengine::Instance>(instanceOptions);
        LOG_INFO << "Instance " << instance->getUuid() << " created." << std::endl;
    } catch (siodb::iomgr::dbengine::DatabaseError& ex) {
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
        auto instance = std::make_unique<siodb::iomgr::dbengine::Instance>(instanceOptions);
        LOG_INFO << "Instance " << instance->getUuid() << " loaded." << std::endl;
    } catch (siodb::iomgr::dbengine::DatabaseError& ex) {
        LOG_ERROR << '[' << ex.getErrorCode() << "] " << ex.what() << '\n'
                  << ex.getStackTraceAsString();
        throw std::runtime_error("Instance initialization failed, DatabaseError");
    } catch (std::exception& ex) {
        LOG_ERROR << ex.what();
        throw std::runtime_error("Instance initialization failed, other std error");
    }
}

}  // anonymous namespace

TEST(DBEngine, CreateAndLoadInstance_WithoutEncryption)
{
    CreateAndLoadInstance(siodb::iomgr::dbengine::crypto::kNoCipherId);
}

TEST(DBEngine, CreateAndLoadInstance_WithEncryption)
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
