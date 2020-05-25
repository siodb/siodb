// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../Column.h"
#include "../Database.h"
#include "../DatabaseObjectName.h"
#include "../MasterColumnRecord.h"
#include "../Table.h"
#include "../ThrowDatabaseError.h"
#include "../crypto/KeyGenerator.h"
#include "../crypto/ciphers/Cipher.h"
#include "../parser/EmptyContext.h"

// Common project headers
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/SiodbProtocolTag.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executeCreateDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::CreateDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);

    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    if (request.m_temporary)
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateTemporaryDatabase);

    auto cipherId = m_instance.getDefaultDatabaseCipherId();

    // Empty seed makes generateCipherKey using default key seed
    std::string cipherKeySeed;
    requests::EmptyContext emptyContext;
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

    // nullptr is possible in case of 'none' cipher
    const auto cipher = crypto::getCipher(cipherId);
    const auto keyLength = cipher ? crypto::getCipher(cipherId)->getKeySize() : 0;
    auto cipherKey = cipher ? crypto::generateCipherKey(keyLength, cipherKeySeed) : BinaryValue();
    m_instance.createDatabase(
            std::string(request.m_database), cipherId, std::move(cipherKey), m_userId, {});

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeCreateTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::CreateTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& dbName = request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(dbName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, dbName);

    const auto db = m_instance.getDatabaseChecked(dbName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (db->isSystemDatabase() && !db->canContainUserTables())
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateUserTablesInSystemDatabase);

    if (db->isTableExists(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorTableAlreadyExists, dbName, request.m_table);

    std::vector<ColumnSpecification> tableColumns;
    tableColumns.reserve(request.m_columns.size());
    for (const auto& column : request.m_columns)
        tableColumns.push_back(convertTableColumnDefinition(column));

    // NOTE: Duplicate columns and columns with invalid names
    // are checked inside the createUserTable().
    db->createUserTable(std::string(request.m_table), TableType::kDisk, tableColumns, m_userId, {});

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeAddColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::AddColumnRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& dbName = request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(dbName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, dbName);

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
        [[maybe_unused]] const requests::CreateIndexRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& dbName = request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(dbName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, dbName);

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
        [[maybe_unused]] const requests::DropDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);
    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    if (m_currentDatabaseName == request.m_database)
        throwDatabaseError(IOManagerMessageId::kErrorCannotDropCurrentDatabase, request.m_database);

    m_instance.dropDatabase(request.m_database, !request.m_ifExists, m_userId);

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeUseDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::UseDatabaseRequest& request)
{
    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    // Throws exception if database does not exist
    auto newDatabase = m_instance.getDatabaseChecked(request.m_database);
    m_instance.getDatabaseChecked(m_currentDatabaseName)->release();
    newDatabase->use();
    m_currentDatabaseName = request.m_database;

    auto tag = response.add_tag();
    tag->set_name(kCurrentDatabaseTag);
    tag->set_string_value(request.m_database);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeDropTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::DropTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& dbName = request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(dbName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, dbName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    sendNotImplementedYet(response);
}

void RequestHandler::executeDropColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::DropColumnRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& dbName = request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(dbName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, dbName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_column))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnName, request.m_column);

    sendNotImplementedYet(response);
}

void RequestHandler::executeDropIndexRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::DropIndexRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& dbName = request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(dbName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, dbName);

    if (!isValidDatabaseObjectName(request.m_index))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidIndexName, request.m_index);

    sendNotImplementedYet(response);
}

void RequestHandler::executeAlterColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::AlterColumnRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& dbName = request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(dbName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, dbName);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_column))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnName, request.m_column);

    sendNotImplementedYet(response);
}

void RequestHandler::executeAttachDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::AttachDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);

    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    sendNotImplementedYet(response);
}

void RequestHandler::executeDetachDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::DetachDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& dbName = request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(dbName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, dbName);

    sendNotImplementedYet(response);
}

void RequestHandler::executeRenameTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::RenameTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const auto& dbName = request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(dbName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, dbName);

    if (!isValidDatabaseObjectName(request.m_newTable))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_newTable);

    if (isValidDatabaseObjectName(request.m_oldTable))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_oldTable);

    sendNotImplementedYet(response);
}

}  // namespace siodb::iomgr::dbengine
