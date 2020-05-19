// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IndexColumnRecord.h"

// Project headers
#include "Helpers.h"
#include "../IndexColumn.h"

namespace siodb::iomgr::dbengine {

IndexColumnRecord::IndexColumnRecord(const IndexColumn& indexColumn) noexcept
    : m_id(indexColumn.getId())
    , m_indexId(indexColumn.getIndexId())
    , m_columnDefinitionId(indexColumn.getColumnDefinitionId())
    , m_isSortDescending(indexColumn.isDescendingSortOrder())
{
}

std::size_t IndexColumnRecord::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_id) + ::getVarIntSize(m_columnDefinitionId)
           + ::getVarIntSize(m_columnDefinitionId) + 1;
}

std::uint8_t* IndexColumnRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_indexId, buffer);
    buffer = ::encodeVarInt(m_columnDefinitionId, buffer);
    *buffer++ = m_isSortDescending ? 1 : 0;
    return buffer;
}

std::size_t IndexColumnRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    std::size_t totalConsumed = 0;

    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_indexId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "indexId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnDefinitionId);
    if (consumed < 1)
        helpers::reportDeserializationFailure(kClassName, "columnDefinitionId", consumed);
    totalConsumed += consumed;

    if (length - 1 < totalConsumed)
        helpers::reportDeserializationFailure(kClassName, "sortDescending", consumed);
    m_isSortDescending = static_cast<bool>(buffer[totalConsumed++]);

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
