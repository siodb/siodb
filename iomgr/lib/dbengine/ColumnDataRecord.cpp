// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDataRecord.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

namespace siodb::iomgr::dbengine {

std::size_t ColumnDataRecord::getSerializedSize() const noexcept
{
    return m_address.getSerializedSize() + ::getVarIntSize(m_createTimestamp)
           + ::getVarIntSize(m_updateTimestamp);
}

std::uint8_t* ColumnDataRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = m_address.serializeUnchecked(buffer);
    buffer = ::encodeVarInt(m_createTimestamp, buffer);
    return ::encodeVarInt(m_updateTimestamp, buffer);
}

std::size_t ColumnDataRecord::deserialize(const std::uint8_t* buffer, std::size_t dataSize) noexcept
{
    std::size_t totalConsumed = m_address.deserialize(buffer, dataSize);
    if (totalConsumed == 0) return 0;

    int consumed =
            ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_createTimestamp);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_updateTimestamp);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
