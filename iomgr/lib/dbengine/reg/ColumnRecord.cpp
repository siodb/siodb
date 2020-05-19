// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnRecord.h"

// Project headers
#include "Helpers.h"
#include "../Column.h"

namespace siodb::iomgr::dbengine {

ColumnRecord::ColumnRecord(const Column& column)
    : m_id(column.getId())
    , m_name(column.getName())
    , m_dataType(column.getDataType())
    , m_tableId(column.getTableId())
    , m_state(column.getState())
    , m_dataBlockDataAreaSize(column.getDataBlockDataAreaSize())
{
}

std::size_t ColumnRecord::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_id) + ::getSerializedSize(m_name)
           + ::getVarIntSize(static_cast<std::uint32_t>(m_dataType)) + ::getVarIntSize(m_tableId)
           + ::getVarIntSize(static_cast<std::uint32_t>(m_state))
           + ::getVarIntSize(m_dataBlockDataAreaSize);
}

std::uint8_t* ColumnRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_dataType), buffer);
    buffer = ::encodeVarInt(m_tableId, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_state), buffer);
    buffer = ::encodeVarInt(m_dataBlockDataAreaSize, buffer);
    return buffer;
}

std::size_t ColumnRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    std::size_t totalConsumed = consumed;

    try {
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, "name", ex.what());
    }

    std::uint32_t uv32 = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, uv32);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "dataType", consumed);
    m_dataType = static_cast<ColumnDataType>(uv32);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_tableId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "tableId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, uv32);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "state", consumed);
    m_state = static_cast<ColumnState>(uv32);
    totalConsumed += consumed;

    consumed =
            ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_dataBlockDataAreaSize);
    if (consumed < 1)
        helpers::reportDeserializationFailure(kClassName, "dataBlockDataAreaSize", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
