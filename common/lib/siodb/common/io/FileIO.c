// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "FileIO.h"

// CRT headers
#include <errno.h>
#include <stdint.h>

// System headers
#include <fcntl.h>
#include <unistd.h>

int closeFileIgnoreSignal(int fd)
{
#if defined(NEED_CLOSE_EINTR_HANDLING)
    int result;
    do {
        result = close(fd);
    } while (result < 0 && errno == EINTR);
    return result;
#else
    return close(fd);
#endif
}

size_t readExact(int fd, void* buffer, size_t size, int ignoreSignal)
{
    size_t bytesRead = 0;
    while (bytesRead < size) {
        const ssize_t n = read(fd, buffer, size);
        if (n < 0) {
            if (errno == EINTR && ignoreSignal) continue;
            break;
        }
        if (n == 0) {
            errno = 0;
            break;
        }
        bytesRead += n;
        size -= n;
        buffer = (uint8_t*) buffer + n;
    }
    return bytesRead;
}

size_t preadExact(int fd, void* buffer, size_t size, off_t offset, int ignoreSignal)
{
    size_t bytesRead = 0;
    while (bytesRead < size) {
        const ssize_t n = pread(fd, buffer, size, offset);
        if (n < 0) {
            if (errno == EINTR && ignoreSignal) continue;
            break;
        }
        if (n == 0) {
            errno = 0;
            break;
        }
        bytesRead += n;
        size -= n;
        offset += n;
        buffer = (uint8_t*) buffer + n;
    }
    return bytesRead;
}

size_t writeExact(int fd, const void* buffer, size_t size, int ignoreSignal)
{
    size_t bytesWritten = 0;
    while (bytesWritten < size) {
        const ssize_t n = write(fd, buffer, size);
        if (n < 0) {
            if (errno == EINTR && ignoreSignal) continue;
            break;
        }
        bytesWritten += n;
        size -= n;
        buffer = (uint8_t*) buffer + n;
    }
    return bytesWritten;
}

size_t pwriteExact(int fd, const void* buffer, size_t size, off_t offset, int ignoreSignal)
{
    size_t bytesWritten = 0;
    while (bytesWritten < size) {
        const ssize_t n = pwrite(fd, buffer, size, offset);
        if (n < 0) {
            if (errno == EINTR && ignoreSignal) continue;
            break;
        }
        bytesWritten += n;
        size -= n;
        offset += n;
        buffer = (uint8_t*) buffer + n;
    }
    return bytesWritten;
}

int posixFileAllocateExact(int fd, off_t offset, off_t len)
{
    int errorCode;
    do {
        errorCode = 0;
        if (posix_fallocate(fd, offset, len) < 0) {
            errorCode = errno;
        }
    } while (errorCode == EINTR);
    return errorCode ? -1 : 0;
}
