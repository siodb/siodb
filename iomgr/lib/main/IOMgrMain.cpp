// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "IOMgrConnectionManager.h"
#include "../dbengine/Instance.h"
#include "../dbengine/crypto/ciphers/Cipher.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/config/SiodbVersion.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/options/DatabaseInstance.h>
#include <siodb/common/options/InstanceOptions.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/CheckOSUser.h>
#include <siodb/common/utils/Debug.h>
#include <siodb/common/utils/FsUtils.h>
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/common/utils/MessageCatalog.h>
#include <siodb/common/utils/SignalHandlers.h>
#include <siodb/common/utils/StartupActions.h>
#include <siodb/common/utils/StringBuilder.h>
#include <siodb/common/utils/SystemError.h>
#include <siodb/iomgr/shared/IOManagerExitCode.h>

// STL headers
#include <iostream>

// System headers
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Boost headers
#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

extern "C" int iomgrMain(int argc, char** argv)
{
    // Must be called very first!
    siodb::utils::performCommonStartupActions();

    std::string instanceName;
    auto instanceOptions = std::make_shared<siodb::config::InstanceOptions>();

    // Parse and validate command-line options
    try {
        siodb::utils::checkUserBelongsToSiodbAdminGroup(geteuid(), getegid());

        boost::program_options::options_description desc("Allowed options");
        desc.add_options()("instance",
                boost::program_options::value<std::string>()->default_value(std::string()),
                "Instance name");
        desc.add_options()("help,h", "Produce help message");

        boost::program_options::variables_map vm;
        boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help") > 0) {
            std::cout << desc << std::endl;
            return siodb::kIOManagerExitCode_Success;
        }

        instanceName = vm["instance"].as<std::string>();
        if (instanceName.empty()) throw std::runtime_error("Instance name not specified");
        siodb::validateInstance(instanceName);

        instanceOptions->load(instanceName);
        instanceOptions->m_logOptions.m_logFileBaseName = "iomgr";

        std::vector<char> executableFullPath(PATH_MAX);
        if (::realpath(argv[0], executableFullPath.data()) == nullptr)
            throw std::runtime_error("Failed to obtain full path of the current executable.");
        instanceOptions->m_generalOptions.m_executablePath = executableFullPath.data();
    } catch (std::exception& ex) {
        std::cerr << "Fatal: " << ex.what() << '.' << std::endl;
        return siodb::kIOManagerExitCode_InvalidConfig;
    }

    siodb::utils::setupSignalHandlers();

    {
        std::unique_ptr<siodb::log::LogSubsystemGuard> logGuard;
        try {
            logGuard =
                    std::make_unique<siodb::log::LogSubsystemGuard>(instanceOptions->m_logOptions);
        } catch (std::exception& ex) {
            std::cerr << "Fatal: " << ex.what() << '.' << std::endl;
            return siodb::kIOManagerExitCode_LogInitializationFailed;
        }

        LOG_INFO << "Siodb IO Manager v." << SIODB_VERSION_MAJOR << '.' << SIODB_VERSION_MINOR
                 << '.' << SIODB_VERSION_PATCH << '.';
        LOG_INFO << "Copyright (C) " << SIODB_COPYRIGHT_YEARS
                 << " Siodb GmbH. All rights reserved.";

        try {
            // Initialize DB message catalog.
            LOG_INFO << "Initializing database message catalog...";
            siodb::utils::MessageCatalog::initDefaultCatalog(siodb::utils::constructPath(
                    instanceOptions->getExecutableDir(), "iomgr_messages.txt"));
        } catch (std::exception& ex) {
            LOG_ERROR << ex.what() << '.' << std::endl;
            return siodb::kIOManagerExitCode_InitializationFailed;
        }

        try {
            // Initialize ciphers
            LOG_INFO << "Initializing built-in ciphers...";
            siodb::iomgr::dbengine::crypto::initializeBuiltInCiphers();
            LOG_INFO << "Initializing external ciphers...";
            siodb::iomgr::dbengine::crypto::initializeExternalCiphers(
                    instanceOptions->m_encryptionOptions.m_externalCipherOptions);
        } catch (std::exception& ex) {
            LOG_FATAL << ex.what() << '.' << std::endl;
            return siodb::kIOManagerExitCode_InitializationFailed;
        }

        siodb::iomgr::dbengine::InstancePtr instance;
        try {
            instance = std::make_shared<siodb::iomgr::dbengine::Instance>(*instanceOptions);
        } catch (std::exception& ex) {
            LOG_FATAL << ex.what() << '.' << std::endl;
            return siodb::kIOManagerExitCode_DatabaseEngineIntializationFailed;
        }

        std::unique_ptr<siodb::iomgr::IOMgrConnectionManager> ipv4UserConnectionManager;
        std::unique_ptr<siodb::iomgr::IOMgrConnectionManager> ipv6UserConnectionManager;

        try {
            // Initialize IPv4 listener
            if (instanceOptions->m_ioManagerOptions.m_ipv4port != 0) {
                ipv4UserConnectionManager = std::make_unique<siodb::iomgr::IOMgrConnectionManager>(
                        AF_INET, instanceOptions, instance);
            }

            // Initialize IPv6 listener
            if (instanceOptions->m_ioManagerOptions.m_ipv6port != 0) {
                ipv6UserConnectionManager = std::make_unique<siodb::iomgr::IOMgrConnectionManager>(
                        AF_INET6, instanceOptions, instance);
            }
        } catch (std::exception& ex) {
            LOG_FATAL << ex.what() << '.' << std::endl;
            return siodb::kIOManagerExitCode_ConnectionCreationFailed;
        }

        try {
            const auto initFlagFilePath = siodb::composeIomgrInitializionFlagFilePath(
                    instanceOptions->m_generalOptions.m_name);
            if (!fs::exists(initFlagFilePath)) {
                // Signal to siodb process that database is initialized and checked.
                siodb::FileDescriptorGuard lockFile(
                        ::open(initFlagFilePath.c_str(), O_CREAT, siodb::kLockFileCreationMode));
                if (!lockFile.isValidFd())
                    siodb::utils::throwSystemError("Can't create iomgr initialization file");
            }
        } catch (std::exception& ex) {
            LOG_FATAL << ex.what() << '.' << std::endl;
            return siodb::kIOManagerExitCode_InitializationFailed;
        }

        LOG_INFO << "IO Manager initialized";

        siodb::utils::waitForExitEvent();
        const int exitSignal = siodb::utils::getExitSignal();
        LOG_INFO << "IO Manager is shutting down due to signal #" << exitSignal << " ("
                 << strsignal(exitSignal) << ").";
    }

    return siodb::kIOManagerExitCode_Success;
}
