// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "IOManagerConnectionManager.h"
#include "../dbengine/Instance.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/config/SiodbVersion.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/CheckOSUser.h>
#include <siodb/common/utils/Debug.h>
#include <siodb/common/utils/FsUtils.h>
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/common/utils/MessageCatalog.h>
#include <siodb/common/utils/SignalHandlers.h>
#include <siodb/common/utils/StartupActions.h>
#include <siodb/iomgr/shared/IOManagerExitCode.h>
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/Cipher.h>

// CRT headers
#include <ctime>

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

void writeInitializationFailureLog(int exitCode, const char* errorMessage);

extern "C" int iomgrMain(int argc, char** argv)
{
    // Must be called very first!
    siodb::utils::performCommonStartupActions();

    std::string instanceName;
    auto instanceOptions = std::make_shared<siodb::config::SiodbOptions>();

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
            return siodb::iomgr::kIOManagerExitCode_Success;
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
        writeInitializationFailureLog(siodb::iomgr::kIOManagerExitCode_InvalidConfig, ex.what());
        return siodb::iomgr::kIOManagerExitCode_InvalidConfig;
    }

    siodb::utils::setupSignalHandlers();

    {
        std::unique_ptr<siodb::log::LogSubsystemGuard> logGuard;
        try {
            logGuard =
                    std::make_unique<siodb::log::LogSubsystemGuard>(instanceOptions->m_logOptions);
        } catch (std::exception& ex) {
            std::cerr << "Fatal: " << ex.what() << '.' << std::endl;
            return siodb::iomgr::kIOManagerExitCode_LogInitializationFailed;
        }

        LOG_INFO << "Siodb IO Manager v." << SIODB_VERSION_MAJOR << '.' << SIODB_VERSION_MINOR
                 << '.' << SIODB_VERSION_PATCH
#ifdef _DEBUG
                 << " (debug build)"
#endif
                ;
        LOG_INFO << "Compiled on " << __DATE__ << ' ' << __TIME__;
        LOG_INFO << "Copyright (C) " << SIODB_COPYRIGHT_YEARS
                 << " Siodb GmbH. All rights reserved.";

        siodb::iomgr::dbengine::InstancePtr instance;
        std::unique_ptr<siodb::iomgr::IOManagerRequestDispatcher> requestDispatcher;
        std::unique_ptr<siodb::iomgr::IOManagerConnectionManager> ipv4ConnectionManager;
        std::unique_ptr<siodb::iomgr::IOManagerConnectionManager> ipv6ConnectionManager;

        try {
            LOG_INFO << "Initializing database message catalog...";
            siodb::utils::MessageCatalog::initDefaultCatalog(siodb::utils::constructPath(
                    instanceOptions->getExecutableDir(), "iomgr_messages.txt"));

            LOG_INFO << "Initializing built-in ciphers...";
            siodb::iomgr::dbengine::crypto::initializeBuiltInCiphers();

            LOG_INFO << "Initializing external ciphers...";
            siodb::iomgr::dbengine::crypto::initializeExternalCiphers(
                    instanceOptions->m_encryptionOptions.m_externalCipherOptions);

            LOG_INFO << "Initializing database engine...";
            instance = std::make_shared<siodb::iomgr::dbengine::Instance>(*instanceOptions);

            LOG_INFO << "Initializing request dispatcher and executors...";
            requestDispatcher = std::make_unique<siodb::iomgr::IOManagerRequestDispatcher>(
                    *instanceOptions, *instance);

            if (instanceOptions->m_ioManagerOptions.m_ipv4port != 0) {
                LOG_INFO << "Initializing IPv4 connection manager...";
                ipv4ConnectionManager = std::make_unique<siodb::iomgr::IOManagerConnectionManager>(
                        AF_INET, instanceOptions, *requestDispatcher);
            }

            if (instanceOptions->m_ioManagerOptions.m_ipv6port != 0) {
                LOG_INFO << "Initializing IPv6 connection manager...";
                ipv6ConnectionManager = std::make_unique<siodb::iomgr::IOManagerConnectionManager>(
                        AF_INET6, instanceOptions, *requestDispatcher);
            }

            LOG_INFO << "Creating initialization flag file...";
            const auto initFlagFilePath = siodb::composeIomgrInitializionFlagFilePath(
                    instanceOptions->m_generalOptions.m_name);
            if (!fs::exists(initFlagFilePath)) {
                // Signal to siodb process that database is initialized and checked.
                siodb::FdGuard lockFile(
                        ::open(initFlagFilePath.c_str(), O_CREAT, siodb::kLockFileCreationMode));
                if (!lockFile.isValidFd())
                    stdext::throw_system_error("Can't create iomgr initialization file");
            }
        } catch (std::exception& ex) {
            LOG_ERROR << ex.what() << '.' << std::endl;
            writeInitializationFailureLog(
                    siodb::iomgr::kIOManagerExitCode_InitializationFailed, ex.what());
            return siodb::iomgr::kIOManagerExitCode_InitializationFailed;
        }

        LOG_INFO << "IO Manager initialized";

        siodb::utils::waitForExitEvent();

        const int exitSignal = siodb::utils::getExitSignal();
        LOG_INFO << "IO Manager is shutting down due to signal #" << exitSignal << " ("
                 << strsignal(exitSignal) << ").";

        // Make shutdown process more detailed in the log
        if (ipv6ConnectionManager) {
            LOG_INFO << "Shutting down IPv6 connection manager...";
            ipv6ConnectionManager.reset();
        }

        if (ipv4ConnectionManager) {
            LOG_INFO << "Shutting down IPv4 connection manager...";
            ipv4ConnectionManager.reset();
        }

        LOG_INFO << "Shutting down request dispatcher...";
        requestDispatcher.reset();

        LOG_INFO << "Shutting down database engine...";
        instance.reset();
    }

    return siodb::iomgr::kIOManagerExitCode_Success;
}

void writeInitializationFailureLog(int exitCode, const char* errorMessage)
{
    std::string logPath;
    {
        std::ostringstream oss;
        oss << "/tmp/siodb_iomgr_init_failure_" << std::time(nullptr) << '_' << ::getpid()
            << ".log";
        logPath = oss.str();
    }
    std::ofstream ofs(logPath);
    if (ofs.is_open()) {
        ofs << "Exit code: " << exitCode << '\n';
        ofs << "Fatal: " << errorMessage << ".\n";
        ofs << std::flush;
        ofs.close();
    } else {
        std::cerr << "Warning: Can't open log file " << logPath << std::endl;
    }
}
