// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "WorkerBase.h"

// Common project headers
#include <siodb/common/log/Log.h>

// STL headers
#include <sstream>

// System headers
#include <pthread.h>
#include <signal.h>

namespace siodb::iomgr {

WorkerBase::WorkerBase(const char* workerType, std::size_t workerId)
    : m_workerId(workerId)
    , m_logContext(createLogContextString(workerType, workerId))
    , m_exitRequested(false)
{
}

WorkerBase::~WorkerBase()
{
    m_exitRequested = true;

    // Stop worker thread
    if (m_thread && m_thread->joinable()) {
        ::pthread_kill(m_thread->native_handle(), SIGUSR1);
        m_thread->join();
    }
}

void WorkerBase::start()
{
    if (m_thread) throw std::runtime_error("Worker thread is already created");

    m_thread = std::make_unique<std::thread>(&WorkerBase::workerThreadEntryPoint, this);
}

void WorkerBase::workerThreadEntryPoint()
{
    LOG_INFO << m_logContext << "Worker thread started.";
    workerThreadMain();
}

std::string WorkerBase::createLogContextString(const char* workerType, std::size_t workerId)
{
    std::ostringstream str;
    str << workerType << '-' << workerId << ": ";
    return str.str();
}

}  // namespace siodb::iomgr
