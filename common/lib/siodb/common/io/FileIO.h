// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <stddef.h>

// System headers
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/** Constant for ignoring signals */
#define kIgnoreSignals 1

/**
 * Attempts to close file until success or error other than EINTR.
 * @param fd File descriptor.
 * @returns 0 on success, -1 on error other than EINTR.
 */
int closeFileIgnoreSignal(int fd);

/**
 * Reads specified amount of data from file.
 * If read() system call succeeds but reads less then specified, next attempts are taken
 * to read subsequent portion of data until specified number of bytes is read.
 * @param fd File descriptor.
 * @param buffer A buffer for data.
 * @param size Desired data size.
 * @param ignoreSignal If nonzero, read failures with error code EINTR cause
 *                     next read attempt instead of failure.
 * @return Number of bytes actually read. Value less than requested indicates error.
 *         In such case, errno is set to an error code. If error code is 0,
 *         then end of file is reached.
 */
size_t readExact(int fd, void* buffer, size_t size, int ignoreSignal);

/**
 * Reads specified amount of data from file starting at a given offset.
 * If pread() system call succeeds but reads less then specified, next attempts are taken
 * to read subsequent portion of data until specified number of bytes is read.
 * @param fd File descriptor.
 * @param buffer A buffer for data.
 * @param size Desired data size.
 * @param offset Starting offset.
 * @param ignoreSignal If nonzero, read failures with error code EINTR cause
 *                     next read attempt instead of failure.
 * @return Number of bytes actually read. Value less than requested indicates error.
 *         In such case, errno is set to an code. If error code is 0,
 *         then end of file is reached.
 */
size_t preadExact(int fd, void* buffer, size_t size, off_t offset, int ignoreSignal);

/**
 * Writes specified amount of data to file.
 * If write() system call succeeds but writes less then specified, next attempts are taken
 * to write subsequent portion of data, until specified number of bytes written.
 * @param fd File descriptor.
 * @param buffer A buffer with data.
 * @param size Data size.
 * @param ignoreSignal If nonzero, write failure with error code EINTR causes
 *                     next write attempt instead of failure.
 * @return Number of bytes known to be written successfully. Value less than requested
 *         indicates error. In such case, errno is set to an error code.
 */
size_t writeExact(int fd, const void* buffer, size_t size, int ignoreSignal);

/**
 * Writes specified amount of data to file starting at a give offset.
 * If pwrite() system call succeeds but writes less then specified, next attempts are taken
 * to write subsequent portion of data, until specified number of bytes written.
 * @param fd File descriptor.
 * @param buffer A buffer with data.
 * @param size Data size.
 * @param offset Starting offset.
 * @param ignoreSignal If nonzero, write failure with error code EINTR causes
 *                     next write attempt instead of failure.
 * @return Number of bytes known to be written successfully. Value less than requested
 *         indicates error. In such case, errno is set to an error code.
 */
size_t pwriteExact(int fd, const void* buffer, size_t size, off_t offset, int ignoreSignal);

/**
 * Wraps posix_fallocate() for purpose of ignoring EINTR.
 * @param fd File descriptor.
 * @param offset File offset.
 * @param len Block length.
 * @return Zero on success, -1 on error other than EINTR. In the latter case, errno is set.
 */
int posixFileAllocateExact(int fd, off_t offset, off_t len);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
