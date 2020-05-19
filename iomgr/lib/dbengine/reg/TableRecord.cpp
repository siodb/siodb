// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TableRecord.h"

// Project headers
#include "Helpers.h"
#include "../Table.h"

namespace siodb::iomgr::dbengine {

TableRecord::TableRecord(const Table& table)
    : m_id(table.getId())
    , m_type(table.getType())
    , m_name(table.getName())
    , m_firstUserTrid(table.getFirstUserTrid())
    , m_currentColumnSetId(table.getCurrentColumnSetId())
{
}

std::size_t TableRecord::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_id) + getVarIntSize(static_cast<std::uint32_t>(m_type))
           + ::getSerializedSize(m_name) + ::getVarIntSize(m_firstUserTrid)
           + ::getVarIntSize(m_currentColumnSetId);
}

std::uint8_t* TableRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_type), buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::encodeVarInt(m_firstUserTrid, buffer);
    buffer = ::encodeVarInt(m_currentColumnSetId, buffer);
    return buffer;
}

std::size_t TableRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    std::size_t totalConsumed = consumed;

    std::uint32_t uv32 = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, uv32);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "type", consumed);
    totalConsumed += consumed;
    m_type = static_cast<TableType>(uv32);

    try {
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, "name", ex.what());
    }

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_firstUserTrid);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "firstUserTrid", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_currentColumnSetId);
    if (consumed < 1)
        helpers::reportDeserializationFailure(kClassName, "currentColumnSetId", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
