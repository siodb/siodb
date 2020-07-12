// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerRequestHandlerBase.h"
#include "../dbengine/Instance.h"

namespace siodb::iomgr {

/** Sequentially executes incoming requests on the database engine instance. */
class IOManagerRequestExecutor : public IOManagerRequestHandlerBase {
public:
    /** 
     * Initializes object of class IOManagerRequestExecutor.
     * @param id Executor ID.
     * @param instance Database engine instance.
     */
    IOManagerRequestExecutor(std::size_t id, dbengine::Instance& instance);

protected:
    /**
     * Handles single request.
     * @param request A request to be handled.
     */
    void handleRequest(const IOManagerRequestPtr& request) override;

private:
    /**
     * Creates log context base string for the IOManagerRequestHandlerBase.
     * @param id Request executor ID.
     * @return Log context base string.
     */
    static std::string createLogContextBaseString(std::size_t id);

private:
    /** Executor ID */
    const std::size_t m_id;

    /** Database engine instance */
    dbengine::Instance& m_instance;

private:
    /** Log context name */
    static constexpr const char* kLogContextBase = "IOManagerRequestExecutor";
};

}  // namespace siodb::iomgr
