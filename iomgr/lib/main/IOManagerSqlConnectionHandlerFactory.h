// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerConnectionHandlerFactory.h"

namespace siodb::iomgr {

/** Produces SQL connection handler objects. */
class IOManagerSqlConnectionHandlerFactory : public IOManagerConnectionHandlerFactory {
public:
    /** 
     * Creates new connection handler object.
     * @param requestDispatcher Request dispatcher object.
     * @param clientFd Client connection file descriptor.
     * @return New connection handler object.
     */
    IOManagerConnectionHandler* createConnectionHandler(
            IOManagerRequestDispatcher& requestDispatcher, FdGuard&& clientFd) override;
};

}  // namespace siodb::iomgr
