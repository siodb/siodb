// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TcpServer.h"

// Internal headers
#include "detail/AddressInfoGuard.h"

// Project headers
#include "NetConstants.h"
#include "../stl_ext/system_error_ext.h"
#include "../stl_ext/utility_ext.h"
#include "../utils/FDGuard.h"
#include "../utils/HelperMacros.h"

// CRT headers
#include <cerrno>
#include <cstring>

// STL error
#include <sstream>

// System headers
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace siodb::net {

int createTcpServer(int domain, const char* serverAddress, int port, int backlog)
{
    // Validate domain
    switch (domain) {
        // Valid domains
        case AF_INET:
        case AF_INET6: break;
        // Anything else
        default: throw std::invalid_argument("Invalid socket domain");
    }

    // Validate port number
    if (port < kMinPortNumber || port > kMaxPortNumber) {
        throw std::invalid_argument("Invalid TCP port number");
    }

    union {
        struct sockaddr_in v4;
        struct sockaddr_in6 v6;
    } addr;

    // Initialize default address info instance.
    std::string localhostStr(kLocalhost);
    struct addrinfo addrInfo1;
    std::memset(&addrInfo1, 0, sizeof(addrInfo1));
    addrInfo1.ai_family = domain;
    addrInfo1.ai_socktype = SOCK_STREAM;
    addrInfo1.ai_protocol = IPPROTO_TCP;
    addrInfo1.ai_canonname = stdext::as_mutable_ptr(localhostStr.c_str());
    struct addrinfo* addrInfos = &addrInfo1;

    // Determine specific server address
    if (serverAddress != nullptr) {
        // Determine IP address
        if (domain == AF_INET && inet_pton(AF_INET, serverAddress, &addr.v4.sin_addr) == 1) {
            // IPv4 address detected
            addr.v4.sin_family = AF_INET;
            addr.v4.sin_port = htons(port);
            addrInfo1.ai_family = AF_INET;
            addrInfo1.ai_addrlen = sizeof(addr.v4);
            addrInfo1.ai_addr = reinterpret_cast<struct sockaddr*>(&addr.v4);
        } else if (domain == AF_INET6
                   && inet_pton(AF_INET6, serverAddress, &addr.v6.sin6_addr) == 1) {
            // IPv6 address detected
            addr.v6.sin6_family = AF_INET6;
            addr.v6.sin6_port = htons(port);
            addrInfo1.ai_family = AF_INET6;
            addrInfo1.ai_addrlen = sizeof(addr.v6);
            addrInfo1.ai_addr = reinterpret_cast<struct sockaddr*>(&addr.v6);
        } else {
            struct addrinfo hints;
            std::memset(&hints, 0, sizeof(hints));
            hints.ai_family = domain;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_V4MAPPED | AI_ALL;
            const auto portStr = std::to_string(port);
            const int errorCode = getaddrinfo(serverAddress, portStr.c_str(), &hints, &addrInfos);
            if (errorCode != 0) {
                std::ostringstream err;
                err << "Could not resolve server address " << serverAddress << ": "
                    << gai_strerror(errorCode);
                throw std::invalid_argument(err.str());
            }
        }
    } else {
        if (domain == AF_INET) {
            // All IPv4 addresses
            addr.v4.sin_addr.s_addr = INADDR_ANY;
            addr.v4.sin_family = AF_INET;
            addr.v4.sin_port = htons(port);
            addrInfo1.ai_family = AF_INET;
            addrInfo1.ai_addrlen = sizeof(addr.v4);
            addrInfo1.ai_addr = reinterpret_cast<struct sockaddr*>(&addr.v4);
        } else {
            // All IPv6 addresses
            addr.v6.sin6_family = AF_INET6;
            addr.v6.sin6_addr = in6addr_any;
            addr.v6.sin6_port = htons(port);
            addrInfo1.ai_family = AF_INET6;
            addrInfo1.ai_addrlen = sizeof(addr.v6);
            addrInfo1.ai_addr = reinterpret_cast<struct sockaddr*>(&addr.v6);
        }
    }

    AddrInfosGuard guard(addrInfos, addrInfos != &addrInfo1);

    // Create socket using first resolved address
    FDGuard socket(::socket(addrInfos->ai_family, addrInfos->ai_socktype, addrInfos->ai_protocol));
    if (!socket.isValidFd()) {
        stdext::throw_system_error("Can't create new socket");
    }

    // Prevent passing this fd to child processes
    if (!socket.setFdFlag(FD_CLOEXEC, true)) {
        stdext::throw_system_error("Can't set FD_CLOEXEC on the server TCP socket");
    }

    // Enable address reuse
    int flag = 1;
    if (::setsockopt(socket.getFD(), SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        stdext::throw_system_error("Can't enable address resuse on the TCP socket");
    }

    // Bind the server
    if (::bind(socket.getFD(), addrInfos->ai_addr, addrInfos->ai_addrlen) < 0) {
        const int errorCode = errno;
        std::ostringstream err;
        err << "Can't bind TCP server socket to " << (serverAddress ? serverAddress : "*") << ':'
            << port << ": " << std::strerror(errorCode);
        throw std::system_error(errorCode, std::generic_category(), err.str());
    }

    // Start listening on the server socket
    if (::listen(socket.getFD(), backlog) < 0) {
        const int errorCode = errno;
        std::ostringstream err;
        err << "Can't listen with TCP server socket on the "
            << (serverAddress ? serverAddress : "*") << ':' << port << ": "
            << std::strerror(errorCode);
        throw std::system_error(errorCode, std::generic_category(), err.str());
    }

    return socket.release();
}

}  // namespace siodb::net
