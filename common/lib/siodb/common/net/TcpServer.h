// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

namespace siodb::net {

/**
 * Creates TCP server on the specified address and port.
 * @param Server address domain. Can be AF_INET or AF_INET6.
 * @param serverAddress Server address, can be nullptr. In this case,
 *                listen on all available addresses.
 * @param port Server port number
 * @param backlog Listener backlog.
 * @return Server file descriptor.
 * @throw std::invalid_argument
 *                              - if server path is too long.
 *                              - if domain is not AF_INET or AF_INET6.
 *                              - if server port is out of range
 * @throw std::system_error if setting up server socket fails.
 */
int createTcpServer(int domain, const char* serverAddress, int port, int backlog);

}  // namespace siodb::net
