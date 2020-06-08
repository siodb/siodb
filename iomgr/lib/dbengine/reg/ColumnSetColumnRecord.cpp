// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnSetColumnRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnSetColumn.h"

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid ColumnSetColumnRecord::s_classUuid =
        boost::lexical_cast<Uuid>("8b96664c-fbbf-49b1-afc7-52ac953efc4c");

ColumnSetColumnRecord::ColumnSetColumnRecord(const ColumnSetColumn& columnSetColumn) noexcept
    : m_id(columnSetColumn.getId())
    , m_columnSetId(columnSetColumn.getColumnSet().getId())
    , m_columnDefinitionId(columnSetColumn.getColumnDefinitionId())
    , m_columnId(columnSetColumn.getColumnId())
{
}

std::size_t ColumnSetColumnRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getVarIntSize(m_columnSetId) + ::getVarIntSize(m_columnDefinitionId)
           + ::getVarIntSize(m_columnId);
}

std::uint8_t* ColumnSetColumnRecord::serializeUnchecked(
        std::uint8_t* buffer, unsigned version) const noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_columnSetId, buffer);
    buffer = ::encodeVarInt(m_columnDefinitionId, buffer);
    buffer = ::encodeVarInt(m_columnId, buffer);
    return buffer;
}

std::size_t ColumnSetColumnRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    if (length < Uuid::static_size())
        helpers::reportInvalidOrNotEnoughData(kClassName, "$classUuid", 0);
    if (std::memcmp(s_classUuid.data, buffer, Uuid::static_size()) != 0)
        helpers::reportClassUuidMismatch(kClassName, buffer, s_classUuid.data);

    std::size_t totalConsumed = Uuid::static_size();

    std::uint32_t classVersion = 0;
    int consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, classVersion);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "$classVersion", consumed);
    totalConsumed += consumed;

    if (classVersion > kClassVersion)
        helpers::reportClassVersionMismatch(kClassName, classVersion, kClassVersion);

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_id);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "id", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnSetId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "columnSetId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnDefinitionId);
    if (consumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "columnDefinitionId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "columnId", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
