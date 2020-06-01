// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOMgrMonitor.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/common/stl_ext/utility_ext.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/iomgr/shared/IOManagerExitCode.h>

// System headers
#include <sys/wait.h>
#include <unistd.h>

namespace siodb {

namespace {
const IOManagerExitCode kFatalErrorCodes[] = {
        kIOManagerExitCode_InvalidConfig,
        kIOManagerExitCode_DatabaseEngineIntializationFailed,
        kIOManagerExitCode_ConnectionCreationFailed,
        kIOManagerExitCode_LogInitializationFailed,
        kIOManagerExitCode_InitializationFailed,
};
}  // namespace

/** IO Manager monitor */
IOMgrMonitor::IOMgrMonitor(const config::ConstInstaceOptionsPtr& instanceOptions)
    : m_dbOptions(instanceOptions)
    , m_iomgrPid(-1)
    , m_running(true)
    , m_startsHistory(kIoManagerHistorySize)
    // IMPORTANT: Thread initialization must be in the end
    , m_thread(&IOMgrMonitor::threadMain, this)
{
}

IOMgrMonitor::~IOMgrMonitor()
{
    try {
        LOG_INFO << kLogPrefix << "Shutting down.";
        stopMonitorThread();
    } catch (std::exception& ex) {
        try {
            LOG_ERROR << kLogPrefix << "Shutdown error: " << ex.what();
        } catch (...) {
            // Ignore errors here.
        }
    }
}

void IOMgrMonitor::startIOManager()
{
    LOG_INFO << kLogPrefix << "Starting IO Manager";
    m_iomgrPid = ::fork();
    if (m_iomgrPid == 0) {
        // Prepare user connection worker command-line parameters
        const std::string ioMgrExecutableName = m_dbOptions->getExecutableDir()
                                                + fs::path::preferred_separator
                                                + kIOManagerExecutable;

        const std::vector<std::string> args = {
                ioMgrExecutableName,
                "--instance",
                m_dbOptions->m_generalOptions.m_name,
        };

        std::vector<char*> execArgs(args.size() + 1);
        std::transform(args.cbegin(), args.cend(), execArgs.begin(),
                [](const auto& s) noexcept { return stdext::as_mutable_ptr(s.c_str()); });
        char* envp[] = {nullptr};

        // Child process
        ::execve(execArgs.front(), execArgs.data(), envp);
        // If we have reached here, execve() failed.
        _exit(-1);

    } else if (m_iomgrPid < 0) {
        stdext::throw_system_error(errno, "Can't fork for IO Manager process");
    } else {
        m_startsHistory.push_back(std::chrono::steady_clock::now());
        LOG_INFO << kLogPrefix << "Started IO Manager";
    }
}

void IOMgrMonitor::stopIOManager()
{
    LOG_INFO << kLogPrefix << "Stopping IO Manager.";
    if (m_iomgrPid > 0) {
        bool needSigKill = false;
        if (::kill(m_iomgrPid, SIGTERM) < 0) {
            const int errorCode = errno;
            LOG_ERROR << kLogPrefix << "Sending SIGTERM to IO Manager failed: " << errorCode << ' '
                      << std::strerror(errorCode) << ".";
            needSigKill = true;
        }

        auto remainingTime = kIOManagerTerminatonTimeout;
        if (!needSigKill) {
            int status = 0;
            while (remainingTime > std::chrono::milliseconds(0)) {
                const int waitpidResult = waitpid(!m_iomgrPid, &status, WNOHANG);
                if (waitpidResult != 0)
                    break;
                else {
                    std::this_thread::sleep_for(kIOManagerStatusCheckPeriod);
                    remainingTime -= kIOManagerStatusCheckPeriod;
                }
            }
            needSigKill = remainingTime <= std::chrono::milliseconds(0);
        }

        if (needSigKill) {
            LOG_INFO << kLogPrefix
                     << "IO Manager process could not be stopped with SIGTERM. Killing it.";
            if (::kill(m_iomgrPid, SIGKILL) < 0)
                stdext::throw_system_error(errno, "Sending SIGKILL to IO Manager failed");
        }
    }
}

void IOMgrMonitor::stopMonitorThread()
{
    LOG_INFO << kLogPrefix << "Stopping IO Manager monitor thread";
    // Signal monitor thread to wake it up and finish
    {
        std::lock_guard lock(m_monitorMutex);
        m_running = false;
        m_monitorThreadAwakeCondition.notify_one();
    }
    if (m_thread.joinable()) m_thread.join();
    LOG_INFO << kLogPrefix << "IO Manager monitor thread stopped.";
}

void IOMgrMonitor::threadMain()
{
    while (shouldRun() && m_iomgrPid <= 0) {
        try {
            startIOManager();
        } catch (std::exception& ex) {
            LOG_ERROR << "Can't start IO manager: " << ex.what();
        }
        if (m_iomgrPid <= 0) std::this_thread::sleep_for(kWaitPeriod);
    }

    while (shouldRun()) {
        try {
            int status = 0;
            const auto pid = waitpid(m_iomgrPid, &status, WNOHANG);
            const int errorCode = errno;
            if (pid == 0) {
                // Process still running, wait for kWaitPeriod or
                // m_monitorThreadAwakeCondition event
                std::unique_lock lock(m_monitorMutex);
                m_monitorThreadAwakeCondition.wait_for(lock, kWaitPeriod);
            } else if (pid < 0) {
                // Wait failed
                LOG_ERROR << kLogPrefix << "Waiting for IO Manager status failed: " << errorCode
                          << ' ' << strerror(errorCode);
            } else if (pid == m_iomgrPid) {
                // Process exited
                m_iomgrPid = -1;
                const auto exitStatus = WEXITSTATUS(status);
                LOG_WARNING << kLogPrefix << "IO Manager (PID " << pid
                            << ") has unexpectedly exited with status " << exitStatus;

                // Restart Iomgr if restarts aren't too often and error code isn't fatal
                bool restartIomgr = std::find(std::begin(kFatalErrorCodes),
                                            std::end(kFatalErrorCodes), exitStatus)
                                    == std::end(kFatalErrorCodes);

                if (restartIomgr && m_startsHistory.full()) {
                    const auto timeBetweenStarts = std::chrono::duration_cast<std::chrono::seconds>(
                            m_startsHistory.back() - m_startsHistory.front());
                    restartIomgr = timeBetweenStarts >= kIOManagerMinTimeBetweenRestarts;
                    if (!restartIomgr) {
                        LOG_ERROR << kLogPrefix
                                  << "IO Manager has been restarted too many times in a period of "
                                  << timeBetweenStarts.count()
                                  << " seconds. This may indicate a persistent issue."
                                  << " Giving up on restarting iomgr.";
                    }
                }

                if (restartIomgr) {
                    if (shouldRun()) startIOManager();
                } else {
                    {
                        std::lock_guard lock(m_monitorMutex);
                        m_running = false;
                    }
                    ::raise(SIGINT);
                }
            }
        } catch (std::exception& ex) {
            LOG_ERROR << kLogPrefix << ex.what();
        }
    }  // while shouldRun()

    if (m_iomgrPid > 0) {
        try {
            stopIOManager();
        } catch (std::exception& ex) {
            LOG_ERROR << kLogPrefix << "Can't stop IO Manager: " << ex.what();
            LOG_WARNING << "Killing IO Manager process.";
            ::kill(m_iomgrPid, SIGKILL);
        }
    }

    LOG_INFO << kLogPrefix << "IO Manager monitor thread stopped.";
}

}  // namespace siodb
