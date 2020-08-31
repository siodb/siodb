// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UnixServer.h"

// Project headers
#include "../config/SiodbDefs.h"
#include "../stl_ext/system_error_ext.h"
#include "../utils/CheckOSUser.h"
#include "../utils/FDGuard.h"

// CRT headers
#include <cstring>

// STL headers
#include <sstream>

// System headers
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace siodb::net {

int createUnixServer(const std::string& serverSocketPath, int backlog, bool removePreviousSocket)
{
    struct sockaddr_un addr;

    if (serverSocketPath.length() > sizeof(addr.sun_path) - 1) {
        throw std::invalid_argument("Server path is too long");
    }

    // Remove any previous socket on that path
    if (removePreviousSocket) {
        if (::remove(serverSocketPath.c_str()) < 0 && errno != ENOENT) {
            stdext::throw_system_error(
                    "Can't remove previous UNIX server socket at ", serverSocketPath.c_str());
        }
    }

    // Create socket
    FDGuard socket(::socket(AF_UNIX, SOCK_STREAM, 0));
    if (!socket.isValidFd()) {
        stdext::throw_system_error("Can't create new UNIX server socket");
    }

    // Prevent passing this fd to child processes
    if (!socket.setFdFlag(FD_CLOEXEC, true)) {
        stdext::throw_system_error("Can't set FD_CLOEXEC on the UNIX server socket");
    }

    // Based on idea from here https://stackoverflow.com/q/38095467/1540501
    // Remove group permissions
    if (fchmod(socket.getFD(), 0700) < 0) {
        stdext::throw_system_error("Can't remove group premissions from UNIX server socket");
    }

    // Fill server socket parameters
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strcpy(addr.sun_path, serverSocketPath.c_str());

    // Bind the server
    if (::bind(socket.getFD(), (struct sockaddr*) &addr, (socklen_t) sizeof(addr)) < 0) {
        stdext::throw_system_error("Can't bind UNIX server socket to ", serverSocketPath.c_str());
    }

    // Change ownership
    if (::chown(serverSocketPath.c_str(), -1, utils::getOsGroupId(kAdminGroupName)) < 0) {
        stdext::throw_system_error(
                "Can't change ownerhip of the UNIX server socket at ", serverSocketPath.c_str());
    }

    // Restore group permissions
    // Note: At least in Linux, fchmod() doesn't work correctly here, so use chmod().
    if (::chmod(serverSocketPath.c_str(), 0770) < 0) {
        stdext::throw_system_error("Can't restore group premissions on the UNIX server socket at ",
                serverSocketPath.c_str());
    }

    // Start listening on the server socket
    if (::listen(socket.getFD(), backlog) < 0) {
        stdext::throw_system_error(
                "Can't listen with UNIX server socket at ", serverSocketPath.c_str());
    }

    return socket.release();
}  // namespace siodb::net

}  // namespace siodb::net
