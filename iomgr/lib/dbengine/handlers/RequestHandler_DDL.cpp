// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../Column.h"
#include "../Database.h"
#include "../MasterColumnRecord.h"
#include "../Table.h"
#include "../ThrowDatabaseError.h"
#include "../crypto/GetCipher.h"
#include "../parser/EmptyExpressionEvaluationContext.h"

// Common project headers
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/ProtocolTag.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>
#include <siodb/iomgr/shared/dbengine/crypto/KeyGenerator.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executeCreateDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::CreateDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);

    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    if (request.m_isTemporary)
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateTemporaryDatabase);

    auto cipherId = m_instance.getDefaultDatabaseCipherId();

    // Empty seed makes generateCipherKey using default key seed
    std::string cipherKeySeed;
    requests::EmptyExpressionEvaluationContext emptyContext;
    if (request.m_cipherId) {
        const auto cipherIdValue = request.m_cipherId->evaluate(emptyContext);
        if (cipherIdValue.isString())
            cipherId = cipherIdValue.getString();
        else
            throwDatabaseError(IOManagerMessageId::kErrorCipherIdTypeIsNotString);
    }

    if (request.m_cipherKeySeed) {
        const auto cipherKeySeedValue = request.m_cipherKeySeed->evaluate(emptyContext);
        if (cipherKeySeedValue.isString())
            cipherKeySeed = cipherKeySeedValue.getString();
        else
            throwDatabaseError(IOManagerMessageId::kErrorCipherKeySeedIsNotString);
    }

    // nullptr is possible in the case of 'none' cipher
    const auto cipher = crypto::getCipher(cipherId);
    const auto keyLength = cipher ? cipher->getKeySizeInBits() : 0;
    auto cipherKey = cipher ? crypto::generateCipherKey(keyLength, cipherKeySeed) : BinaryValue();
    m_instance.createDatabase(std::string(request.m_database), cipherId, std::move(cipherKey), {},
            request.m_maxTableCount, m_userId);

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeCreateTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::CreateTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    const auto database = m_instance.findDatabaseChecked(databaseName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    std::vector<ColumnSpecification> tableColumns;
    tableColumns.reserve(request.m_columns.size());
    for (const auto& column : request.m_columns)
        tableColumns.push_back(convertTableColumnDefinition(column));

    // NOTE: Duplicate columns and columns with invalid names
    // are checked inside the createUserTable().
    database->createUserTable(
            std::string(request.m_table), TableType::kDisk, tableColumns, m_userId, {});

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeAddColumnRequest(
        iomgr_protocol::DatabaseEngineResponse& response, const requests::AddColumnRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_column.m_name))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnName, request.m_column.m_name);

    if (!ColumnDataType_IsValid(request.m_column.m_dataType)) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnType, request.m_database,
                request.m_table, request.m_column.m_dataType);
    }

    sendNotImplementedYet(response);
}

void RequestHandler::executeCreateIndexRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::CreateIndexRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_index))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidIndexName, request.m_index);

    if (request.m_columns.empty()) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidIndexColumns, request.m_database,
                request.m_table, request.m_index);
    }

    sendNotImplementedYet(response);
}

void RequestHandler::executeDropDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::DropDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);
    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    if (m_currentDatabaseName == request.m_database)
        throwDatabaseError(IOManagerMessageId::kErrorCannotDropCurrentDatabase, request.m_database);

    m_instance.dropDatabase(request.m_database, !request.m_ifExists, m_userId);

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeRenameDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::RenameDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);
    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    if (m_currentDatabaseName == request.m_database) {
        throwDatabaseError(
                IOManagerMessageId::kErrorCannotRenameCurrentDatabase, request.m_database);
    }

    sendNotImplementedYet(response);
}

void RequestHandler::executeSetDatabaseAttributesRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::SetDatabaseAttributesRequest& request)
{
    response.set_has_affected_row_count(false);
    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    sendNotImplementedYet(response);
}

void RequestHandler::executeUseDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::UseDatabaseRequest& request)
{
    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    auto newDatabase = m_instance.findDatabaseChecked(request.m_database);
    m_instance.findDatabaseChecked(m_currentDatabaseName)->release();
    newDatabase->use();
    m_currentDatabaseName = request.m_database;

    auto tag = response.add_tag();
    tag->set_name(kCurrentDatabaseTag);
    tag->set_string_value(request.m_database);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeDropTableRequest(
        iomgr_protocol::DatabaseEngineResponse& response, const requests::DropTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    const auto database = m_instance.findDatabaseChecked(databaseName);

    database->dropTable(request.m_table, !request.m_ifExists, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeDropColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::DropColumnRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_column))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnName, request.m_column);

    sendNotImplementedYet(response);
}

void RequestHandler::executeRenameColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::RenameColumnRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_column))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnName, request.m_column);

    if (!isValidDatabaseObjectName(request.m_newColumn))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnName, request.m_column);

    sendNotImplementedYet(response);
}

void RequestHandler::executeDropIndexRequest(
        iomgr_protocol::DatabaseEngineResponse& response, const requests::DropIndexRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_index))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidIndexName, request.m_index);

    sendNotImplementedYet(response);
}

void RequestHandler::executeRedefineColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::RedefineColumnRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_newColumn.m_name))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnName, request.m_newColumn.m_name);

    sendNotImplementedYet(response);
}

void RequestHandler::executeAttachDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::AttachDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);

    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    sendNotImplementedYet(response);
}

void RequestHandler::executeDetachDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::DetachDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    sendNotImplementedYet(response);
}

void RequestHandler::executeRenameTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::RenameTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_newTable))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_newTable);

    if (isValidDatabaseObjectName(request.m_oldTable))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_oldTable);

    sendNotImplementedYet(response);
}

void RequestHandler::executeSetTableAttributesRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::SetTableAttributesRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    const auto database = m_instance.findDatabaseChecked(databaseName);
    const auto table = database->findTableChecked(request.m_table);

    if (request.m_nextTrid) {
        const auto nextTrid = *request.m_nextTrid;
        if (nextTrid == 0) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidNextUserTrid, database->getName(),
                    table->getName(), nextTrid);
        }
        try {
            table->setLastUserTrid(nextTrid - 1);
        } catch (std::exception& ex) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidNextUserTrid, database->getName(),
                    table->getName(), nextTrid);
        }
    }

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

}  // namespace siodb::iomgr::dbengine
