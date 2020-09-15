// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerConnectionHandler.h"
#include "../dbengine/parser/DBEngineRestRequestFactory.h"

namespace siodb::iomgr {

class IOManagerRestConnectionHandler : public IOManagerConnectionHandler {
public:
    /**
     * Initializes object of class IOManagerSqlConnectionHandler.
     * @param clientFd Client connection file descriptor guard.
     * @param requestDispatcher Request dispatcher.
     * @param maxJsonPayloadSize Maximum JSON payload size.
     */
    IOManagerRestConnectionHandler(IOManagerRequestDispatcher& requestDispatcher,
            FDGuard&& clientFd, std::size_t maxJsonPayloadSize)
        : IOManagerConnectionHandler(requestDispatcher, std::move(clientFd))
        , m_requestFactory(maxJsonPayloadSize)
    {
    }

private:
    /** Thread logic implementation. */
    void threadLogicImpl() override;

private:
    /** Request factory object */
    dbengine::parser::DBEngineRestRequestFactory m_requestFactory;

private:
    /** REST authentication error code */
    static constexpr int kRestAuthenticationError = 11;
    /** REST parse error code */
    static constexpr int kRestParseError = 12;
};

}  // namespace siodb::iomgr
