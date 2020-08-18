// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/protobuf/ProtobufJsonIO.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executeRestGetDatabasesRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::GetDatabasesRestRequest& request)
{
#if 0
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);

    // Get databases
    const auto databaseNames = m_instance.getDatabaseNames();

    // Write response message
    utils::DefaultErrorCodeChecker errorChecker;
    protobuf::SiodbProtobufOutputStream rawOutput(m_connection, errorChecker);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);

    // Create JSON payload
    google::protobuf::io::CodedOutputStream codedOutput(&rawOutput);
    protobuf::JsonWriter jsonWriter(codedOutput);

    // Start top level object
    protobuf::JsonObjectWriteGuard topLevelObjectGuard(jsonWriter);

    // Write status
    jsonWriter.writeField(kRestStatusFieldName, kRestStatusOk);
    protobuf::checkOutputStreamError(rawOutput);

    // Start rows array
    protobuf::JsonArrayWriteGuard rowsArrayGuard(jsonWriter, kRestRowsFieldName, true);

    // Write rows
    bool needComma = false;
    for (const auto& databaseName : databaseNames) {
        protobuf::checkOutputStreamError(rawOutput);
        protobuf::JsonObjectWriteGuard databaseObjectGuard(jsonWriter, nullptr, needComma);
        needComma = true;
        jsonWriter.writeField(kDatabaseNameFieldName, databaseName);
    }
    protobuf::checkOutputStreamError(rawOutput);
#else
    sendNotImplementedYet(response);
#endif
}

void RequestHandler::executeRestGetTablesRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::GetTablesRestRequest& request)
{
#if 0
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);

    // Get table list
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    auto tableNames = database->getTableNames(false);
    std::sort(tableNames.begin(), tableNames.end());

    // Write response message
    utils::DefaultErrorCodeChecker errorChecker;
    protobuf::SiodbProtobufOutputStream rawOutput(m_connection, errorChecker);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);

    // Create JSON payload
    google::protobuf::io::CodedOutputStream codedOutput(&rawOutput);
    protobuf::JsonWriter jsonWriter(codedOutput);

    // Start top level object
    protobuf::JsonObjectWriteGuard topLevelObjectGuard(jsonWriter);

    // Write status
    jsonWriter.writeField(kRestStatusFieldName, kRestStatusOk);
    protobuf::checkOutputStreamError(rawOutput);

    // Start rows array
    protobuf::JsonArrayWriteGuard rowsArrayGuard(jsonWriter, kRestRowsFieldName, true);

    // Write rows
    bool needComma = false;
    for (const auto& tableName : tableNames) {
        protobuf::checkOutputStreamError(rawOutput);
        protobuf::JsonObjectWriteGuard tableObjectGuard(jsonWriter, nullptr, needComma);
        needComma = true;
        jsonWriter.writeField(kTableNameFieldName, tableName);
    }
    protobuf::checkOutputStreamError(rawOutput);
#else
    sendNotImplementedYet(response);
#endif
}

void RequestHandler::executeRestGetAllRowsRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::GetAllRowsRestRequest& request)
{
    sendNotImplementedYet(response);
}

void RequestHandler::executeRestGetSingleRowRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::GetSingleRowRestRequest& request)
{
    sendNotImplementedYet(response);
}

}  // namespace siodb::iomgr::dbengine
