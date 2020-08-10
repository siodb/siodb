// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

// Boost headers
#include <boost/algorithm/hex.hpp>

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

void RequestHandler::executeSetUserAttributesRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::SetUserAttributesRequest& request)
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

void RequestHandler::executeSetUserAccessKeyAttributesRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::SetUserAccessKeyAttributesRequest& request)
{
    response.set_has_affected_row_count(false);
    m_instance.updateUserAccessKey(
            request.m_userName, request.m_keyName, request.m_params, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeRenameUserAccessKeyRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::RenameUserAccessKeyRequest& request)
{
    sendNotImplementedYet(response);
}

void RequestHandler::executeAddUserTokenRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::AddUserTokenRequest& request)
{
    response.set_has_affected_row_count(false);
    const auto result = m_instance.createUserToken(request.m_userName, request.m_tokenName,
            request.m_value, request.m_description, request.m_expirationTimestamp, m_userId);

    if (!request.m_value) {
        constexpr auto prefixLen = ::ct_strlen(kTokenResponsePrefix);
        std::string tokenText(prefixLen + result.second.size() * 2, ' ');
        std::strcpy(tokenText.data(), kTokenResponsePrefix);
        boost::algorithm::hex_lower(
                result.second.cbegin(), result.second.cend(), tokenText.begin() + prefixLen);
        response.add_freetext_message(std::move(tokenText));
    }

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeDropUserTokenRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::DropUserTokenRequest& request)
{
    response.set_has_affected_row_count(false);
    m_instance.dropUserToken(
            request.m_userName, request.m_tokenName, !request.m_ifExists, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeSetUserTokenAttributesRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::SetUserTokenAttributesRequest& request)
{
    response.set_has_affected_row_count(false);
    m_instance.updateUserToken(request.m_userName, request.m_tokenName, request.m_params, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

void RequestHandler::executeRenameUserTokenRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::RenameUserTokenRequest& request)
{
    sendNotImplementedYet(response);
}

void RequestHandler::executeCheckUserTokenRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::CheckUserTokenRequest& request)
{
    response.set_has_affected_row_count(false);
    m_instance.checkUserToken(
            request.m_userName, request.m_tokenName, request.m_tokenValue, m_userId);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

}  // namespace siodb::iomgr::dbengine
