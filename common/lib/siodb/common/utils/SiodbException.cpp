// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SiodbException.h"

// CRT headers
#include <cstdio>
#include <cstdlib>
#include <cstring>

// STL headers
#include <sstream>

// System headers
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>

namespace siodb {

const std::vector<void*> SiodbException::m_emptyStackTrace;

std::string SiodbException::getStackTraceAsString()
{
#ifdef _DEBUG
    if (m_stackTrace.empty()) return std::string();
    if (m_cachedStackTraceAsString.empty()) {
        char** const symbols = ::backtrace_symbols(m_stackTrace.data(), m_stackTrace.size());
        if (!symbols) return std::string();
        try {
            std::vector<char> buffer(2048);
            std::ostringstream oss;
            // Based on the following public code:
            // https://gist.github.com/fmela/591333/c64f4eb86037bb237862a8283df70cdfc25f01d3
            for (std::size_t i = 0; i < m_stackTrace.size(); ++i) {
                Dl_info info;
                if (::dladdr(m_stackTrace[i], &info)) {
                    int status = 0;
                    char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, 0, &status);
                    ::snprintf(buffer.data(), buffer.size(), "%.3zu  %p %s + %zd\n", i,
                            m_stackTrace[i], (demangled ? demangled : info.dli_sname),
                            static_cast<std::uint8_t*>(m_stackTrace[i])
                                    - static_cast<std::uint8_t*>(info.dli_saddr));
                    ::free(demangled);
                    oss << buffer.data();
                } else {
                    oss << symbols[i] << '\n';
                }
                oss << '\n';
            }
            m_cachedStackTraceAsString = oss.str();
        } catch (...) {
            ::free(symbols);
            return std::string();
        }
        ::free(symbols);
    }
    return m_cachedStackTraceAsString;
#else
    return std::string();
#endif
}

#ifdef _DEBUG
void SiodbException::fillStackTrace(int skipCount) noexcept
{
    try {
        std::vector<void*> frames(kMaxStackTraceFrameCount);
        const int size = ::backtrace(frames.data(), kMaxStackTraceFrameCount);
        std::vector<void*> v;
        if (size > skipCount) {
            v.resize(size - skipCount);
            std::memcpy(v.data(), &frames[skipCount], v.size() * sizeof(void*));
        }
        m_stackTrace.swap(v);
    } catch (...) {
        m_stackTrace.clear();
    }
}
#endif

}  // namespace siodb
