// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/common/utils/SignalHandlers.h>

// STL headers
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

// System headers
#include <sys/types.h>

// Boost headers
#include <boost/circular_buffer.hpp>

namespace siodb {

/** IO manager monitor */
class IOManagerMonitor final {
public:
    /**
     * Initialized object of class IOManagerMonitor.
     * @param instanceOptions Database options.
     */
    IOManagerMonitor(const config::ConstInstaceOptionsPtr& instanceOptions);

    /** Cleans up object */
    ~IOManagerMonitor();

    DECLARE_NONCOPYABLE(IOManagerMonitor);

    /**
     * Returns indication that monitor thread and IO manager should run.
     * @return true if monitor thread and IO Manager should run, false if it should exit.
     */
    bool shouldRun() const
    {
        std::lock_guard lock(m_monitorMutex);
        return m_running && !utils::isExitEventSignaled();
    }

private:
    /** Starts IO Manager */
    void startIOManager();

    /** Stops IO Manager */
    void stopIOManager();

    /** Stops IO Manager monitor thread */
    void stopThread();

    /** IO Manage monitor thread function. */
    void threadMain();

private:
    /** Database options */
    const config::ConstInstaceOptionsPtr m_dbOptions;

    /** Process ID of the IO Manager */
    pid_t m_iomgrPid;

    /** Monitor thread data access synchronization object */
    mutable std::mutex m_monitorMutex;

    /** Monitor thread awake condition */
    std::condition_variable m_monitorThreadAwakeCondition;

    /** Run state indicator */
    bool m_running;

    /** Circular buffer of Iomgr starts */
    boost::circular_buffer<std::chrono::steady_clock::time_point> m_startsHistory;

    /** Monitor thread. Must be the last non-static member variable in this class. */
    std::thread m_thread;

    /** Period of checking that IO manager is active */
    static constexpr std::chrono::milliseconds kWaitPeriod = std::chrono::milliseconds(2000);

    /** IO Manager termination timeout */
    static constexpr std::chrono::milliseconds kIOManagerTerminatonTimeout =
            std::chrono::milliseconds(10000);

    /** Period of checking IO Manager status when requested to terminate */
    static constexpr std::chrono::milliseconds kIOManagerStatusCheckPeriod =
            std::chrono::milliseconds(100);

    /** Minimal time between first and last IoMgrStart */
    static constexpr std::chrono::seconds kIOManagerMinTimeBetweenRestarts =
            std::chrono::seconds(300);

    /** Size of @ref m_startsHistory */
    static constexpr std::size_t kIoManagerHistorySize = 3;

    /** Log message prefix */
    static constexpr const char* kLogPrefix = "IOManagerMonitor: ";
};

}  // namespace siodb
