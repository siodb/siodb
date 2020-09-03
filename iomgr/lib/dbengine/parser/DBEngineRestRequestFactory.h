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

/** DBEngineRestRequestFactory produces DB Engine requests for the SQL statements. */
class DBEngineRestRequestFactory {
public:
    /**
     * Creates database engine request from a statement.
     * May read additional data from input, if provided.
     * @param msg Request message.
     * @param input Input stream.
     * @return Database engine request object filled with parsed data.
     */
    static requests::DBEngineRequestPtr createRestRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg,
            siodb::io::InputStream* input = nullptr);

private:
    /**
     * Creates GET databases request.
     * @param msg Request message.
     * @return GET databases request.
     */
    static requests::DBEngineRequestPtr createGetDatabasesRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg);

    /**
     * Creates GET tables request.
     * @param msg Request message.
     * @return GET tables request.
     */
    static requests::DBEngineRequestPtr createGetTablesRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg);

    /**
     * Creates GET all rows request.
     * @param msg Request message.
     * @return GET all rows request.
     */
    static requests::DBEngineRequestPtr createGetAllRowsRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg);

    /**
     * Creates GET single row request.
     * @param msg Request message.
     * @return GET single row request.
     */
    static requests::DBEngineRequestPtr createGetSingleRowRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg);

    /**
     * Creates POST rows request.
     * @param msg Request message.
     * @param input Input stream.
     * @return POST rows request.
     */
    static requests::DBEngineRequestPtr createPostRowsRequest(
            const iomgr_protocol::DatabaseEngineRestRequest& msg, siodb::io::InputStream& input);

private:
    /** JSON chunk size */
    static constexpr std::size_t kJsonChunkSize = 65536;

    /** Maximum row count for single POST */
    static constexpr std::size_t kMaxPostRowCount = 10000;
};

}  // namespace siodb::iomgr::dbengine::parser
