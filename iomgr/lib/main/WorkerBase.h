// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IORequest.h"

// Common project headers
#include <siodb/common/utils/HelperMacros.h>

// STL header
#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

namespace siodb::iomgr {

/** Base class for all IO manager worker threads */
class WorkerBase {
public:
    /**
     * Initializes object of class Worker.
     * @param workerType Worker type.
     * @param workerId Worker identifier.
     */
    WorkerBase(const char* workerType, std::size_t workerId);

    /** De-initializes object */
    virtual ~WorkerBase();

    DECLARE_NONCOPYABLE(WorkerBase);

    /**
     * Returns worker ID.
     * @return Worker ID.
     */
    std::size_t getWorkerId() const noexcept
    {
        return m_workerId;
    }

protected:
    /** Worker thread main function */
    virtual void workerThreadMain() = 0;

    /** Returns exit requested flag */
    bool isExitRequested() const noexcept
    {
        return m_exitRequested;
    }

    /** Starts worker thread.
     * @throw std::system_error if the thread could not be starteв or already started.
     */
    void start();

private:
    /** Worker thread entry point */
    void workerThreadEntryPoint();

    /**
     * Creates log context string.
     * @param workerType Worker type.
     * @param workerId Worker identifier.
     */
    static std::string createLogContextString(const char* workerType, std::size_t workerId);

protected:
    /** Worker ID */
    const std::size_t m_workerId;

    /** Log context */
    const std::string m_logContext;

    /** IO request queue access synchronization object */
    mutable std::mutex m_ioRequestQueueMutex;

    /** IO request queue signaling facility */
    std::condition_variable m_ioRequestQueueCond;

    /** IO request queue */
    std::deque<std::unique_ptr<IORequest>> m_ioRequestQueue;

private:
    /** Worker thread exit request */
    std::atomic<bool> m_exitRequested;

    /** Worker thread */
    std::unique_ptr<std::thread> m_thread;
};

}  // namespace siodb::iomgr
