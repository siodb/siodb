// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnSetColumnRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnSetColumn.h"

namespace siodb::iomgr::dbengine {

ColumnSetColumnRecord::ColumnSetColumnRecord(const ColumnSetColumn& columnSetColumn) noexcept
    : m_id(columnSetColumn.getId())
    , m_columnSetId(columnSetColumn.getColumnSet().getId())
    , m_columnDefinitionId(columnSetColumn.getColumnDefinitionId())
    , m_columnId(columnSetColumn.getColumnId())
{
}

std::size_t ColumnSetColumnRecord::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_id) + ::getVarIntSize(m_columnSetId)
           + ::getVarIntSize(m_columnDefinitionId) + ::getVarIntSize(m_columnId);
}

std::uint8_t* ColumnSetColumnRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_columnSetId, buffer);
    buffer = ::encodeVarInt(m_columnDefinitionId, buffer);
    buffer = ::encodeVarInt(m_columnId, buffer);
    return buffer;
}

std::size_t ColumnSetColumnRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    std::size_t totalConsumed = 0;

    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnSetId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "columnSetId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnDefinitionId);
    if (consumed < 1)
        helpers::reportDeserializationFailure(kClassName, "columnDefinitionId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "columnId", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
