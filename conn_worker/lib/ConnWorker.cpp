// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConnWorker.h"

// Project headers
#include "ConnWorkerConnectionHandler.h"

// Common project headers
#include <siodb/common/config/SiodbVersion.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/utils/CheckOSUser.h>
#include <siodb/common/utils/Debug.h>
#include <siodb/common/utils/FdGuard.h>
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/common/utils/SignalHandlers.h>
#include <siodb/common/utils/StartupActions.h>

// STL headers
#include <iostream>

// System headers
#include <sys/types.h>
#include <unistd.h>

// Boost headers
#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace {

std::unique_ptr<siodb::conn_worker::ConnWorkerConnectionHandler> g_connectionHandler;

int run(int argc, char** argv);

}  // namespace

extern "C" int connWorkerMain(int argc, char** argv)
{
    // Must be called very first!
    siodb::utils::performCommonStartupActions();

    const int exitCode = run(argc, argv);
    DEBUG_TRACE("conn_worker exits with code " << exitCode);
    return exitCode;
}

namespace {

int run(int argc, char** argv)
{
    auto instanceOptions = std::make_shared<siodb::config::SiodbOptions>();
    std::string instanceName;
    siodb::FdGuard client;
    bool adminMode = false;

    // Parse and validate command-line options
    try {
        siodb::utils::checkUserBelongsToSiodbAdminGroup(geteuid(), getegid());

        boost::program_options::options_description desc("Allowed options");
        desc.add_options()("admin", "Administrator mode");
        desc.add_options()("instance",
                boost::program_options::value<std::string>()->default_value(std::string()),
                "Instance name");
        desc.add_options()("client-fd", boost::program_options::value<int>()->default_value(-1),
                "Client file descriptor number");
        desc.add_options()("help,h", "Produce help message");

        boost::program_options::variables_map vm;
        boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help") > 0) {
            std::cout << desc << std::endl;
            return 0;
        }

        instanceName = vm["instance"].as<std::string>();
        if (instanceName.empty()) {
            throw std::runtime_error("Instance name not specified");
        }
        siodb::validateInstance(instanceName);

        instanceOptions->load(instanceName);
        instanceOptions->m_logOptions.m_logFileBaseName = "conn_worker";

        std::vector<char> executableFullPath(PATH_MAX);
        if (::realpath(argv[0], executableFullPath.data()) == nullptr) {
            throw std::runtime_error("Failed to obtain full path of the current executable.");
        }
        instanceOptions->m_generalOptions.m_executablePath = executableFullPath.data();

        const auto fd = vm["client-fd"].as<int>();
        if (fd < 3) throw std::runtime_error("Invalid client file descriptor");

        client.reset(fd);

        adminMode = vm.count("admin") > 0;
    } catch (std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '.' << std::endl;
        return 1;
    }

    siodb::utils::setupSignalHandlers(&terminationSignalHandler);

    // Work with connection
    {
        std::unique_ptr<siodb::log::LogSubsystemGuard> logGuard;
        try {
            logGuard =
                    std::make_unique<siodb::log::LogSubsystemGuard>(instanceOptions->m_logOptions);
        } catch (std::exception& ex) {
            std::cerr << "Error: " << ex.what() << '.' << std::endl;
            return 2;
        }

        LOG_INFO << "Siodb Connection Worker v." << SIODB_VERSION_MAJOR << '.'
                 << SIODB_VERSION_MINOR << '.' << SIODB_VERSION_PATCH
#ifdef _DEBUG
                 << " (debug build)"
#endif
                ;
        LOG_INFO << "Compiled on " << __DATE__ << ' ' << __TIME__;
        LOG_INFO << "Copyright (C) " << SIODB_COPYRIGHT_YEARS
                 << " Siodb GmbH. All rights reserved.";

        try {
            g_connectionHandler = std::make_unique<siodb::conn_worker::ConnWorkerConnectionHandler>(
                    std::move(client), instanceOptions, adminMode);
            g_connectionHandler->run();
        } catch (std::exception& ex) {
            LOG_ERROR << "Error: " << ex.what() << '.' << std::endl;
            return 2;
        }
    }

    return 0;
}

void terminationSignalHandler([[maybe_unused]] int signal)
{
    if (g_connectionHandler) {
        g_connectionHandler->closeConnection();
        DEBUG_TRACE("ConnWorker: Closed connection.");
    }
}

}  // anonymous namespace
