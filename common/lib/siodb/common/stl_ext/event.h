// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace stdext {

class event {
public:
    /**
     * Initializes new object of class event.
     * @param autoReset Flag showing whether to auto-reset event after it has been signaled.
     */
    explicit event(bool autoReset = false) noexcept
        : m_signaled(false)
        , m_auto_reset(autoReset)
    {
    }

    /** Copy constrution is prohibited */
    event(const event&) = delete;
    /** Copy assignment is prohibited */
    event& operator=(const event&) = delete;

    /**
     * Returns current state.
     * @return Current state.
     */
    bool signaled() const
    {
        std::lock_guard lock(m_mutex);
        return m_signaled;
    }

    /** Waits for even to happen. Returns when even happened or by exception */
    void wait()
    {
        std::unique_lock lock(m_mutex);
        while (!m_signaled)
            m_cond.wait(lock);
        if (m_auto_reset) m_signaled = false;
    }

    /**
     * Waits for event to happen. Returns when even happened or by exception or when timeout expired.
     * @param timeoutTime Time when to stop waiting.
     * @return true, if even caught, false if timeout has expired.
     */
    template<class Clock, class Duration>
    bool wait_until(const std::chrono::time_point<Clock, Duration>& timeoutTime)
    {
        std::unique_lock lock(m_mutex);
        while (!m_signaled) {
            if (m_cond.wait_until(lock, timeoutTime) == std::cv_status::timeout) return false;
        }
        if (m_auto_reset) m_signaled = false;
        return true;
    }

    /**
     * Waits for event to happen. Returns when even happened or by exception or when timeout expired.
     * @param timeout Timeout to wait for.
     * @return true, if even caught, false if timeout has expired.
     */
    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout)
    {
        std::unique_lock lock(m_mutex);
        while (!m_signaled) {
            if (m_cond.wait_for(lock, timeout) == std::cv_status::timeout) return false;
        }
        if (m_auto_reset) m_signaled = false;
        return true;
    }

    /** Signal event and notify single waiting thread. */
    void notify_one()
    {
        std::lock_guard lock(m_mutex);
        m_signaled = true;
        m_cond.notify_all();
    }

    /** Signal event and notify all waiting threads. */
    void notify_all()
    {
        std::lock_guard lock(m_mutex);
        m_signaled = true;
        m_cond.notify_one();
    }

    /** Resets state to not signaled */
    void reset()
    {
        std::lock_guard lock(m_mutex);
        m_signaled = false;
    }

private:
    /** Value access synchronization object */
    mutable std::mutex m_mutex;
    /** Condition variable */
    std::condition_variable m_cond;
    /** Current state */
    bool m_signaled;
    /** Flag showing whether to auto-reset state after signal */
    const bool m_auto_reset;
};

}  // namespace stdext
