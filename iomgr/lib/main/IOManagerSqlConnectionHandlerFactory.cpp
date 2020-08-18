// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerSqlConnectionHandlerFactory.h"

// Project headers
#include "IOManagerSqlConnectionHandler.h"

namespace siodb::iomgr {

IOManagerConnectionHandler* IOManagerSqlConnectionHandlerFactory::createConnectionHandler(
        IOManagerRequestDispatcher& requestDispatcher, FdGuard&& clientFd)
{
    return new IOManagerSqlConnectionHandler(requestDispatcher, std::move(clientFd));
}

}  // namespace siodb::iomgr
