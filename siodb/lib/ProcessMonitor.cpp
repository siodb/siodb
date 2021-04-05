// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ProcessMonitor.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/common/stl_ext/utility_ext.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>

// STL headers
#include <iostream>
#include <sstream>

// System headers
#include <sys/wait.h>
#include <unistd.h>

namespace siodb {

ProcessMonitor::ProcessMonitor(const char* processName, std::vector<std::string>&& args,
        std::vector<int>&& fatalExitCodes, unsigned statusCheckIntervalMs,
        unsigned terminatonTimeoutMs, unsigned minTimeBetweenRestarts)
    : m_processName(processName)
    , m_args(std::move(args))
    , m_fatalExitCodes(std::move(fatalExitCodes))
    , m_pid(-1)
    , m_running(true)
    , m_statusCheckInterval(statusCheckIntervalMs)
    , m_terminatonTimeout(terminatonTimeoutMs)
    , m_statusCheckIntervalDuringTermination(500)
    , m_minTimeBetweenRestarts(minTimeBetweenRestarts)
    , m_restartHistorySize(3)
    , m_restartHistory(m_restartHistorySize)
    // IMPORTANT: Thread initialization must be in the end
    , m_thread(&ProcessMonitor::threadMain, this)
{
}

ProcessMonitor::~ProcessMonitor()
{
    try {
        LOG_INFO << kLogPrefix << "Shutting down.";
        stopThread();
    } catch (std::exception& ex) {
        try {
            LOG_ERROR << kLogPrefix << "Shutdown error: " << ex.what();
        } catch (...) {
            // Ignore errors here.
        }
    }
}

bool ProcessMonitor::shouldRun() const
{
    std::lock_guard lock(m_monitorMutex);
    return m_running && !utils::isExitEventSignaled();
}

// ----- internals ----

void ProcessMonitor::startProcess()
{
    LOG_INFO << kLogPrefix << "Starting child process " << m_processName;
    m_pid = ::fork();
    if (m_pid == 0) {
        // Prepare user connection worker command-line parameters
        std::vector<char*> execArgs(m_args.size() + 1);
        std::transform(m_args.cbegin(), m_args.cend(), execArgs.begin(),
                [](const auto& s) noexcept { return stdext::as_mutable_ptr(s.c_str()); });
        char* envp[] = {nullptr};

        // Child process
        ::execve(execArgs.front(), execArgs.data(), envp);
        // If we have reached here, execve() failed.
        const int errorCode = errno;
        const char* errorMessage = std::strerror(errno);
        std::cerr << "Can't execute " << m_processName << ": (" << errorCode
                  << "): " << errorMessage << std::endl;
        _exit(-1);
    } else if (m_pid < 0) {
        stdext::throw_system_error(errno, "Can't fork");
    } else {
        m_restartHistory.push_back(std::chrono::steady_clock::now());
        LOG_INFO << kLogPrefix << "Started child process " << m_processName;
    }
}

void ProcessMonitor::stopProcess()
{
    LOG_INFO << kLogPrefix << "Stopping " << m_processName;
    if (m_pid > 0) {
        bool needToKill = false;
        if (::kill(m_pid, SIGTERM) < 0) {
            const int errorCode = errno;
            LOG_ERROR << kLogPrefix << "Sending SIGTERM to the " << m_processName
                      << " failed: " << errorCode << ' ' << std::strerror(errorCode) << ".";
            needToKill = true;
        }

        auto remainingTime = m_terminatonTimeout;
        if (!needToKill) {
            int waitResult = 0, status = 0;
            while (remainingTime > std::chrono::milliseconds(0)) {
                waitResult = ::waitpid(!m_pid, &status, WNOHANG);
                if (waitResult != 0) break;
                std::this_thread::sleep_for(m_statusCheckIntervalDuringTermination);
                remainingTime -= m_statusCheckIntervalDuringTermination;
            }
            needToKill = waitResult == 0 && remainingTime <= std::chrono::milliseconds(0);
        }

        if (needToKill) {
            LOG_INFO << kLogPrefix << m_processName
                     << " could not be stopped with SIGTERM. Killing it.";
            if (::kill(m_pid, SIGKILL) < 0) {
                const int errorCode = errno;
                std::ostringstream err;
                err << "Sending SIGKILL to " << m_processName << " failed";
                stdext::throw_system_error(errorCode, err.str());
            }
        }
    }
}

void ProcessMonitor::stopThread()
{
    LOG_INFO << kLogPrefix << "Stopping " << m_processName << " monitor thread";
    // Signal monitor thread to wake it up and finish
    {
        std::lock_guard lock(m_monitorMutex);
        m_running = false;
        m_monitorThreadAwakeCondition.notify_one();
    }
    if (m_thread.joinable()) m_thread.join();
    LOG_INFO << kLogPrefix << m_processName << " monitor thread stopped.";
}

void ProcessMonitor::threadMain()
{
    while (shouldRun() && m_pid <= 0) {
        try {
            startProcess();
        } catch (std::exception& ex) {
            LOG_ERROR << "Can't start " << m_processName << ": " << ex.what();
        }
        if (m_pid <= 0) std::this_thread::sleep_for(m_statusCheckInterval);
    }

    while (shouldRun()) {
        try {
            int status = 0;
            const auto pid = waitpid(m_pid, &status, WNOHANG);
            const int errorCode = errno;
            if (pid == 0) {
                // Process still running, wait for m_statusCheckInterval or
                // m_monitorThreadAwakeCondition event
                std::unique_lock lock(m_monitorMutex);
                m_monitorThreadAwakeCondition.wait_for(lock, m_statusCheckInterval);
            } else if (pid < 0) {
                // Wait failed
                LOG_ERROR << kLogPrefix << "Waiting for the " << m_processName
                          << " status failed: " << errorCode << ' ' << strerror(errorCode);
            } else if (pid == m_pid) {
                // Process exited
                m_pid = -1;
                const auto exitStatus = WEXITSTATUS(status);
                LOG_WARNING << kLogPrefix << m_processName << " (PID " << pid
                            << ") has unexpectedly exited with status " << exitStatus;

                // Restart process if exit code isn't fatal and restarts aren't too often
                bool restartProcess =
                        std::find(m_fatalExitCodes.cbegin(), m_fatalExitCodes.cend(), exitStatus)
                        == m_fatalExitCodes.cend();

                if (restartProcess && m_restartHistory.full()) {
                    const auto timeBetweenStarts = std::chrono::duration_cast<std::chrono::seconds>(
                            m_restartHistory.back() - m_restartHistory.front());
                    restartProcess = timeBetweenStarts >= m_minTimeBetweenRestarts;
                    if (!restartProcess) {
                        LOG_ERROR << kLogPrefix << m_processName
                                  << " has been restarted too many times in a period of "
                                  << timeBetweenStarts.count()
                                  << " seconds. This may indicate a persistent issue. Giving up on "
                                     "restarting "
                                  << m_processName << '.';
                    }
                }

                if (restartProcess) {
                    if (shouldRun()) startProcess();
                } else {
                    {
                        std::lock_guard lock(m_monitorMutex);
                        m_running = false;
                    }
                    LOG_INFO << kLogPrefix << "Given up on restarting monitored process "
                             << m_processName << ". Raising SIGINT.";
                    ::raise(SIGINT);
                }
            }
        } catch (std::exception& ex) {
            LOG_ERROR << kLogPrefix << ex.what();
        }
    }  // while shouldRun()

    if (m_pid > 0) {
        try {
            stopProcess();
        } catch (std::exception& ex) {
            LOG_ERROR << kLogPrefix << "Can't stop " << m_processName << ": " << ex.what();
            LOG_WARNING << "Killing " << m_processName << " process.";
            ::kill(m_pid, SIGKILL);
        }
    }

    LOG_INFO << kLogPrefix << m_processName << " monitor thread stopped.";
}

}  // namespace siodb
