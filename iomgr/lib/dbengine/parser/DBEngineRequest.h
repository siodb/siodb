// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "DBEngineRequestPtr.h"
#include "DBEngineRequestType.h"

namespace siodb::iomgr::dbengine::requests {

/** Base class for all database engine requests */
struct DBEngineRequest {
protected:
    /** Initializes object of class DBEngineRequest */
    explicit DBEngineRequest(DBEngineRequestType requestType) noexcept
        : m_requestType(requestType)
    {
    }

public:
    /** Disable copy construction */
    DBEngineRequest(const DBEngineRequest&) = delete;

    virtual ~DBEngineRequest() = default;

    /** Request type */
    const DBEngineRequestType m_requestType;
};

}  // namespace siodb::iomgr::dbengine::requests
