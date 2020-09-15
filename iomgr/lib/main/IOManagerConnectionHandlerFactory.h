// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerConnectionHandler.h"

namespace siodb::iomgr {

/** 
 * Connection handler abstract factory interface.
 * Conneciton handler factory produces connection handler objects.
 */
class IOManagerConnectionHandlerFactory {
public:
    /** De-initializes object of class IOManagerConnectionHandlerFactory */
    virtual ~IOManagerConnectionHandlerFactory() = default;

    /** 
     * Creates new connection handler object.
     * @param requestDispatcher Request dispatcher object.
     * @param clientFd Client connection file descriptor.
     * @return New connection handler object.
     */
    virtual IOManagerConnectionHandler* createConnectionHandler(
            IOManagerRequestDispatcher& requestDispatcher, FDGuard&& clientFd) = 0;
};

}  // namespace siodb::iomgr
