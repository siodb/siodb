// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerConnectionHandler.h"

namespace siodb::iomgr {

class IOManagerRestConnectionHandler : public IOManagerConnectionHandler {
public:
    /**
     * Initializes object of class IOManagerSqlConnectionHandler.
     * @param clientFd Client connection file descriptor guard.
     * @param requestDispatcher Request dispatcher.
     */
    IOManagerRestConnectionHandler(
            IOManagerRequestDispatcher& requestDispatcher, FDGuard&& clientFd)
        : IOManagerConnectionHandler(requestDispatcher, std::move(clientFd))
    {
    }

private:
    /** Thread logic implementation. */
    void threadLogicImpl() override;

private:
    /** REST authentication error code */
    static constexpr int kRestAuthenticationError = 11;
    /** REST parse error code */
    static constexpr int kRestParseError = 12;
};

}  // namespace siodb::iomgr
