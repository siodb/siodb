// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "JsonOutput.h"
#include "RequestHandlerSharedConstants.h"
#include "RestProtocolRowsetWriterFactory.h"
#include "VariantOutput.h"
#include "../Index.h"
#include "../TableDataSet.h"
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/BufferedChunkedOutputStream.h>
#include <siodb/common/io/JsonWriter.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/net/HttpStatus.h>
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
    auto databaseRecords = m_instance.getDatabaseRecordsOrderedByName(m_currentUserId);

    response.set_rest_status_code(
            databaseRecords.empty() ? net::HttpStatus::kNotFound : net::HttpStatus::kOk);

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
    writeGetJsonProlog(response.rest_status_code(), jsonWriter);
    bool needComma = false;
    for (const auto& dbRecord : databaseRecords) {
        if (SIODB_LIKELY(needComma)) jsonWriter.writeComma();
        jsonWriter.writeObjectBegin();
        needComma = true;
        jsonWriter.writeFieldName(kDatabaseNameFieldName, ::ct_strlen(kDatabaseNameFieldName));
        jsonWriter.writeValue(dbRecord.m_name);
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
    response.set_rest_status_code(net::HttpStatus::kNotFound);

    // Get table list
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);

    const auto tableRecords = database->getTableRecordsOrderedByName(m_currentUserId);

    response.set_rest_status_code(
            tableRecords.empty() ? net::HttpStatus::kNotFound : net::HttpStatus::kOk);

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
    writeGetJsonProlog(response.rest_status_code(), jsonWriter);
    bool needComma = false;
    for (const auto& tableRecord : tableRecords) {
        if (SIODB_LIKELY(needComma)) jsonWriter.writeComma();
        jsonWriter.writeObjectBegin();
        needComma = true;
        jsonWriter.writeFieldName(kTableNameFieldName, ::ct_strlen(kTableNameFieldName));
        jsonWriter.writeValue(tableRecord.m_name);
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
    response.set_rest_status_code(net::HttpStatus::kNotFound);

    // Find table
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_table);
    table->checkOperationPermitted(m_currentUserId,
            table->isSystemTable() ? PermissionType::kSelectSystem : PermissionType::kSelect);

    // Create data set
    TableDataSet dataSet(table);
    dataSet.fillColumnInfosFromTable();

    // Write response message
    response.set_rest_status_code(net::HttpStatus::kOk);
    {
        utils::DefaultErrorCodeChecker errorChecker;
        protobuf::StreamOutputStream rawOutput(m_connection, errorChecker);
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);
    }

    // Write JSON payload
    siodb::io::BufferedChunkedOutputStream chunkedOutput(kJsonChunkSize, m_connection);
    siodb::io::JsonWriter jsonWriter(chunkedOutput);
    writeGetJsonProlog(net::HttpStatus::kOk, jsonWriter);
    const auto& columns = dataSet.getColumns();
    const auto columnCount = columns.size();
    bool hadFirstRow = false;
    for (dataSet.resetCursor(); dataSet.hasCurrentRow(); dataSet.moveToNextRow()) {
        dataSet.readCurrentRow();
        const auto& values = dataSet.getValues();
        if (SIODB_LIKELY(hadFirstRow)) jsonWriter.writeComma();
        hadFirstRow = true;
        jsonWriter.writeObjectBegin();
        bool hadFirstColumn = false;
        for (std::size_t i = 0; i != columnCount; ++i) {
            const auto& column = columns[i];
            if (SIODB_LIKELY(hadFirstColumn)) jsonWriter.writeComma();
            hadFirstColumn = true;
            jsonWriter.writeFieldName(column->getName());
            writeVariant(values.at(i), jsonWriter);
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
    response.set_rest_status_code(net::HttpStatus::kNotFound);

    // Find table
    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_table);
    table->checkOperationPermitted(m_currentUserId,
            table->isSystemTable() ? PermissionType::kSelectSystem : PermissionType::kSelect);

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
        response.set_rest_status_code(net::HttpStatus::kOk);
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
    writeGetJsonProlog(response.rest_status_code(), jsonWriter);
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
            writeVariant(v, jsonWriter);
        }
        jsonWriter.writeObjectEnd();
    }
    writeJsonEpilog(jsonWriter);

    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

void RequestHandler::executeGetSqlQueryRowsRestRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::GetSqlQueryRowsRestRequest& request)
{
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);
    RestProtocolRowsetWriterFactory rowsetWriterFactory;
    executeSelectRequest(response, *request.m_query, rowsetWriterFactory);
}

void RequestHandler::executePostRowsRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::PostRowsRestRequest& request)
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

    const TransactionParameters transactionParams(
            m_currentUserId, database->generateNextTransactionId());

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
        try {
            const auto result =
                    table->insertRow(columnNames, std::move(rowValues), transactionParams);
            tridList.push_back(result.m_mcr->getTableRowId());
        } catch (DatabaseError& ex) {
            response.set_rest_status_code(net::HttpStatus::kBadRequest);
            throw;
        } catch (std::exception& ex) {
            response.set_rest_status_code(net::HttpStatus::kInternalServerError);
            throw;
        }
    }

    response.set_affected_row_count(tridList.size());
    response.set_rest_status_code(net::HttpStatus::kCreated);

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
    writeModificationJsonProlog(net::HttpStatus::kCreated, tridList.size(), jsonWriter);
    bool needComma = false;
    for (const auto& trid : tridList) {
        if (SIODB_LIKELY(needComma)) jsonWriter.writeComma();
        needComma = true;
        jsonWriter.writeValue(trid);
    }
    writeJsonEpilog(jsonWriter);
    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

void RequestHandler::executeDeleteRowRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::DeleteRowRestRequest& request)
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
            throwDatabaseError(IOManagerMessageId::kErrorCannotDeleteFromSystemTable,
                    table->getDatabaseName(), table->getName());
        } else {
            throwDatabaseError(IOManagerMessageId::kErrorTableDoesNotExist,
                    table->getDatabaseName(), table->getName());
        }
    }

    // Delete row
    DeleteRowResult deleteResult;
    try {
        const TransactionParameters tp(
                m_currentUserId, table->getDatabase().generateNextTransactionId());
        deleteResult = table->deleteRow(request.m_trid, tp);
        if (deleteResult.m_deleted) {
            response.set_affected_row_count(1);
            response.set_rest_status_code(net::HttpStatus::kOk);
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
            deleteResult.m_deleted ? net::HttpStatus::kOk : net::HttpStatus::kNotFound,
            response.affected_row_count(), jsonWriter);
    if (response.affected_row_count() > 0) jsonWriter.writeValue(request.m_trid);
    writeJsonEpilog(jsonWriter);
    if (chunkedOutput.close() != 0) stdext::throw_system_error("Failed to send JSON payload");
}

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
        const TransactionParameters tp(
                m_currentUserId, table->getDatabase().generateNextTransactionId());
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
