// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "HelperMacros.h"
#include "../crt_ext/compiler_defs.h"

// CRT headers
#include <cerrno>

// STL headers
#include <utility>

// System headers
#include <fcntl.h>
#include <unistd.h>

namespace siodb {

/**
 * File descriptor lifetime guard. Holds file descriptor until end of current scope,
 * then closes it.
 */
class FdGuard {
public:
    /**
     * Initialize object of the class FdGuard.
     * @param fd File descriptor to be guarded.
     */
    explicit FdGuard(int fd = -1) noexcept
        : m_fd(fd)
    {
    }

    /**
     * Initialize object of the class FdGuard.
     * @param src Source file descriptor guard.
     */
    FdGuard(FdGuard&& src) noexcept
        : m_fd(src.m_fd)
    {
        src.m_fd = -1;
    }

    /** Cleans up object of the class FdGuard */
    ~FdGuard() noexcept
    {
        reset();
    }

    DECLARE_NONCOPYABLE(FdGuard);

    /**
     * Returns file descriptor validity flag.
     * @return true if there is valid file descriptor, false otherwise.
     */
    bool isValidFd() const noexcept
    {
        return m_fd >= 0;
    }

    /**
     * Returns file descriptor.
     * @return file descriptor.
     */
    int getFd() const noexcept
    {
        return m_fd;
    }

    /**
     * Releases current file descriptor.
     * @return Released file descriptor.
     */
    int release() noexcept
    {
        const int fd = m_fd;
        m_fd = -1;
        return fd;
    }

    /**
     * Resets guard with new file descriptor.
     * @param fd New file descriptor.
     * @return true if there were no file descriptor before or it was closed successfully,
     *         false otherwise. If false is returned, errno is set to actual error number.
     */
    bool reset(int fd = -1) noexcept
    {
        bool ret = true;
        if (isValidFd()) {
            ::close(m_fd);
            ret = (errno == 0);
        }
        m_fd = fd;
        return ret;
    }

    /**
     * Swaps guard contents.
     * @param other Other guard object.
     */
    void swap(FdGuard& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) std::swap(m_fd, other.m_fd);
    }

    /**
     * Modifies file descriptor flag value.
     * @param flag Flag mask.
     * @param value New value of the flag.
     * @return true on success, false on error, errno is set.
     */
    bool setFdFlag(int flag, bool value) noexcept
    {
        int flags = ::fcntl(m_fd, F_GETFD, 0);
        if (flags < 0) return false;
        if (value)
            flags |= flag;
        else
            flags = flags & (~flag);
        return ::fcntl(m_fd, F_SETFD, flags) >= 0;
    }

    /**
     * Locks the file.
     * @param flag Lock command (F_LOCK, F_TLOCK, F_ULOCK, F_TEST).
     * @param len Position in file.
     * @return true on success, false on error, errno is set.
     */
    bool lock(int flag, off_t len) noexcept
    {
        return lockf(m_fd, flag, len) == 0;
    }

    /**
     * Move assignment operator.
     * @param src Source object.
     * @return this object.
     */
    FdGuard& operator=(FdGuard&& src) noexcept
    {
        if (&src != this) {
            swap(src);
            src.reset();
        }
        return *this;
    }

private:
    /** File descriptor */
    int m_fd;
};

/**
 * Swaps two FdGuard objects.
 * @param a first object.
 * @param b second object.
 */
inline void swap(FdGuard& a, FdGuard& b)
{
    a.swap(b);
}

}  // namespace siodb
