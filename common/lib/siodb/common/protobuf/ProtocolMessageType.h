// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::protobuf {

/** Message type code */
enum class ProtocolMessageType : unsigned {
    kNoMessage = 0,  // no message
    kCommand,  // message Command
    kServerResponse,  // message ServerResponse
    kDatabaseEngineRequest,  // message DatabaseEngineRequest
    kDatabaseEngineResponse,  // message DatabaseEngineResponse
    kClientBeginSessionRequest,  // message BeginSessionRequest
    kClientBeginSessionResponse,  // message BeginSessionResponse
    kClientAuthenticationRequest,  // message ClientAuthenticationRequest
    kClientAuthenticationResponse,  // message ClientAuthenticationResponse
    kBeginAuthenticateUserRequest,  // message BeginAuthenticateUserRequest
    kBeginAuthenticateUserResponse,  // message BeginAuthenticateUserResponse
    kAuthenticateUserRequest,  // message AuthenticateUserRequest
    kAuthenticateUserResponse,  // message AuthenticateUserResponse
    kDatabaseEngineRestRequest,  // message DatabaseEngineRestRequest
    kValidateUserTokenRequest,  // message ValidateUserTokenRequest
    kServerInformationRequest,  // message ServerInformationRequest
    kServerInformation,  // message ServerInformation

    kMax  // message type limit
};

}  // namespace siodb::protobuf
