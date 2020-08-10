// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SocketDomain.h"

// STL headers
#include <stdexcept>

// System headers
#include <sys/socket.h>

namespace siodb::net {

namespace {

[[noreturn]] void throwUnsupporedSocketDomainException()
{
    throw std::invalid_argument(
            "Invalid connection listener socket domain, only IPv4, IPv6 and Unix sockets are "
            "supported");
}

[[noreturn]] void throwUnsupporedSocketDomainExceptionIpOnly()
{
    throw std::invalid_argument(
            "Invalid connection listener socket domain, only IPv4 and IPv6 sockets are "
            "supported");
}

}  // namespace

int checkSocketDomain(int socketDomain)
{
    switch (socketDomain) {
        case AF_INET:
        case AF_INET6:
        case AF_UNIX: return socketDomain;
        default: throwUnsupporedSocketDomainException();
    }
}

int checkSocketDomainIpOnly(int socketDomain)
{
    switch (socketDomain) {
        case AF_INET:
        case AF_INET6: return socketDomain;
        default: throwUnsupporedSocketDomainExceptionIpOnly();
    }
}

const char* getSocketDomainName(int socketDomain)
{
    switch (socketDomain) {
        case AF_INET: return "IPv4";
        case AF_INET6: return "IPv6";
        case AF_UNIX: return "UNIX"; ;
        default: throwUnsupporedSocketDomainException();
    }
}

}  // namespace siodb::net
