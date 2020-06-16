// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executeCreateUserRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::CreateUserRequest& request)
{
    response.set_has_affected_row_count(false);
    m_instance.createUser(
            request.m_name, request.m_realName, request.m_description, request.m_active, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeDropUserRequest(
        iomgr_protocol::DatabaseEngineResponse& response, const requests::DropUserRequest& request)
{
    response.set_has_affected_row_count(false);
    m_instance.dropUser(request.m_name, !request.m_ifExists, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeAlterUserRequest(
        iomgr_protocol::DatabaseEngineResponse& response, const requests::AlterUserRequest& request)
{
    response.set_has_affected_row_count(false);
    m_instance.updateUser(request.m_userName, request.m_params, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeAddUserAccessKeyRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::AddUserAccessKeyRequest& request)
{
    response.set_has_affected_row_count(false);
    m_instance.createUserAccessKey(request.m_userName, request.m_keyName, request.m_text,
            request.m_description, request.m_active, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeDropUserAccessKeyRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::DropUserAccessKeyRequest& request)
{
    response.set_has_affected_row_count(false);
    m_instance.dropUserAccessKey(
            request.m_userName, request.m_keyName, !request.m_ifExists, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeAlterUserAccessKeyRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::AlterUserAccessKey& request)
{
    response.set_has_affected_row_count(false);
    m_instance.updateUserAccessKey(
            request.m_userName, request.m_keyName, request.m_params, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

}  // namespace siodb::iomgr::dbengine
