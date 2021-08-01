// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
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

    requests::EmptyExpressionEvaluationContext emptyContext;

    auto cipherId = m_instance.getDefaultDatabaseCipherId();
    if (request.m_cipherId) {
        const auto cipherIdValue = request.m_cipherId->evaluate(emptyContext);
        if (cipherIdValue.isString())
            cipherId = cipherIdValue.getString();
        else
            throwDatabaseError(IOManagerMessageId::kErrorWrongAttributeType, "CIPHER_ID", "STRING");
    }

    // Empty seed makes generateCipherKey using default key seed
    std::string cipherKeySeed;
    if (request.m_cipherKeySeed) {
        const auto cipherKeySeedValue = request.m_cipherKeySeed->evaluate(emptyContext);
        if (cipherKeySeedValue.isString())
            cipherKeySeed = cipherKeySeedValue.getString();
        else {
            throwDatabaseError(
                    IOManagerMessageId::kErrorWrongAttributeType, "CIPHER_KEY_SEED", "STRING");
        }
    }

    // Empty uuid makes Siodb calculate database UUID in the standard way
    std::optional<Uuid> uuid;
    if (request.m_uuid) {
        const auto uuidValue = request.m_uuid->evaluate(emptyContext);
        if (uuidValue.isString()) {
            std::istringstream iss(uuidValue.getString());
            Uuid tmpUuid;
            iss >> tmpUuid;
            if (!iss) throwDatabaseError(IOManagerMessageId::kErrorInvalidAttributeValue, "UUID");
            char c = 0;
            iss >> c;
            if (!iss.eof())
                throwDatabaseError(IOManagerMessageId::kErrorInvalidAttributeValue, "UUID");
            uuid = tmpUuid;
        } else
            throwDatabaseError(IOManagerMessageId::kErrorWrongAttributeType, "UUID", "STRING");
    }

    bool dataDirectoryMustExist = false;
    if (request.m_dataDirectoryMustExist) {
        const auto dataDirectoryMustExistValue =
                request.m_dataDirectoryMustExist->evaluate(emptyContext);
        if (dataDirectoryMustExistValue.isBool())
            dataDirectoryMustExist = dataDirectoryMustExistValue.getBool();
        else {
            throwDatabaseError(IOManagerMessageId::kErrorWrongAttributeType,
                    "DATA_DIRECTORY_MUST_EXIST", "BOOLEAN");
        }
    }

    // nullptr value of the "cipher" is possible here if the cipher is 'none'
    const auto cipher = crypto::getCipher(cipherId);
    const auto keyLength = cipher ? cipher->getKeySizeInBits() : 0;
    auto cipherKey = cipher ? crypto::generateCipherKey(keyLength, cipherKeySeed) : BinaryValue();
    m_instance.createDatabase(std::string(request.m_database), cipherId, std::move(cipherKey), {},
            request.m_maxTableCount, uuid, dataDirectoryMustExist, m_userId);

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
    UseDatabaseGuard databaseGuard(*database);

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

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_table);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to alter this table or any table in this database
    // or any table in the any database
    if (!user->hasPermissions(
                database->getId(), DatabaseObjectType::kTable, table->getId(), kAlterPermissionMask)
            && !user->hasPermissions(
                    database->getId(), DatabaseObjectType::kTable, 0, kAlterPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kTable, 0, kAlterPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
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

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_table);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // Table should be visible to the user
    if (!user->hasPermissions(
                database->getId(), DatabaseObjectType::kTable, table->getId(), kShowPermissionMask)
            && !user->hasPermissions(
                    database->getId(), DatabaseObjectType::kTable, 0, kShowPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kTable, 0, kShowPermissionMask)) {
        throwDatabaseError(
                IOManagerMessageId::kErrorTableDoesNotExist, databaseName, request.m_table);
    }

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to create index in this database or in the any database
    if (!user->hasPermissions(
                database->getId(), DatabaseObjectType::kIndex, 0, kCreatePermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kIndex, 0, kCreatePermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

    // TODO: check if index already exists

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

    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to alter this database or any database
    if (!user->hasPermissions(
                0, DatabaseObjectType::kDatabase, database->getId(), kAlterPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kDatabase, 0, kAlterPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
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

    const auto database = m_instance.findDatabaseChecked(request.m_database);
    UseDatabaseGuard databaseGuard(*database);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to alter this database or any database
    if (!user->hasPermissions(
                0, DatabaseObjectType::kDatabase, database->getId(), kAlterPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kDatabase, 0, kAlterPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

    sendNotImplementedYet(response);
}

void RequestHandler::executeUseDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::UseDatabaseRequest& request)
{
    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    const auto currentDatabase = m_instance.findDatabaseChecked(m_currentDatabaseName);
    const auto newDatabase = m_instance.findDatabaseChecked(request.m_database);
    const auto user = m_instance.findUserChecked(m_userId);

    // User should have permission to list this database or any database
    if (!user->hasPermissions(
                0, DatabaseObjectType::kDatabase, newDatabase->getId(), kShowPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kDatabase, 0, kShowPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseDoesNotExist, request.m_database);
    }

    currentDatabase->release();
    newDatabase->use();
    m_currentDatabaseName = request.m_database;

    const auto tag = response.add_tag();
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
    UseDatabaseGuard databaseGuard(*database);

    if (database->isSystemTable(request.m_table)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotDropSystemTable);
    }

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

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_table);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to alter this table or any table in this database
    // or any table in the any database
    if (!user->hasPermissions(
                database->getId(), DatabaseObjectType::kTable, table->getId(), kAlterPermissionMask)
            && !user->hasPermissions(
                    database->getId(), DatabaseObjectType::kTable, 0, kAlterPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kTable, 0, kAlterPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

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

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_table);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to alter this table or any table in this database
    // or any table in the any database
    if (!user->hasPermissions(
                database->getId(), DatabaseObjectType::kTable, table->getId(), kAlterPermissionMask)
            && !user->hasPermissions(
                    database->getId(), DatabaseObjectType::kTable, 0, kAlterPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kTable, 0, kAlterPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

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

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

#if 0
    const auto index = database->findIndexChecked(request.m_index);
#endif

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to drop this index or drop any index in this database
    // or drop any index in the any database
    if (
#if 0
        !user->hasPermissions(database->getId(), DatabaseObjectType::kIndex, index->getId(), kDropPermissionMask)
            &&
#endif
            !user->hasPermissions(
                    database->getId(), DatabaseObjectType::kTable, 0, kDropPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kTable, 0, kDropPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

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

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_table);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to alter this table or any table in this database
    // or any table in the any database
    if (!user->hasPermissions(
                database->getId(), DatabaseObjectType::kTable, table->getId(), kAlterPermissionMask)
            && !user->hasPermissions(
                    database->getId(), DatabaseObjectType::kTable, 0, kAlterPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kTable, 0, kAlterPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

    sendNotImplementedYet(response);
}

void RequestHandler::executeAttachDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::AttachDatabaseRequest& request)
{
    response.set_has_affected_row_count(false);

    if (!isValidDatabaseObjectName(request.m_database))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to attach database
    if (!user->hasPermissions(0, DatabaseObjectType::kDatabase, 0, kAttachPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

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

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to detach this database or any database
    if (!user->hasPermissions(
                0, DatabaseObjectType::kDatabase, database->getId(), kDetachPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kDatabase, 0, kDetachPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

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

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_oldTable);

    const auto user = m_instance.findUserChecked(m_userId);

    // TODO: Move this check into the proper function when implemented.
    // User should have permission to alter this table or any table in this database
    // or any table in the any database
    if (!user->hasPermissions(
                database->getId(), DatabaseObjectType::kTable, table->getId(), kAlterPermissionMask)
            && !user->hasPermissions(
                    database->getId(), DatabaseObjectType::kTable, 0, kAlterPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kTable, 0, kAlterPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

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
    UseDatabaseGuard databaseGuard(*database);

    const auto table = database->findTableChecked(request.m_table);

    const auto user = m_instance.findUserChecked(m_userId);

    // Validate
    if (request.m_nextTrid) {
        const auto nextTrid = *request.m_nextTrid;
        if (nextTrid == 0) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidNextUserTrid, database->getName(),
                    table->getName(), nextTrid);
        }
    }

    // Check permissions
    // User should have permission to alter this table or any table in this database
    // or any table in the any database
    if (!user->hasPermissions(
                database->getId(), DatabaseObjectType::kTable, table->getId(), kAlterPermissionMask)
            && !user->hasPermissions(
                    database->getId(), DatabaseObjectType::kTable, 0, kAlterPermissionMask)
            && !user->hasPermissions(0, DatabaseObjectType::kTable, 0, kAlterPermissionMask)) {
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
    }

    // Execute
    if (request.m_nextTrid) {
        const auto nextTrid = *request.m_nextTrid;
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
