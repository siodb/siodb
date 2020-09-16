// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ProcessMonitor.h"
#include "SiodbConnectionManager.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/config/SiodbVersion.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/net/UnixServer.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/CheckOSUser.h>
#include <siodb/common/utils/FDGuard.h>
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/common/utils/SignalHandlers.h>
#include <siodb/common/utils/StartupActions.h>
#include <siodb/iomgr/shared/IOManagerExitCode.h>

// CRT headers
#include <cerrno>
#include <cstring>

// STL headers
#include <chrono>
#include <iostream>

// System headers
#include <sys/socket.h>
#include <unistd.h>

// Boost headers
#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

extern "C" int siodbMain(int argc, char** argv)
{
    // Must be called very first!
    siodb::utils::performCommonStartupActions();

    char* program = std::strrchr(argv[0], '/');
    program = program ? program + 1 : argv[0];
    if (argc < 2) {
        std::cerr << "Error: Not enough command line arguments." << std::endl;
        std::cerr << "Try " << program << " --help for more information." << std::endl;
        return 1;
    }

    bool runAsDaemon = false;
    auto instanceOptions = std::make_shared<siodb::config::SiodbOptions>();

    try {
        siodb::utils::checkUserBelongsToSiodbAdminGroup(geteuid(), getegid());

        boost::program_options::options_description desc("Allowed options");
        desc.add_options()("instance,i",
                boost::program_options::value<std::string>()->default_value(std::string()),
                "Instance name");
        desc.add_options()("daemon,d", "Run as daemon")("help,h", "Produce help message");

        boost::program_options::variables_map vm;
        boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help") > 0) {
            std::cout << desc << std::endl;
            return 0;
        }

        runAsDaemon = vm.count("daemon") > 0;

        const auto instanceName = vm["instance"].as<std::string>();
        if (instanceName.empty()) throw std::runtime_error("Instance name not defined");

        instanceOptions->load(instanceName);
        instanceOptions->m_logOptions.m_logFileBaseName = "siodb";

        std::vector<char> executableFullPath(PATH_MAX);
        if (::realpath(argv[0], executableFullPath.data()) == nullptr)
            throw std::runtime_error("Failed to obtain full path of the current executable.");
        instanceOptions->m_generalOptions.m_executablePath = executableFullPath.data();
    } catch (std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '.' << std::endl;
        return 2;
    }

    if (runAsDaemon) {
        if (daemon(0, 0) < 0) {
            // stdio/stderr may be closed or /dev/null, so just exit.
            return 3;
        }

        // daemon() in glibc does not perform double fork,
        // so now the process is a session leader,
        // and we need to fork() once again to give up session leadership.
        pid_t pid = fork();
        if (pid < 0) {
            // fork failed, but stderr is already /dev/null after daemon(), so just exit.
            return 3;
        }
        if (pid > 0) {
            // parent process
            return 0;
        }
        // child process, continue initialization
    }

    siodb::utils::setupSignalHandlers();

    int exitCode = 0;

    {
        std::unique_ptr<siodb::log::LogSubsystemGuard> logGuard;
        try {
            logGuard =
                    std::make_unique<siodb::log::LogSubsystemGuard>(instanceOptions->m_logOptions);
        } catch (std::exception& ex) {
            std::cerr << "Error: Can't initialize logging: " << ex.what() << '.' << std::endl;
            return 2;
        }

        LOG_INFO << "Siodb v." << SIODB_VERSION_MAJOR << '.' << SIODB_VERSION_MINOR << '.'
                 << SIODB_VERSION_PATCH
#ifdef _DEBUG
                 << " (debug build)"
#endif
                ;
        LOG_INFO << "Compiled on " << __DATE__ << ' ' << __TIME__;
        LOG_INFO << "Copyright (C) " << SIODB_COPYRIGHT_YEARS
                 << " Siodb GmbH. All rights reserved.";

        try {
            const auto lockFilePath = siodb::composeInstanceInitializationLockFilePath(
                    instanceOptions->m_generalOptions.m_name);

            // lockf() needs write permission
            siodb::FDGuard lockFile(::open(lockFilePath.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC,
                    siodb::kLockFileCreationMode));
            if (!lockFile.isValidFd())
                stdext::throw_system_error("Can't open or create initialization lock file");

            if (!lockFile.lock(F_TLOCK, 0))
                stdext::throw_system_error("Can't lock initialization lock file");

            const auto iomgrInitFlagFilePath = siodb::composeIomgrInitializionFlagFilePath(
                    instanceOptions->m_generalOptions.m_name);
            if (fs::exists(iomgrInitFlagFilePath)) {
                // IO manager should create flag file after initialization,
                if (!fs::remove(iomgrInitFlagFilePath)) {
                    stdext::throw_system_error(stdext::string_builder()
                                               << "Can't remove iomgr initialization file "
                                               << iomgrInitFlagFilePath);
                }
            }

            // Create IO Manager monitor
            std::unique_ptr<siodb::ProcessMonitor> iomgrMonitor;
            {
                std::vector<std::string> args;
                args.reserve(3);
                args.push_back(instanceOptions->getExecutableDir() + fs::path::preferred_separator
                               + siodb::kIOManagerExecutable);
                args.push_back("--instance");
                args.push_back(instanceOptions->m_generalOptions.m_name);

                std::vector<int> fatalExitCodes {
                        siodb::iomgr::kIOManagerExitCode_InvalidConfig,
                        siodb::iomgr::kIOManagerExitCode_LogInitializationFailed,
                        siodb::iomgr::kIOManagerExitCode_InitializationFailed,
                };

                iomgrMonitor = std::make_unique<siodb::ProcessMonitor>(
                        "IO Manager", std::move(args), std::move(fatalExitCodes), 2000, 10000, 300);
            }

            // Wait for IO Manager to initialize database
            while (!fs::exists(iomgrInitFlagFilePath) && iomgrMonitor->shouldRun())
                std::this_thread::sleep_for(siodb::kIomgrInitializationCheckPeriod);
            if (!iomgrMonitor->shouldRun())
                throw std::runtime_error("IO Manager exited unexpectedly");

            // Create REST Server monitor
            std::unique_ptr<siodb::ProcessMonitor> restServerMonitor;
            if (instanceOptions->m_generalOptions.m_enableRestServer) {
                std::vector<std::string> args;
                args.reserve(3);
                args.push_back(instanceOptions->getExecutableDir() + fs::path::preferred_separator
                               + siodb::kRestServerExecutable);
                args.push_back("--instance");
                args.push_back(instanceOptions->m_generalOptions.m_name);

                std::vector<int> fatalExitCodes {1, 2};

                restServerMonitor = std::make_unique<siodb::ProcessMonitor>("REST Server",
                        std::move(args), std::move(fatalExitCodes), 2000, 10000, 300);
            }

            siodb::SiodbConnectionManager adminConnectionManager(AF_UNIX, true, instanceOptions);

            std::unique_ptr<siodb::SiodbConnectionManager> ipv4UserConnectionManager;
            if (instanceOptions->m_generalOptions.m_ipv4Port != 0) {
                ipv4UserConnectionManager = std::make_unique<siodb::SiodbConnectionManager>(
                        AF_INET, false, instanceOptions);
            }

            std::unique_ptr<siodb::SiodbConnectionManager> ipv6UserConnectionManager;
            if (instanceOptions->m_generalOptions.m_ipv6Port != 0) {
                ipv6UserConnectionManager = std::make_unique<siodb::SiodbConnectionManager>(
                        AF_INET6, false, instanceOptions);
            }

            siodb::utils::waitForExitEvent();

            const int exitSignal = siodb::utils::getExitSignal();
            LOG_INFO << "Database instance is shutting down due to signal #" << exitSignal << " ("
                     << strsignal(exitSignal) << ").";
        } catch (std::exception& ex) {
            exitCode = 4;
            LOG_FATAL << ex.what();
        }
    }

    return exitCode;
}
