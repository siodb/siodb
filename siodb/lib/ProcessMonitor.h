// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/common/utils/SignalHandlers.h>

// STL headers
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// System headers
#include <sys/types.h>

// Boost headers
#include <boost/circular_buffer.hpp>

namespace siodb {

/** ProcessMonitor monitors execution of a child process. */
class ProcessMonitor {
public:
    /**
     * Initializes object of class ProcessMonitor.
     * @param processName User visible process name.
     * @param args execve arguments used to run new process.
     * @param fatalExitCodes List of fatal exit codes.
     * @param statusCheckIntervalMs Process status check interval in milliseconds.
     * @param terminatonTimeoutMs Process termination timeout in milliseconds.
     * @param minTimeBetweenRestarts Minimum time in seconds between two restarts that is considered normal.
     */
    ProcessMonitor(const char* processName, std::vector<std::string>&& args,
            std::vector<int>&& fatalExitCodes, unsigned statusCheckIntervalMs,
            unsigned terminatonTimeoutMs, unsigned minTimeBetweenRestarts);

    /** De-initializes object of class ProcessMonitor. */
    ~ProcessMonitor();

    DECLARE_NONCOPYABLE(ProcessMonitor);

    /**
     * Returns indication that monitor thread and controlled process should run.
     * @return true if monitor thread and controlled process should run, false if they should exit.
     */
    bool shouldRun() const;

private:
    /** Starts contolled process */
    void startProcess();

    /** Stops controlled process */
    void stopProcess();

    /** Stops monitor thread */
    void stopThread();

    /** Monitor thread main function. */
    void threadMain();

private:
    /** User visible process name */
    const std::string m_processName;

    /** execve arguments */
    const std::vector<std::string> m_args;

    /** List of fatal exit codes, after which restart is infeasible */
    const std::vector<int> m_fatalExitCodes;

    /** Controlled process ID */
    pid_t m_pid;

    /** Monitor thread data access synchronization object */
    mutable std::mutex m_monitorMutex;

    /** Monitor thread awake condition */
    std::condition_variable m_monitorThreadAwakeCondition;

    /** Run state indicator */
    bool m_running;

    /** Interval between checks that process is active */
    const std::chrono::milliseconds m_statusCheckInterval;

    /** Controlled process termination timeout */
    const std::chrono::milliseconds m_terminatonTimeout;

    /** Period of checking controlled process status during termination */
    const std::chrono::milliseconds m_statusCheckIntervalDuringTermination;

    /** Minimal time between first and last process restarts */
    const std::chrono::seconds m_minTimeBetweenRestarts;

    /** Number of restart history entries */
    const std::size_t m_restartHistorySize;

    /** Circular buffer of the process start times */
    boost::circular_buffer<std::chrono::steady_clock::time_point> m_restartHistory;

    /** Monitor thread. Must be the last non-static member variable in this class. */
    std::thread m_thread;

    /** Log message prefix */
    static constexpr const char* kLogPrefix = "ProcessMonitor: ";
};

}  // namespace siodb
