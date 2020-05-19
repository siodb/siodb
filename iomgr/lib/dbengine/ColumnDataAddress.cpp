// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDataAddress.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

std::size_t ColumnDataAddress::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_blockId) + ::getVarIntSize(m_offset);
}

std::uint8_t* ColumnDataAddress::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_blockId, buffer);
    return ::encodeVarInt(m_offset, buffer);
}

std::size_t ColumnDataAddress::deserialize(
        const std::uint8_t* buffer, std::size_t dataSize) noexcept
{
    int consumed = ::decodeVarInt(buffer, dataSize, m_blockId);
    if (consumed < 1) return 0;
    std::size_t totalConsumed = consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_offset);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    return totalConsumed;
}

const std::uint8_t* ColumnDataAddress::pbeDeserialize(
        const std::uint8_t* buffer, std::size_t dataSize) noexcept
{
    if (dataSize < 12) return nullptr;
    buffer = ::pbeDecodeUInt64(buffer, &m_blockId);
    buffer = ::pbeDecodeUInt32(buffer, &m_offset);
    return buffer;
}

}  // namespace siodb::iomgr::dbengine
