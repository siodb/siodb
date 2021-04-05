// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerRequestExecutor.h"

namespace siodb::iomgr {

/** Dispatches incoming requests to executors */
class IOManagerRequestDispatcher : public IOManagerRequestHandlerBase {
public:
    /**
     * Initializes object of class IOManagerRequestDispatcher.
     * @param options Siodb options.
     * @param instance Database engine instance.
     */
    explicit IOManagerRequestDispatcher(
            const config::SiodbOptions& options, dbengine::Instance& instance);

    /**
     * Returns database engine instance.
     * @return Database engine instance.
     */
    auto& getInstance() const noexcept
    {
        return m_instance;
    }

protected:
    /**
     * Handles single request.
     * @param request A request to be handled.
     */
    void handleRequest(const IOManagerRequestPtr& request) override;

private:
    /**
     * Creates request executor pool.
     * @param size Pool size.
     */
    std::vector<std::unique_ptr<IOManagerRequestExecutor>> createRequestExecutorPool(
            std::size_t size) const;

private:
    /** Database engine instance */
    dbengine::Instance& m_instance;

    /** Request executor pool */
    const std::vector<std::unique_ptr<IOManagerRequestExecutor>> m_requestExecutorPool;

    /** Log context name */
    static constexpr const char* kLogContextBase = "IOManagerRequestDispatcher";
};

}  // namespace siodb::iomgr
