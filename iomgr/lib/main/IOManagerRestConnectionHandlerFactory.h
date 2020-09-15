// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerConnectionHandlerFactory.h"

namespace siodb::iomgr {

/** Produces REST connection handler objects. */
class IOManagerRestConnectionHandlerFactory : public IOManagerConnectionHandlerFactory {
public:
    /**
     * Initializes object of class IOManagerRestConnectionHandlerFactory.
     * @param maxJsonPayloadSize Maximum JSON payload size.
     */
    explicit IOManagerRestConnectionHandlerFactory(std::size_t maxJsonPayloadSize) noexcept
        : m_maxJsonPayloadSize(maxJsonPayloadSize)
    {
    }

    /** 
     * Creates new connection handler object.
     * @param requestDispatcher Request dispatcher object.
     * @param clientFd Client connection file descriptor.
     * @return New connection handler object.
     */
    IOManagerConnectionHandler* createConnectionHandler(
            IOManagerRequestDispatcher& requestDispatcher, FDGuard&& clientFd) override;

private:
    /** Maximum JSON payload size */
    const std::size_t m_maxJsonPayloadSize;
};

}  // namespace siodb::iomgr
