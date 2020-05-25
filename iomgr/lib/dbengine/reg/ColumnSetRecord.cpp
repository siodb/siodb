// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnSetRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnSet.h"

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid ColumnSetRecord::kClassUuid =
        boost::lexical_cast<Uuid>("1b61e9c0-ed46-4294-a1ec-c555ec00d0f0");

ColumnSetRecord::ColumnSetRecord(const ColumnSet& columnSet)
    : m_id(columnSet.getId())
    , m_tableId(columnSet.getTableId())
{
    for (const auto& columnSetColumn : columnSet.getColumns())
        m_columns.emplace(*columnSetColumn);
}

std::size_t ColumnSetRecord::getSerializedSize(unsigned version) const noexcept
{
    std::size_t result = Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
                         + ::getVarIntSize(m_tableId)
                         + ::getVarIntSize(static_cast<std::uint32_t>(m_columns.size()));
    for (const auto& column : m_columns.byColumnDefinitionId())
        result += column.getSerializedSize();
    return result;
}

std::uint8_t* ColumnSetRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const
        noexcept
{
    std::memcpy(buffer, kClassUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_tableId, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_columns.size()), buffer);
    for (const auto& column : m_columns.byColumnDefinitionId())
        buffer = column.serializeUnchecked(buffer);
    return buffer;
}

std::size_t ColumnSetRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    if (length < Uuid::static_size())
        helpers::reportInvalidOrNotEnoughData(kClassName, "$classUuid", 0);
    if (std::memcmp(kClassUuid.data, buffer, Uuid::static_size()) != 0)
        helpers::reportClassUuidMismatch(kClassName, buffer, kClassUuid.data);

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

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_tableId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "tableId", consumed);
    totalConsumed += consumed;

    std::uint32_t columnCount = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, columnCount);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "columns.size", consumed);
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
