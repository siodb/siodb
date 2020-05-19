// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TcpConnection.h"

// Project headers
#include "NetConstants.h"
#include "internal/AddressInfoGuard.h"
#include "../stl_ext/utility_ext.h"
#include "../utils/Debug.h"
#include "../utils/FileDescriptorGuard.h"
#include "../utils/SystemError.h"

// STL headers
#include <iostream>
#include <sstream>

// System headers
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Boost headers
#include <boost/algorithm/string.hpp>

namespace siodb::net {

int openTcpConnection(const std::string& host, int port, bool closeOnExecute)
{
    std::ostringstream errors;
    errors << "Could not connect to any resolved address:" << std::endl;

    // Validate host name
    const auto host1 = boost::trim_copy(host);
    if (host1.empty()) {
        throw std::invalid_argument("Invalid host name");
    }

    // Validate port number
    if (port < kMinPortNumber || port > kMaxPortNumber) {
        throw std::invalid_argument("Invalid TCP port number");
    }

    // Initialize socket address info
    union {
        struct sockaddr_in v4;
        struct sockaddr_in6 v6;
    } addr;
    std::memset(&addr, 0, sizeof(addr));

    // Initialize default address info instance.
    struct addrinfo addrInfo1;
    std::memset(&addrInfo1, 0, sizeof(addrInfo1));
    addrInfo1.ai_socktype = SOCK_STREAM;
    addrInfo1.ai_protocol = IPPROTO_TCP;
    addrInfo1.ai_canonname = stdext::as_mutable_ptr(host1.c_str());
    struct addrinfo* addrInfos = &addrInfo1;

    // Determine IP address
    if (inet_pton(AF_INET, host1.c_str(), &addr.v4.sin_addr) == 1) {
        // IPv4 address detected
        addr.v4.sin_family = AF_INET;
        addr.v4.sin_port = htons(port);
        addrInfo1.ai_family = AF_INET;
        addrInfo1.ai_addrlen = sizeof(addr.v4);
        addrInfo1.ai_addr = reinterpret_cast<struct sockaddr*>(&addr.v4);
    } else if (inet_pton(AF_INET6, host1.c_str(), &addr.v6.sin6_addr) == 1) {
        // IPv6 address detected
        addr.v6.sin6_family = AF_INET6;
        addr.v6.sin6_port = htons(port);
        addrInfo1.ai_family = AF_INET6;
        addrInfo1.ai_addrlen = sizeof(addr.v6);
        addrInfo1.ai_addr = reinterpret_cast<struct sockaddr*>(&addr.v6);
    } else {
        // Neither of above, try to resolve host name
        struct addrinfo hints;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_ADDRCONFIG | AI_V4MAPPED | AI_ALL;
        const auto portStr = std::to_string(port);
        const int errorCode = getaddrinfo(host1.c_str(), portStr.c_str(), &hints, &addrInfos);
        if (errorCode != 0) {
            std::ostringstream err;
            err << "Could not resolve host " << host1 << ": " << gai_strerror(errorCode);
            throw std::invalid_argument(err.str());
        }
        //std::cout << "Resolved host" << std::endl;
    }

    AddrInfosGuard guard(addrInfos, addrInfos != &addrInfo1);

    int lastError = 0;
    struct addrinfo* currentAddrInfo = addrInfos;
    while (currentAddrInfo != nullptr) {
        // Create socket
        FileDescriptorGuard socket(::socket(currentAddrInfo->ai_family,
                currentAddrInfo->ai_socktype, currentAddrInfo->ai_protocol));
        if (!socket.isValidFd()) {
            utils::throwSystemError("Can't create TCP socket");
        }

        // If requested, prevent passing this fd to child processes
        if (closeOnExecute && !socket.setFdFlag(FD_CLOEXEC, true)) {
            utils::throwSystemError("Can't set FD_CLOEXEC on the client TCP socket");
        }

        // Convert actual address to string and show message
        const char* serverName =
                currentAddrInfo->ai_canonname ? currentAddrInfo->ai_canonname : host1.c_str();
        char serverAddr[128];
        const char* result;
        if (currentAddrInfo->ai_family == AF_INET) {
            // IPv4 address
            result = inet_ntop(currentAddrInfo->ai_family,
                    &reinterpret_cast<struct sockaddr_in*>(currentAddrInfo->ai_addr)->sin_addr,
                    serverAddr, sizeof(serverAddr));
        } else {
            // IPv6 address
            result = inet_ntop(currentAddrInfo->ai_family,
                    &reinterpret_cast<struct sockaddr_in6*>(currentAddrInfo->ai_addr)->sin6_addr,
                    serverAddr, sizeof(serverAddr));
        }
        if (!result) {
            *serverAddr = '\0';
        }

        // Connect to server
        if (::connect(socket.getFd(), currentAddrInfo->ai_addr, currentAddrInfo->ai_addrlen) < 0) {
            lastError = errno;
            errors << "Could not connect to " << serverName << ":" << port << ": "
                   << std::strerror(lastError) << '.' << std::endl;
            currentAddrInfo = currentAddrInfo->ai_next;
            continue;
        }

        // Connected to server
        return socket.release();
    }

    errors << "Last error";
    throw std::system_error(lastError, std::generic_category(), errors.str().c_str());
}

}  // namespace siodb::net
