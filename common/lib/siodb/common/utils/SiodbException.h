// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <stdexcept>
#include <vector>

namespace siodb {

class SiodbException : public std::runtime_error {
public:
    /** Maximum frame count in the stack trace */
    static constexpr std::size_t kMaxStackTraceFrameCount = 128;

public:
    /**
     * Initializes object of class SiodbException
     * @param what Explanatory message.
     * @param stackTraceSkipCount How many frames to skip in the beginning of a stack trace.
     */
    explicit SiodbException(const char* what, [[maybe_unused]] int stackTraceSkipCount = 0) noexcept
        : std::runtime_error(what)
    {
#ifdef _DEBUG
        fillStackTrace(stackTraceSkipCount);
#endif
    }

    /**
     * Initializes object of class SiodbException
     * @param what Explanatory message.
     * @param stackTraceSkipCount How many frames to skip in the beginning of a stack trace.
     */
    explicit SiodbException(
            const std::string& what, [[maybe_unused]] int stackTraceSkipCount = 0) noexcept
        : runtime_error(what)
    {
#ifdef _DEBUG
        fillStackTrace(stackTraceSkipCount);
#endif
    }

    /**
     * Return stack trace, if available.
     * @return Stack trace vector.
     */
    const std::vector<void*>& getStackTrace() const noexcept
    {
#ifdef _DEBUG
        return m_stackTrace;
#else
        return s_emptyStackTrace;
#endif
    }

    /**
     * Return backtrace as string.
     * @return Backtrace vector.
     */
    std::string getStackTraceAsString();

private:
#ifdef _DEBUG
    /**
     * Fills stack backtrace.
     * @param skipCount Number of stack frames to skip.
     */
    void fillStackTrace(int skipCount) noexcept;
#endif

private:
#ifdef _DEBUG
    /** Stack trace vector */
    std::vector<void*> m_stackTrace;

    /** Cached stack trace as string */
    std::string m_cachedStackTraceAsString;
#endif

    /** Fallback empty stack trace vector. */
    static const std::vector<void*> s_emptyStackTrace;
};

}  // namespace siodb
