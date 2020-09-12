// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "DBEngineRequestPtr.h"

// Protobuf message headers
#include <siodb/common/io/InputStream.h>
#include <siodb/common/proto/IOManagerProtocol.pb.h>

namespace siodb::iomgr::dbengine::parser {

/** DBEngineRestRequestFactory produces DB Engine requests from the REST requests. */
class DBEngineRestRequestFactory {
public:
    /**
     * Initializes object of class DBEngineRestRequestFactory.
     * @param maxJsonPayloadSize Maximum JSON payload size.
     */
    DBEngineRestRequestFactory(std::size_t maxJsonPayloadSize) noexcept
        : m_maxJsonPayloadSize(maxJsonPayloadSize)
    {
    }

    /**
     * Creates database engine request from a statement.
     * May read additional data from input, if provided.
     * @param msg Request message.
     * @param input Input stream.
     * @return Database engine request object filled with parsed data.
     */
    requests::DBEngineRequestPtr createRestRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg,
            siodb::io::InputStream* input = nullptr);

private:
    /**
     * Creates GET databases request.
     * @return GET databases request.
     */
    requests::DBEngineRequestPtr createGetDatabasesRequest();

    /**
     * Creates GET tables request.
     * @param msg Request message.
     * @return GET tables request.
     */
    requests::DBEngineRequestPtr createGetTablesRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg);

    /**
     * Creates GET all rows request.
     * @param msg Request message.
     * @return GET all rows request.
     */
    requests::DBEngineRequestPtr createGetAllRowsRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg);

    /**
     * Creates GET single row request.
     * @param msg Request message.
     * @return GET single row request.
     */
    requests::DBEngineRequestPtr createGetSingleRowRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg);

    /**
     * Creates POST rows request.
     * @param msg Request message.
     * @param input Input stream.
     * @return POST rows request.
     */
    requests::DBEngineRequestPtr createPostRowsRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg, siodb::io::InputStream& input);

private:
    /** Max JSON payload size */
    const std::size_t m_maxJsonPayloadSize;

private:
    /** JSON buffer grow step */
    static constexpr std::size_t kJsonBufferGrowStep = 65536;
};

}  // namespace siodb::iomgr::dbengine::parser
