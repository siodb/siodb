// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../Index.h"
#include "../TableDataSet.h"
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/BufferedChunkedOutputStream.h>
#include <siodb/common/io/JsonWriter.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/iomgr/shared/dbengine/SystemObjectNames.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executeGetDatabasesRestRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::GetDatabasesRestRequest& request)
{
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);

    // Get databases
    const auto databaseNames = m_instance.getDatabaseNames(isSuperUser());

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
    writeGetJsonProlog(jsonWriter);
    bool needComma = false;
    for (const auto& databaseName : databaseNames) {
        if (SIODB_LIKELY(needComma)) jsonWriter.writeComma();
        jsonWriter.writeObjectBegin();
        needComma = true;
        jsonWriter.writeFieldName(kDatabaseNameFieldName, ::ct_strlen(kDatabaseNameFieldName));
        jsonWriter.writeValue(databaseName);
        jsonWriter.writeObjectEnd();
    }
    writeJsonEpilog(jsonWriter);
    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

void RequestHandler::executeGetTablesRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::GetTablesRestRequest& request)
{
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);

    // Get table list
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    auto tableNames = database->getTableNames(isSuperUser());
    std::sort(tableNames.begin(), tableNames.end());

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
    writeGetJsonProlog(jsonWriter);
    bool needComma = false;
    for (const auto& tableName : tableNames) {
        if (SIODB_LIKELY(needComma)) jsonWriter.writeComma();
        jsonWriter.writeObjectBegin();
        needComma = true;
        jsonWriter.writeFieldName(kTableNameFieldName, ::ct_strlen(kTableNameFieldName));
        jsonWriter.writeValue(tableName);
        jsonWriter.writeObjectEnd();
    }
    writeJsonEpilog(jsonWriter);
    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

void RequestHandler::executeGetAllRowsRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::GetAllRowsRestRequest& request)
{
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);

    // Find table
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);
    const auto table = database->findTableChecked(request.m_table);
    if (table->isSystemTable() && !isSuperUser()) {
        throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotExist, table->getDatabaseName(),
                table->getName());
    }

    // Create data set
    TableDataSet dataSet(table);
    dataSet.fillColumnInfosFromTable();

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
    writeGetJsonProlog(jsonWriter);
    const auto& columns = dataSet.getColumns();
    const auto columnCount = columns.size();
    bool needCommaBeforeRow = false;
    for (dataSet.resetCursor(); dataSet.hasCurrentRow(); dataSet.moveToNextRow()) {
        dataSet.readCurrentRow();
        const auto& values = dataSet.getValues();
        if (SIODB_LIKELY(needCommaBeforeRow)) jsonWriter.writeComma();
        jsonWriter.writeObjectBegin();
        needCommaBeforeRow = true;
        bool needCommaBeforeColumn = false;
        for (std::size_t i = 0; i < columnCount; ++i) {
            const auto& column = columns[i];
            if (SIODB_LIKELY(needCommaBeforeColumn)) jsonWriter.writeComma();
            jsonWriter.writeFieldName(column->getName());
            writeVariantAsJson(values.at(i), jsonWriter);
            needCommaBeforeColumn = true;
        }
        jsonWriter.writeObjectEnd();
    }
    writeJsonEpilog(jsonWriter);
    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

void RequestHandler::executeGetSingleRowRestRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::GetSingleRowRestRequest& request)
{
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);

    // Find table
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);
    const auto table = database->findTableChecked(request.m_table);
    if (table->isSystemTable() && !isSuperUser()) {
        throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotExist, table->getDatabaseName(),
                table->getName());
    }

    // Find row
    const auto masterColumn = table->getMasterColumn();
    const auto index = masterColumn->getMasterColumnMainIndex();
    std::uint8_t key[8];
    ::pbeEncodeUInt64(request.m_trid, key);
    IndexValue indexValue;
    const auto valueCount = index->find(key, indexValue.m_data, 1);
    if (valueCount > 1) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnRecordIndexCorrupted,
                database->getName(), table->getName(), database->getUuid(), table->getId(), 2);
    }
    const bool haveRow = (valueCount == 1);

    struct RowRelatedData {
        MasterColumnRecord m_mcr;
        std::vector<ColumnPtr> m_columns;
    };
    std::unique_ptr<RowRelatedData> rowRelatedData;
    if (haveRow) {
        ColumnDataAddress mcrAddr;
        mcrAddr.pbeDeserialize(indexValue.m_data, sizeof(indexValue.m_data));
        rowRelatedData = std::make_unique<RowRelatedData>();
        masterColumn->readMasterColumnRecord(mcrAddr, rowRelatedData->m_mcr);
        const auto expectedColumnCount = table->getColumnCount() - 1;
        if (rowRelatedData->m_mcr.getColumnCount() != expectedColumnCount) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordColumnCount,
                    database->getName(), table->getName(), database->getUuid(), table->getId(),
                    mcrAddr.getBlockId(), mcrAddr.getOffset(), expectedColumnCount,
                    rowRelatedData->m_mcr.getColumnCount());
        }
        rowRelatedData->m_columns = table->getColumnsOrderedByPosition();
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
    writeGetJsonProlog(jsonWriter);
    if (haveRow) {
        jsonWriter.writeObjectBegin();
        jsonWriter.writeFieldName(rowRelatedData->m_columns[0]->getName());
        jsonWriter.writeValue(request.m_trid);
        const auto& columnRecords = rowRelatedData->m_mcr.getColumnRecords();
        for (std::size_t i = 0, n = columnRecords.size(); i < n; ++i) {
            const auto& column = rowRelatedData->m_columns.at(i + 1);
            jsonWriter.writeComma();
            jsonWriter.writeFieldName(column->getName());
            Variant v;
            column->readRecord(columnRecords[i].getAddress(), v);
            writeVariantAsJson(v, jsonWriter);
        }
        jsonWriter.writeObjectEnd();
    }
    writeJsonEpilog(jsonWriter);
    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

}  // namespace siodb::iomgr::dbengine
