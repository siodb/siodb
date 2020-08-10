// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerRequestDispatcher.h"

// Project headers
#include "IOManagerRequest.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/options/SiodbOptions.h>

namespace siodb::iomgr {

IOManagerRequestDispatcher::IOManagerRequestDispatcher(
        const config::SiodbOptions& options, dbengine::Instance& instance)
    : IOManagerRequestHandlerBase(kLogContextBase)
    , m_instance(instance)
    , m_requestExecutorPool(
              createRequestExecutorPool(options.m_ioManagerOptions.m_workerThreadNumber))
{
}

void IOManagerRequestDispatcher::handleRequest(const IOManagerRequestPtr& request)
{
    LOG_DEBUG << m_logContext << "Dispatching IO Manager request #" << request->getId();

    // For now here is simplest (but least effective) request dipatching solution:
    // just to post all requests to the first executor. Suitable for testing only.
    std::size_t executorId = 0;
    LOG_DEBUG << m_logContext << "Dispatching IO Manager request #" << request->getId()
              << " to the executor #" << executorId;
    m_requestExecutorPool.at(executorId)->addRequest(request);

    // TODO
    // Implement more advanced algorithm which should allow at least parallel SELECTs
    // on the same database and and parallel any type requests on the different datbases.
}

// --- internals

std::vector<std::unique_ptr<IOManagerRequestExecutor>>
IOManagerRequestDispatcher::createRequestExecutorPool(std::size_t size) const
{
    if (size == 0) throw std::runtime_error("Can't create request executor pool of size 0");
    LOG_DEBUG << m_logContext << "Creating request executor pool of size " << size;
    std::vector<std::unique_ptr<IOManagerRequestExecutor>> pool;
    pool.reserve(size);
    for (std::size_t id = 0; id < size; ++id) {
        LOG_DEBUG << m_logContext << "Creating request executor #" << id;
        pool.push_back(std::make_unique<IOManagerRequestExecutor>(id, m_instance));
    }
    return pool;
}

}  // namespace siodb::iomgr
