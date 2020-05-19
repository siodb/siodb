// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "EpollHelpers.h"

// Project headers
#include "ConnectionError.h"
#include "../utils/SystemError.h"

// System headers
#include <sys/epoll.h>
#include <unistd.h>

namespace siodb::net {

void epollWaitForData(int epollFd, bool ignoreEintr)
{
    struct epoll_event epollEvent;
    while (true) {
        const auto n = epoll_wait(epollFd, &epollEvent, 1, -1);
        if (n < 0) {
            if (ignoreEintr && errno == EINTR)
                continue;
            else
                utils::throwSystemError("epoll_wait failed");
        }

        if (epollEvent.events & EPOLLERR)
            throw ConnectionError("Connection closed");
        else if (epollEvent.events & EPOLLHUP)
            throw ConnectionError("Connection hangup");
        else if (epollEvent.events & EPOLLIN)
            return;
    }
}

int createEpollFd(int fd, int events)
{
    const int epollFd = ::epoll_create1(0);
    if (epollFd < 0) utils::throwSystemError("epoll_create1 failed");

    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) != 0) {
        const int errorCode = errno;
        ::close(epollFd);
        utils::throwSystemError(errorCode, "epoll_ctl failed");
    }

    return epollFd;
}

}  // namespace siodb::net
