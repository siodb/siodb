// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "HelperMacros.h>
#include "WaitInterruptedException.h"

// STL headers
#include <condition_variable>
#include <deque>
#include <mutex>

namespace siodb::utils {

/**
 * A thread-safe queue.
 * @tparam T Element type.
 */
template<typename T>
class ConcurrentQueue {
public:
    /** Initializes object of class ConcurrentQueue */
    ConcurrentQueue()
        : m_interruptRequested(false)
    {
    }

    DECLARE_NONCOPYABLE(ConcurrentQueue);

    /**
     * Pushes element to queue.
     * @param item Pushing item
     */
    void push(const T& item)
    {
        std::unique_lock lock(m_mutex);
        m_queue.push_back(item);
        m_cond.notify_one();
    }

    /**
     * Pushes element to the queue.
     * @param e An element.
     */
    void push(T&& e)
    {
        std::unique_lock lock(m_mutex);
        m_queue.push_back(std::move(e));
        m_cond.notify_one();
    }

    /** 
     * Pops element from the queue. If no elements available, waits for an element to appear.
     * Wait can be interrupted by calling interrupt().
     * @return Popped element.
     * @throw WaitInterruptedException if operation was interrupted.
     */
    T pop()
    {
        std::unique_lock lock(m_mutex);

        while (!m_interruptRequested && m_queue.empty())
            m_cond.wait(lock);

        if (m_interruptRequested)
            throw WaitInterruptedException("ConcurrentQueue::pop(): wait interrupted");

        auto e = std::move(m_queue.front());
        m_queue.pop_front();
        return e;
    }

    /**
     * Returns size of queue
     * @return Size of queue
     */
    std::size_t size() const
    {
        std::unique_lock lock(m_mutex);
        return m_queue.size();
    }

    /**
     * Returns indication that queue is empty
     * @return True if queue is empty, false otherwise
     */
    bool empty() const
    {
        std::unique_lock lock(m_mutex);
        return m_queue.empty();
    }

    /** Requests interrupt of waiting for an element in the pop(). */
    void request_interrupt()
    {
        std::unique_lock lock(m_mutex);
        m_interruptRequested = true;
        m_cond.notify_one();
    }

    /** Cancels interrupt request. */
    void cancel_interrupt()
    {
        std::unique_lock lock(m_mutex);
        m_interruptRequested = false;
    }

private:
    /** Access synchronization object. */
    mutable std::mutex m_mutex;

    /** Condition variable for signaling about data availability. */
    std::condition_variable m_cond;

    /** Underlying queue */
    std::deque<T> m_queue;

    /** Indicates that pop() operation must be interrupted. */
    bool m_interruptRequested;
};

}  // namespace siodb::utils
