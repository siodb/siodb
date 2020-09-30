// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// STL headers
#include <ostream>
#include <string>

namespace siodb::rest_client {

/** REST client parameters. */
struct RestClientParameters {
    /** Host address or name */
    std::string m_host;

    /** Host port */
    int m_port = 0;

    /** Reqeust ID */
    std::uint64_t m_requestId = 1;

    /** Request method */
    std::string m_method;

    /** Object type */
    std::string m_objectType;

    /** Object name */
    std::string m_objectName;

    /** Object identifier */
    std::uint64_t m_objectId = 0;

    /** User name */
    std::string m_user;

    /** User token */
    std::string m_token;

    /** User token file */
    std::string m_tokenFile;

    /** Payload string */
    std::string m_payload;

    /** Payload file */
    std::string m_payloadFile;

    /** Indicates that restcli should drop connection in the middle of sending payload */
    bool m_dropConnection = false;

    /** Indicates that restcli should not print logo */
    bool m_noLogo = false;

    /** Indicates that debug messages should be printed out */
    bool m_printDebugMessages = false;
};

/** Prints logo. */
void printLogo();

/**
 * Executes REST requst to IO Manager.
 * @param params Client parameters.
 * @param os Output stream.
 * @return Exit code.
 */
int executeRestRequest(const RestClientParameters& params, std::ostream& os);

}  // namespace siodb::rest_client
