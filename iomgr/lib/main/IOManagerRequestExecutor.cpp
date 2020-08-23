// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerRequestExecutor.h"

// Project headers
#include "IOManagerConnectionHandler.h"
#include "IOManagerRequest.h"

// Common project headers
#include <siodb/common/log/Log.h>

namespace siodb::iomgr {

IOManagerRequestExecutor::IOManagerRequestExecutor(std::size_t id, dbengine::Instance& instance)
    : IOManagerRequestHandlerBase(createLogContextBaseString(id))
    , m_id(id)
    , m_instance(instance)
{
}

void IOManagerRequestExecutor::handleRequest(const IOManagerRequestPtr& request)
{
    LOG_DEBUG << m_logContext << "Executing IO Manager request #" << request->getId();
    IOManagerRequestExecutionResultAssignmentGuard guard(*request);
    const auto connectionHandler = request->getConnectionHanlder();
    if (connectionHandler) guard.setResult(connectionHandler->executeIOManagerRequest(*request));
}

std::string IOManagerRequestExecutor::createLogContextBaseString(std::size_t id)
{
    std::ostringstream oss;
    oss << kLogContextBase << '-' << id;
    return oss.str();
}

}  // namespace siodb::iomgr
