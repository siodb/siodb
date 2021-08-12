// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>
#include <siodb/iomgr/shared/dbengine/parser/CommonConstants.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executeGrantPermissionsForTableRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::GrantPermissionsForTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const std::string& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (databaseName != requests::kAllObjectsName && !isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    if (request.m_table != requests::kAllObjectsName || !isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_user))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserName, request.m_user);

    m_instance.grantTablePermissionsToUser(request.m_user, databaseName, request.m_table,
            request.m_permissions, request.m_withGrantOption, m_currentUserId);

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeRevokePermissionsForTableRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::RevokePermissionsForTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const std::string& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (databaseName != requests::kAllObjectsName && !isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    if (request.m_table != requests::kAllObjectsName || !isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_user))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserName, request.m_user);

    m_instance.revokeTablePermissionsFromUser(
            request.m_user, databaseName, request.m_table, request.m_permissions, m_currentUserId);

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

}  // namespace siodb::iomgr::dbengine
