// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::net {

/**
 * Waits for data ready for read event (EPOLLIN).
 * @param epollFd epoll file descriptor.
 * @param ignoreEintr Indication that EINTR error should be ignored.
 * @throw ConnectionError if connection was closed or handgup.
 * @throw SystemError if epoll_wait error happens.
 */
void epollWaitForData(int epollFd, bool ignoreEintr);

/**
 * Creates epoll file descriptor.
 * @param fd File descriptor assigned to epoll file descriptor.
 * @param events Initial event bitmast which should be. catched by epoll_wait for @ref fd
 * @throw SystemError if system error happens
 */
int createEpollFd(int fd, int events);

}  // namespace siodb::net
