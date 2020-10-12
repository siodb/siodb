// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DebugUtils.h"

// Common project headers
#include "../stl_ext/buffer.h"

// CRT headers
#include <cstdint>

// STL headers
#include <algorithm>
#include <sstream>

// Boost headers
#include <boost/algorithm/hex.hpp>

namespace siodb::utils {

std::string dumpMemoryToString(const void* addr, std::size_t length, std::size_t stride)
{
    std::ostringstream oss;
    dumpMemoryToStream(addr, length, stride, oss);
    return oss.str();
}

void dumpMemoryToStream(const void* addr, std::size_t length, std::size_t stride, std::ostream& os)
{
    if (stride < 1 || stride > 256) throw std::invalid_argument("Invalid stride");
    os << "MEMORY DUMP: addr " << addr << ", length " << length << '\n';
    if (length > 0) {
        stdext::buffer<char> hexBuffer(stride * 2 + 1);
        while (length > 0) {
            const auto dumpLineLength = std::min(length, stride);
            const auto endAddr = static_cast<const std::uint8_t*>(addr) + dumpLineLength;
            boost::algorithm::hex_lower(
                    static_cast<const std::uint8_t*>(addr), endAddr, hexBuffer.data());
            hexBuffer[2 * dumpLineLength] = 0;
            os << addr << "  " << hexBuffer.data() << '\n';
            addr = endAddr;
            length -= dumpLineLength;
        }
    }
    os << '\n';
}

namespace {

std::atomic<std::uint64_t> g_debugCounter(0);

}  // namespace

void debugPlaceholder() noexcept
{
    // Do something reasonable
    ++g_debugCounter;
}

std::uint64_t getDebugCounter() noexcept
{
    return g_debugCounter.load();
}

}  //  namespace siodb::utils
