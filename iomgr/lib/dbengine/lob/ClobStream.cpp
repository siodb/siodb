// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ClobStream.h"

// Common project headers
#include <siodb/common/utils/BinaryValue.h>

// STL headers
#include <algorithm>
#include <stdexcept>

namespace siodb::iomgr::dbengine {

namespace {
constexpr const char* kClobReadError = "CLOB read error";
}  // namespace

ClobStream* ClobStream::clone() const
{
    return nullptr;
}

std::string ClobStream::readAsString(std::uint32_t length)
{
    auto available = std::min(length, getRemainingSize());
    if (available == 0) return std::string();
    stdext::buffer<std::uint8_t> buffer(available + 1);
    std::uint64_t pos = 0;
    while (pos < available) {
        const auto n = read(buffer.data() + pos, available - pos);
        if (n < 0) throw std::runtime_error(kClobReadError);
        if (n == 0) break;
        pos += n;
    }
    return std::string(reinterpret_cast<const char*>(buffer.data()));
}

}  // namespace siodb::iomgr::dbengine
