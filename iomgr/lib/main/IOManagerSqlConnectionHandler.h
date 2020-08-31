// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerConnectionHandler.h"
#include "../dbengine/AuthenticationResult.h"

namespace siodb::iomgr {

class IOManagerSqlConnectionHandler : public IOManagerConnectionHandler {
public:
    /**
     * Initializes object of class IOManagerSqlConnectionHandler.
     * @param clientFd Client connection file descriptor guard.
     * @param requestDispatcher Request dispatcher.
     */
    IOManagerSqlConnectionHandler(IOManagerRequestDispatcher& requestDispatcher, FDGuard&& clientFd)
        : IOManagerConnectionHandler(requestDispatcher, std::move(clientFd))
    {
    }

private:
    /** Thread logic implementation. */
    void threadLogicImpl() override;

    /** 
     * Authenticates user on the connection.
     * @return Pair of (user ID, session UUID).
     */
    dbengine::AuthenticationResult authenticateUser();

private:
    /** SQL parse error message code */
    static constexpr int kSqlParseError = 2;
};

}  // namespace siodb::iomgr
