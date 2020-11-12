// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/io/BufferedChunkedOutputStream.h>
#include <siodb/common/io/JsonWriter.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/system_error_ext.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executePostRowsRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::PostRowsRestRequest& request)
{
    response.set_has_affected_row_count(true);
    response.set_affected_row_count(0);

    // Find table
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);
    const auto table = database->findTableChecked(request.m_table);
    if (table->isSystemTable()) {
        if (isSuperUser()) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotInsertToSystemTable,
                    table->getDatabaseName(), table->getName());
        } else {
            throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotExist,
                    table->getDatabaseName(), table->getName());
        }
    }

    std::vector<std::uint64_t> tridList;
    tridList.reserve(request.m_values.size());

    auto& mutableRequest = stdext::as_mutable(request);
    const auto maxColumnCount = table->getColumnCount() - 1;

    std::vector<std::string> columnNames;
    columnNames.reserve(maxColumnCount);

    std::vector<Variant> rowValues;
    rowValues.reserve(maxColumnCount);

    const TransactionParameters transactionParams(m_userId, database->generateNextTransactionId());

    for (std::size_t i = 0, n = mutableRequest.m_values.size(); i < n; ++i) {
        auto& row = mutableRequest.m_values[i];

        // Check number of columns
        if (row.size() > maxColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorTooManyValuesInPayload, row.size(), i,
                    maxColumnCount, database->getName(), table->getName());
        }

        // Prepare columns
        columnNames.clear();
        rowValues.clear();
        for (auto& e : row) {
            columnNames.push_back(mutableRequest.m_columnNames.at(e.first));
            rowValues.push_back(std::move(e.second));
        }

        // Insert row
        auto result = table->insertRow(columnNames, std::move(rowValues), transactionParams);
        tridList.push_back(result.first->getTableRowId());
    }

    response.set_affected_row_count(tridList.size());
    response.set_rest_status_code(kRestStatusCreated);

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
    writeModificationJsonProlog(kRestStatusCreated, tridList.size(), jsonWriter);
    bool needComma = false;
    for (const auto& trid : tridList) {
        if (SIODB_LIKELY(needComma)) jsonWriter.writeComma();
        needComma = true;
        jsonWriter.writeValue(trid);
    }
    writeJsonEpilog(jsonWriter);
    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

}  // namespace siodb::iomgr::dbengine
