// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UnixConnection.h"

// Project headers
#include "../stl_ext/system_error_ext.h"
#include "../utils/FDGuard.h"

// CRT headers
#include <cstring>

// STL headers
#include <sstream>

// System headers
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace siodb::net {

int openUnixConnection(const std::string& serverSocketPath, bool closeOnExecute)
{
    struct sockaddr_un addr;

    // Validate server path
    if (serverSocketPath.length() > sizeof(addr.sun_path) - 1) {
        throw std::invalid_argument("Server path is too long");
    }

    // Create socket
    FDGuard socket(::socket(AF_UNIX, SOCK_STREAM, 0));
    if (!socket.isValidFd()) {
        stdext::throw_system_error("Can't create new UNIX client socket");
    }

    // If requested, prevent passing this fd to child processes
    if (closeOnExecute && !socket.setFdFlag(FD_CLOEXEC, true)) {
        stdext::throw_system_error("Can't set FD_CLOEXEC on the UNIX client socket");
    }

    // Fill connection parameters
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strcpy(addr.sun_path, serverSocketPath.c_str());

    // Attempt to connect
    if (::connect(socket.getFD(), (struct sockaddr*) &addr, (socklen_t) sizeof(addr)) < 0) {
        stdext::throw_system_error(
                "Can't connect via UNIX client socket to the ", serverSocketPath.c_str());
    }

    return socket.release();
}

}  // namespace siodb::net
