// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "stdexcept_ext.h"

// STL headers
#include <condition_variable>
#include <deque>
#include <mutex>

namespace stdext {

/**
 * A thread-safe queue.
 * @tparam T Element type.
 */
template<typename T>
class concurrent_queue {
public:
    /** Initializes object of class concurrent_queue */
    concurrent_queue()
        : m_interrupt_requested(false)
    {
    }

    /** Copy construction disabled. */
    concurrent_queue(const concurrent_queue&) = delete;

    /** Copying assignment disbled. */
    concurrent_queue& operator=(const concurrent_queue&) = delete;

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
     * @throw wait_interrupted_error if operation was interrupted.
     */
    T pop()
    {
        std::unique_lock lock(m_mutex);

        while (!m_interrupt_requested && m_queue.empty())
            m_cond.wait(lock);

        if (m_interrupt_requested)
            throw wait_interrupted_error("concurrent_queue::pop(): wait interrupted");

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
        m_interrupt_requested = true;
        m_cond.notify_one();
    }

    /** Cancels interrupt request. */
    void cancel_interrupt()
    {
        std::unique_lock lock(m_mutex);
        m_interrupt_requested = false;
    }

private:
    /** Access synchronization object. */
    mutable std::mutex m_mutex;

    /** Condition variable for signaling about data availability. */
    std::condition_variable m_cond;

    /** Underlying queue */
    std::deque<T> m_queue;

    /** Indicates that pop() operation must be interrupted. */
    bool m_interrupt_requested;
};

}  // namespace stdext
