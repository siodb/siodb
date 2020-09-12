// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerRestConnectionHandlerFactory.h"

// Project headers
#include "IOManagerRestConnectionHandler.h"

// Common project headers
#include <siodb/common/log/Log.h>

namespace siodb::iomgr {

IOManagerConnectionHandler* IOManagerRestConnectionHandlerFactory::createConnectionHandler(
        IOManagerRequestDispatcher& requestDispatcher, FDGuard&& clientFd)
{
    DBG_LOG_DEBUG("Creating REST connection handler object...");
    return new IOManagerRestConnectionHandler(
            requestDispatcher, std::move(clientFd), m_maxJsonPayloadSize);
}

}  // namespace siodb::iomgr
