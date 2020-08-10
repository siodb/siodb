// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerRequestPtr.h"
#include "../dbengine/parser/DBEngineRequestPtr.h"

// Common project headers
#include <siodb/common/utils/HelperMacros.h>

// STL headers
#include <atomic>
#include <future>

namespace siodb::iomgr {

class IOManagerConnectionHandler;

/** IO Manager request from client */
class IOManagerRequest {
public:
    using ExecutionResult = bool;

public:
    /**
     * Initializes object of class IOManagerRequest.
     * @param requestId Incoming request ID.
     * @param responseId Response ID.
     * @param statementCount Total statement count in the client request.
     * @param connectionHandler Connection handler.
     * @param dbeRequest Database engine request.
     */
    IOManagerRequest(std::uint64_t requestId, std::uint32_t responseId, std::size_t statementCount,
            const std::weak_ptr<IOManagerConnectionHandler>& connectionHandler,
            const dbengine::requests::ConstDBEngineRequestPtr& dbeRequest) noexcept
        : m_id(++s_idCounter)
        , m_requestId(requestId)
        , m_responseId(responseId)
        , m_statementCount(statementCount)
        , m_connectionHandler(connectionHandler)
        , m_dbeRequest(dbeRequest)
        , m_future(m_promise.get_future())
    {
    }

    DECLARE_NONCOPYABLE(IOManagerRequest);

    /**
     * Returns request object ID.
     * @return Request object ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns request ID.
     * @return Request ID.
     */
    auto getRequestId() const noexcept
    {
        return m_requestId;
    }

    /**
     * Returns response ID.
     * @return Response ID.
     */
    auto getResponseId() const noexcept
    {
        return m_responseId;
    }

    /**
     * Returns statement count in the client request.
     * @return Statement count.
     */
    auto getStatementCount() const noexcept
    {
        return m_statementCount;
    }

    /**
     * Returns database engine request.
     * @return Database engine request.
     */
    const dbengine::requests::DBEngineRequest& getDBEngineRequest() const noexcept
    {
        return *m_dbeRequest;
    }

    /**
     * Returns connection handler object.
     * @return Connection handler object it it still exist,
     *         nullptr if connection handler no longer exists.
     */
    auto getConnectionHanlder() const noexcept
    {
        return m_connectionHandler.lock();
    }

    /**
     * Returns future object on which result of this request execution can be waited for.
     * @return Shared future object.
     */
    auto getFuture() const noexcept
    {
        return m_future;
    }

    /**
     * Sets and communicates back request execution result.
     * @param result A request execution result.
     */
    void setResult(ExecutionResult result)
    {
        m_promise.set_value(std::move(result));
    }

private:
    /** Request object ID */
    const std::uint64_t m_id;

    /** Incoming request ID */
    const std::uint64_t m_requestId;

    /** Response ID */
    const std::uint32_t m_responseId;

    /** Total statement count in the client request. */
    const std::size_t m_statementCount;

    /** Connection handler */
    const std::weak_ptr<IOManagerConnectionHandler> m_connectionHandler;

    /** Database engine request */
    const dbengine::requests::ConstDBEngineRequestPtr m_dbeRequest;

    /** Promise object for the request execution result communication */
    std::promise<ExecutionResult> m_promise;

    /** Shared future object to track request execution result in the mutiple threads */
    std::shared_future<ExecutionResult> m_future;

    /** Request object ID counter */
    static std::atomic<std::uint64_t> s_idCounter;
};

/** Request executuon result assignment guard. */
class IOManagerRequestExecutionResultAssignmentGuard {
public:
    /**
     * Initializes object of class IOManagerRequestExecutionResultAssignmentGuard
     * @param request A request to assign result to.
     */
    explicit IOManagerRequestExecutionResultAssignmentGuard(IOManagerRequest& request)
        : m_request(request)
        , m_result(IOManagerRequest::ExecutionResult(0))
    {
    }

    /** De-initializes object. Assigns execution result to the underlying request. */
    ~IOManagerRequestExecutionResultAssignmentGuard()
    {
        try {
            m_request.setResult(std::move(m_result));
        } catch (...) {
            // Ignore errors here
        }
    }

    DECLARE_NONCOPYABLE(IOManagerRequestExecutionResultAssignmentGuard);

    /**
     * Sets new execution result.
     * @param result New result value.
     */
    void setResult(IOManagerRequest::ExecutionResult result) noexcept(
            std::is_nothrow_move_assignable_v<IOManagerRequest::ExecutionResult>)
    {
        m_result = std::move(result);
    }

private:
    /** Request to assign result to */
    IOManagerRequest& m_request;

    /** Execution result to be assigned */
    IOManagerRequest::ExecutionResult m_result;
};

}  // namespace siodb::iomgr
