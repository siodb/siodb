// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnSetRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnSet.h"

namespace siodb::iomgr::dbengine {

ColumnSetRecord::ColumnSetRecord(const ColumnSet& columnSet)
    : m_id(columnSet.getId())
    , m_tableId(columnSet.getTableId())
{
    for (const auto& columnSetColumn : columnSet.getColumns())
        m_columns.emplace(*columnSetColumn);
}

std::size_t ColumnSetRecord::getSerializedSize() const noexcept
{
    std::size_t result =
            ::getVarIntSize(m_id) + ::getVarIntSize(m_tableId) + getVarIntSize(m_columns.size());
    for (const auto& column : m_columns.byColumnDefinitionId())
        result += column.getSerializedSize();
    return result;
}

std::uint8_t* ColumnSetRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_tableId, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_columns.size()), buffer);
    for (const auto& column : m_columns.byColumnDefinitionId())
        buffer = column.serializeUnchecked(buffer);
    return buffer;
}

std::size_t ColumnSetRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    std::size_t totalConsumed = 0;

    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_tableId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "tableId", consumed);
    totalConsumed += consumed;

    std::uint32_t columnCount = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, columnCount);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "columns.size", consumed);
    totalConsumed += consumed;

    std::uint32_t index = 0;
    try {
        m_columns.clear();
        for (; index < columnCount; ++index) {
            ColumnSetColumnRecord r;
            totalConsumed += r.deserialize(buffer + totalConsumed, length - totalConsumed);
            m_columns.insert(std::move(r));
        }
    } catch (std::exception& ex) {
        std::ostringstream err;
        err << "columns[" << index << ']';
        helpers::reportDeserializationFailure(kClassName, err.str().c_str(), ex.what());
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
