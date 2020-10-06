// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

namespace siodb::net {

/**
 * Opens TCP connection to server.
 * @param host Target host.
 * @params port Target port.
 * @param closeOnExecute Set FD_CLOEXEC flag on the resulting file descriptor.
 * @throw std::invalid_argument if host name or port number is invalid
 *                               or host can't be resolved
 * @throw std::system_error if server socket could not be established.
 */
int openTcpConnection(const std::string& host, int port, bool closeOnExecute = true);

}  // namespace siodb::net
