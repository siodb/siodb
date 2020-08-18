// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerRestConnectionHandlerFactory.h"

// Project headers
#include "IOManagerRestConnectionHandler.h"

namespace siodb::iomgr {

IOManagerConnectionHandler* IOManagerRestConnectionHandlerFactory::createConnectionHandler(
        IOManagerRequestDispatcher& requestDispatcher, FdGuard&& clientFd)
{
    return new IOManagerRestConnectionHandler(requestDispatcher, std::move(clientFd));
}

}  // namespace siodb::iomgr
