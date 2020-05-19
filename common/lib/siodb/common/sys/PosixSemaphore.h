// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../utils/HelperMacros.h"

// CRT headers
#include <cerrno>

// STL headers
#include <stdexcept>

// System headers
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>

namespace siodb {

/** POSIX semaphore holder */
class PosixSemaphore {
public:
    /**
     * Open new semaphore.
     * @param path Semaphore path.
     * @param openFlags Opening flags.
     * @param value Initial value.
     * @param mode Semaphore access mode.
     */
    PosixSemaphore(
            const char* path, int openFlags = 0, unsigned value = 0, mode_t mode = 0600) noexcept
        : m_sem((openFlags & O_CREAT) ? sem_open(path, openFlags, mode, value)
                                      : sem_open(path, openFlags))
        , m_lastError(0)
        , m_owner(true)
    {
        if (!m_sem) m_lastError = errno;
    }

    /**
     * Create semaphore object from existing handles.
     * @param sem Existing semaphore handle.
     * @param owner Semaphore handle ownerhip flag, if true, object becomes owner.
     */
    PosixSemaphore(sem_t* sem, bool owner = true) noexcept
        : m_sem(sem)
        , m_lastError(0)
        , m_owner(owner)
    {
    }

    /**
     * Move constructor.
     * @param src Source object.
     */
    explicit PosixSemaphore(PosixSemaphore&& src) noexcept
        : m_sem(src.m_sem)
        , m_lastError(src.m_lastError)
        , m_owner(src.m_owner)
    {
        src.m_sem = nullptr;
        src.m_lastError = 0;
    }

    /** De-initialize object, frees owned system resource */
    ~PosixSemaphore()
    {
        if (m_sem && m_owner) {
            sem_close(m_sem);
        }
    }

    DECLARE_NONCOPYABLE(PosixSemaphore);

    /**
     * Move assignment.
     * @param src Source object.
     */
    PosixSemaphore& operator=(PosixSemaphore&& src) noexcept
    {
        if (&src != this) {
            m_sem = src.m_sem;
            m_lastError = src.m_lastError;
            m_owner = src.m_owner;
            src.m_sem = nullptr;
            src.m_lastError = 0;
        }
        return *this;
    }

    /**
     * Returns semaphore handle.
     * @return semaphore handle.
     */
    sem_t* getHandle() const noexcept
    {
        return m_sem;
    }

    /**
     * Returns semaphore handle validity flag.
     * @return true if object owns valid semaphore handle, false otherwise.
     */
    bool isValid() const noexcept
    {
        return m_sem != nullptr;
    }

    /**
     * Returns semaphore handle ownerhip flag.
     * @return true if object owns its semaphore handle, false otherwise.
     */
    bool isOwner() const noexcept
    {
        return m_owner;
    }

    /**
     * Returns last error code.
     * @return true if object owns its semaphore handle, false otherwise.
     */
    bool getLastError() const noexcept
    {
        return m_lastError;
    }

    /**
     * Gets semaphore value.
     * @return semaphore value.
     */
    int getValue() const
    {
        checkSemaphoreHandle();
        int value = 0;
        sem_getvalue(m_sem, &value);
        return value;
    }

    /** Post semaphore */
    void post() const
    {
        checkSemaphoreHandle();
        sem_post(m_sem);
    }

    /** Wait on semaphore */
    void wait() const
    {
        checkSemaphoreHandle();
        sem_wait(m_sem);
    }

private:
    /** Checks whether semaphore handle is valid */
    void checkSemaphoreHandle() const
    {
        if (!m_sem) throw std::runtime_error("Invalid semaphore handle");
    }

private:
    /** Semaphore handle */
    sem_t* m_sem;

    /** Last error code */
    int m_lastError;

    /** Semaphore handle ownership flag */
    bool m_owner;
};

/** Semaphore name guard */
class PosixSemaphoreGuard {
public:
    /**
     * Initializes semaphore guard object.
     * @param semaphoreName Semaphore name.
     */
    explicit PosixSemaphoreGuard(const char* semaphoreName)
        : m_semaphoreName(semaphoreName)
    {
    }

    /**
     * Initializes semaphore guard object.
     * @param semaphoreName Semaphore name.
     */
    explicit PosixSemaphoreGuard(const std::string& semaphoreName)
        : m_semaphoreName(semaphoreName)
    {
    }

    /**
     * Initializes semaphore guard object.
     * @param semaphoreName Semaphore name.
     */
    explicit PosixSemaphoreGuard(std::string&& semaphoreName) noexcept
        : m_semaphoreName(std::move(semaphoreName))
    {
    }

    /**
     * De-initializes object.
     * Removes semaphore name from system.
     */
    ~PosixSemaphoreGuard()
    {
        sem_unlink(m_semaphoreName.c_str());
    }

    DECLARE_NONCOPYABLE(PosixSemaphoreGuard);

    /**
     * Returns semaphore name.
     * @return semaphore name.
     */
    const std::string& getSemaphoreName() const noexcept
    {
        return m_semaphoreName;
    }

private:
    /** Semaphore name */
    const std::string m_semaphoreName;
};

}  // namespace siodb
