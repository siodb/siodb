// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerRequestPtr.h"

// Common project headers
#include <siodb/common/utils/HelperMacros.h>

// STL headers
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace siodb::iomgr {

/** Base class for the handlers of incoming requests from clients. */
class IOManagerRequestHandlerBase {
protected:
    /** 
     * Initializes object of class IOManagerRequestHandlerBase.
     * @param logContextBase Log context name base string.
     */
    explicit IOManagerRequestHandlerBase(std::string&& logContextBase);

public:
    /** De-initializes object */
    virtual ~IOManagerRequestHandlerBase();

    DECLARE_NONCOPYABLE(IOManagerRequestHandlerBase);

    /**
     * Adds request to the request queue.
     * @param request A request.
     */
    void addRequest(const IOManagerRequestPtr& request);

protected:
    /**
     * Handles single request.
     * @param request A request to be handled.
     */
    virtual void handleRequest(const IOManagerRequestPtr& request) = 0;

private:
    /** Request handler thread entry point. */
    void threadMain();

protected:
    /** Log context name */
    const std::string m_logContext;

private:
    /** Access synchronization object for the requesн queue */
    std::mutex m_mutex;

    /** Access synchronization object for the requesн queue */
    std::condition_variable m_cond;

    /** Request queue */
    std::deque<IOManagerRequestPtr> m_requestQueue;

    /** Indication that thread should run */
    std::atomic_bool m_shouldRun;

    /** Request handler thread, must be the last member variable in this class. */
    std::thread m_thread;
};

}  // namespace siodb::iomgr
