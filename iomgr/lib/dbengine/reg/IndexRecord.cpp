// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IndexRecord.h"

// Project headers
#include "Helpers.h"
#include "../IndexColumn.h"

namespace siodb::iomgr::dbengine {

IndexRecord::IndexRecord(const Index& index)
    : m_id(index.getId())
    , m_type(index.getType())
    , m_tableId(index.getTableId())
    , m_unique(index.isUnique())
    , m_name(index.getName())
    , m_dataFileSize(index.getDataFileSize())
{
    for (const auto& indexColumn : index.getColumns())
        m_columns.emplace(*indexColumn);
}

std::size_t IndexRecord::getSerializedSize() const noexcept
{
    std::size_t result = ::getVarIntSize(m_id) + getVarIntSize(static_cast<std::uint32_t>(m_type))
                         + ::getVarIntSize(m_tableId) + 1 + ::getSerializedSize(m_name)
                         + ::getVarIntSize(m_columns.size()) + ::getVarIntSize(m_dataFileSize);
    for (const auto& column : m_columns.byId())
        result += column.getSerializedSize();
    return result;
}

std::uint8_t* IndexRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_type), buffer);
    buffer = ::encodeVarInt(m_tableId, buffer);
    *buffer++ = m_unique ? 1 : 0;
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_columns.size()), buffer);
    for (const auto& column : m_columns.byId())
        buffer = column.serializeUnchecked(buffer);
    buffer = ::encodeVarInt(m_dataFileSize, buffer);
    return buffer;
}

std::size_t IndexRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    std::size_t totalConsumed = consumed;

    std::uint32_t uv32 = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, uv32);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "type", consumed);
    totalConsumed += consumed;
    m_type = static_cast<IndexType>(uv32);

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_tableId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "tableId", consumed);
    totalConsumed += consumed;

    if (length - 1 < totalConsumed)
        helpers::reportDeserializationFailure(kClassName, "unique", consumed);
    m_unique = static_cast<bool>(buffer[totalConsumed++]);

    try {
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, "name", ex.what());
    }

    std::uint32_t columnCount = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, columnCount);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "column.size", columnCount);
    totalConsumed += consumed;

    std::uint32_t index = 0;
    try {
        m_columns.clear();
        for (; index < columnCount; ++index) {
            IndexColumnRecord r;
            totalConsumed += r.deserialize(buffer + totalConsumed, length - totalConsumed);
            m_columns.insert(std::move(r));
        }
    } catch (std::exception& ex) {
        std::ostringstream err;
        err << "columns[" << index << ']';
        helpers::reportDeserializationFailure(kClassName, err.str().c_str(), ex.what());
    }

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_dataFileSize);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "dataFileSize", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
