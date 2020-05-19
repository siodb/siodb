// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

namespace siodb::net {

/**
 * Creates UNIX server on the specified path.
 * @param serverSocketPath Server socker path.
 * @param backlog Listener backlog.
 * @param removePreviousSocket Remove previous socket on the same path if exists.
 * @return Server file descriptor.
 * @throw std::invalid_argument if server path is too long.
 * @throw std::system_error if server socket could not be established.
 */
int createUnixServer(
        const std::string& serverSocketPath, int backlog, bool removePreviousSocket = true);

}  // namespace siodb::net
