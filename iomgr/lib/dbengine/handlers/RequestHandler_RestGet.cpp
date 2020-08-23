// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/crt_ext/compiler_defs.h>
#include <siodb/common/io/ChunkedOutputStream.h>
#include <siodb/common/io/JsonWriter.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executeGetDatabasesRestRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::GetDatabasesRestRequest& request)
{
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);

    // Get databases
    const auto databaseNames = m_instance.getDatabaseNames();

    // Write response message
    {
        utils::DefaultErrorCodeChecker errorChecker;
        protobuf::StreamOutputStream rawOutput(m_connection, errorChecker);
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);
    }

    // Write JSON payload
    siodb::io::ChunkedOutputStream chunkedStream(kJsonChunkSize, m_connection);
    siodb::io::JsonWriter jsonWriter(chunkedStream);

    // Start top level object
    jsonWriter.writeObjectBegin();

    // Write status
    jsonWriter.writeField(kRestStatusFieldName, kRestStatusOk);

    // Start rows array
    jsonWriter.writeArrayBegin(kRestRowsFieldName, true);

    // Write rows
    bool needComma = false;
    for (const auto& databaseName : databaseNames) {
        jsonWriter.writeObjectBegin(nullptr, needComma);
        needComma = true;
        jsonWriter.writeField(kDatabaseNameFieldName, databaseName);
        jsonWriter.writeObjectEnd();
    }

    // End rows array
    jsonWriter.writeArrayEnd();

    // End top level object
    jsonWriter.writeObjectEnd();
}

void RequestHandler::executeGetTablesRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::GetTablesRestRequest& request)
{
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);

    // Get table list
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    auto tableNames = database->getTableNames(false);
    std::sort(tableNames.begin(), tableNames.end());

    // Write response message
    {
        utils::DefaultErrorCodeChecker errorChecker;
        protobuf::StreamOutputStream rawOutput(m_connection, errorChecker);
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);
    }

    // Write JSON payload
    siodb::io::ChunkedOutputStream chunkedStream(kJsonChunkSize, m_connection);
    siodb::io::JsonWriter jsonWriter(chunkedStream);

    // Start top level object
    jsonWriter.writeObjectBegin();

    // Write status
    jsonWriter.writeField(kRestStatusFieldName, kRestStatusOk);

    // Start rows array
    jsonWriter.writeArrayBegin(kRestRowsFieldName, true);

    // Write rows
    bool needComma = false;
    for (const auto& tableName : tableNames) {
        jsonWriter.writeObjectBegin(nullptr, needComma);
        needComma = true;
        jsonWriter.writeField(kTableNameFieldName, tableName);
        jsonWriter.writeObjectEnd();
    }

    // End rows array
    jsonWriter.writeArrayEnd();

    // End top level object
    jsonWriter.writeObjectEnd();
}

void RequestHandler::executeGetAllRowsRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::GetAllRowsRestRequest& request)
{
    sendNotImplementedYet(response);
}

void RequestHandler::executeGetSingleRowRestRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::GetSingleRowRestRequest& request)
{
    sendNotImplementedYet(response);
}

}  // namespace siodb::iomgr::dbengine
