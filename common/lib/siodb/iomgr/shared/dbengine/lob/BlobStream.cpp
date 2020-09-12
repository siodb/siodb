// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BlobStream.h"

// STL headers
#include <algorithm>
#include <stdexcept>

namespace siodb::iomgr::dbengine {

namespace {
constexpr const char* kBlobReadError = "BLOB read error";
}  // namespace

BlobStream* BlobStream::clone() const
{
    throw std::logic_error("Can't clone BLOB stream");
}

BinaryValue BlobStream::readAsBinary(std::uint32_t length)
{
    auto available = std::min(length, getRemainingSize());
    if (available == 0) return BinaryValue();
    BinaryValue bv(available);
    auto data = bv.data();
    std::uint64_t pos = 0;
    while (pos < available) {
        const auto n = read(data + pos, available - pos);
        if (n < 0) throw std::runtime_error(kBlobReadError);
        if (n == 0) break;
        pos += n;
    }
    return (pos < available) ? BinaryValue(data, data + pos) : bv;
}

}  // namespace siodb::iomgr::dbengine
