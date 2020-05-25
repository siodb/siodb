// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnRecord.h"

// Project headers
#include "Helpers.h"
#include "../Column.h"

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid ColumnRecord::kClassUuid =
        boost::lexical_cast<Uuid>("fde8ee48-9505-4cfa-bef5-c72254cc123d");

ColumnRecord::ColumnRecord(const Column& column)
    : m_id(column.getId())
    , m_name(column.getName())
    , m_dataType(column.getDataType())
    , m_tableId(column.getTableId())
    , m_state(column.getState())
    , m_dataBlockDataAreaSize(column.getDataBlockDataAreaSize())
    , m_description(column.getDescription())
{
}

std::size_t ColumnRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getSerializedSize(m_name) + ::getVarIntSize(static_cast<std::uint32_t>(m_dataType))
           + ::getVarIntSize(m_tableId) + ::getVarIntSize(static_cast<std::uint32_t>(m_state))
           + ::getVarIntSize(m_dataBlockDataAreaSize) + ::getSerializedSize(m_description);
}

std::uint8_t* ColumnRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const
        noexcept
{
    std::memcpy(buffer, kClassUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_dataType), buffer);
    buffer = ::encodeVarInt(m_tableId, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_state), buffer);
    buffer = ::encodeVarInt(m_dataBlockDataAreaSize, buffer);
    buffer = ::serializeUnchecked(m_description, buffer);
    return buffer;
}

std::size_t ColumnRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    try {
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, "name", ex.what());
    }

    std::uint32_t dataType = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, dataType);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "dataType", consumed);
    m_dataType = static_cast<ColumnDataType>(dataType);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_tableId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "tableId", consumed);
    totalConsumed += consumed;

    std::uint32_t state = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, state);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "state", consumed);
    m_state = static_cast<ColumnState>(state);
    totalConsumed += consumed;

    consumed =
            ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_dataBlockDataAreaSize);
    if (consumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "dataBlockDataAreaSize", consumed);
    totalConsumed += consumed;

    try {
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_description);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, "description", ex.what());
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
