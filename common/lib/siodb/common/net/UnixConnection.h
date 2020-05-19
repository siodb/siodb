// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

namespace siodb::net {

/**
 * Opens UNIX connection to server on the specified path.
 * @param serverSocketPath Server socket path.
 * @param closeOnExecute Set FD_CLOEXEC flag on the resulting file descriptor.
 * @return Connection file descriptor.
 * @throw std::invalid_argument if server path is too long.
 * @throw std::system_error if connection fails.
 */
int openUnixConnection(const std::string& serverSocketPath, bool closeOnExecute = true);

}  // namespace siodb::net
