// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/io/ChunkedOutputStream.h>
#include <siodb/common/io/JsonWriter.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/system_error_ext.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executePostRowsRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::PostRowsRestRequest& request)
{
    // Find table
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);
    const auto table = database->findTableChecked(request.m_table);
    if (table->isSystemTable()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotOperateOnSystemTableViaRest,
                table->getDatabaseName(), table->getName());
    }

    response.set_has_affected_row_count(true);
    response.set_affected_row_count(0);

#if 1
    sendNotImplementedYet(response);
#else
    std::vector<std::uint64_t> tridList;

    // TODO: implement actual INSERT

    // for now: just simulate activity
    for (std::size_t i = 0; i < request.m_values.size(); ++i)
        tridList.push_back(i);
    response.set_affected_row_count(request.m_values.size());

    // Write response message
    {
        utils::DefaultErrorCodeChecker errorChecker;
        protobuf::StreamOutputStream rawOutput(m_connection, errorChecker);
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);
    }

    // Write JSON payload
    siodb::io::ChunkedOutputStream chunkedOutput(kJsonChunkSize, m_connection);
    siodb::io::JsonWriter jsonWriter(chunkedOutput);
    writeJsonProlog(jsonWriter);
    bool needComma = false;
    for (const auto& trid : tridList) {
        if (SIODB_LIKELY(needComma)) jsonWriter.writeComma();
        needComma = true;
        jsonWriter.writeValue(trid);
    }
    writeJsonEpilog(jsonWriter);
    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
#endif
}

}  // namespace siodb::iomgr::dbengine
