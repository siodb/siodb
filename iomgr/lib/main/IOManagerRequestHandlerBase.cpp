// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerRequestHandlerBase.h"

// Common project headers
#include <siodb/common/log/Log.h>

// System headers
#include <pthread.h>
#include <signal.h>

namespace siodb::iomgr {

IOManagerRequestHandlerBase::IOManagerRequestHandlerBase(std::string&& logContextBase)
    : m_logContext(std::move(logContextBase += ": "))
    , m_shouldRun(true)
    , m_thread(&IOManagerRequestHandlerBase::threadMain, this)
{
}

IOManagerRequestHandlerBase::~IOManagerRequestHandlerBase()
{
    m_shouldRun = false;
    {
        std::unique_lock lock(m_mutex);
        m_cond.notify_one();
    }
    if (m_thread.joinable()) {
        ::pthread_kill(m_thread.native_handle(), SIGUSR1);
        m_thread.join();
    }
}

void IOManagerRequestHandlerBase::addRequest(const IOManagerRequestPtr& request)
{
    std::unique_lock lock(m_mutex);
    m_requestQueue.push_back(request);
    m_cond.notify_one();
}

void IOManagerRequestHandlerBase::threadMain()
{
    LOG_INFO << m_logContext << "Worker thread started";
    while (m_shouldRun) {
        IOManagerRequestPtr request;
        {
            std::unique_lock lock(m_mutex);
            m_cond.wait(lock);
            if (!m_shouldRun) break;
            if (m_requestQueue.empty()) continue;
            request = m_requestQueue.front();
            m_requestQueue.pop_front();
        }
        handleRequest(request);
    }
    LOG_INFO << m_logContext << "Worker thread is exiting";
}

}  // namespace siodb::iomgr
