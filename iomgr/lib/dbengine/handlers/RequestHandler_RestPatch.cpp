// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "JsonOutput.h"
#include "RequestHandlerSharedConstants.h"
#include "../Index.h"
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/io/BufferedChunkedOutputStream.h>
#include <siodb/common/io/JsonWriter.h>
#include <siodb/common/net/HttpStatus.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executePatchRowRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::PatchRowRestRequest& request)
{
    response.set_has_affected_row_count(true);
    response.set_affected_row_count(0);
    response.set_rest_status_code(net::HttpStatus::kNotFound);

    // Find table
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_table);
    if (table->isSystemTable()) {
        if (isSuperUser()) {
            response.set_rest_status_code(net::HttpStatus::kForbidden);
            throwDatabaseError(IOManagerMessageId::kErrorCannotUpdateSystemTable,
                    table->getDatabaseName(), table->getName());
        } else {
            throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotExist,
                    table->getDatabaseName(), table->getName());
        }
    }

    // Update row
    UpdateRowResult updateResult;
    try {
        const TransactionParameters tp(m_userId, table->getDatabase().generateNextTransactionId());
        updateResult = table->updateRow(request.m_trid, request.m_columnNames,
                std::move(const_cast<std::vector<Variant>&>(request.m_values)), false, tp);
        if (updateResult.m_updated) {
            response.set_rest_status_code(net::HttpStatus::kOk);
            response.set_affected_row_count(1);
        }
    } catch (DatabaseError& ex) {
        response.set_rest_status_code(net::HttpStatus::kBadRequest);
        throw;
    } catch (std::exception& ex) {
        response.set_rest_status_code(net::HttpStatus::kInternalServerError);
        throw;
    }

    // Write response message
    {
        utils::DefaultErrorCodeChecker errorChecker;
        protobuf::StreamOutputStream rawOutput(m_connection, errorChecker);
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);
    }

    // Write JSON payload
    siodb::io::BufferedChunkedOutputStream chunkedOutput(kJsonChunkSize, m_connection);
    siodb::io::JsonWriter jsonWriter(chunkedOutput);
    writeModificationJsonProlog(
            updateResult.m_updated ? net::HttpStatus::kOk : net::HttpStatus::kNotFound,
            response.affected_row_count(), jsonWriter);
    if (response.affected_row_count() > 0) jsonWriter.writeValue(request.m_trid);
    writeJsonEpilog(jsonWriter);
    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

}  // namespace siodb::iomgr::dbengine
