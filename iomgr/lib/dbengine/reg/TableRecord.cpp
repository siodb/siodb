// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TableRecord.h"

// Project headers
#include "Helpers.h"
#include "../Table.h"

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid TableRecord::kClassUuid =
        boost::lexical_cast<Uuid>("be67ce29-0485-4d3c-885d-fd2fe799eb1b");

TableRecord::TableRecord(const Table& table)
    : m_id(table.getId())
    , m_type(table.getType())
    , m_name(table.getName())
    , m_firstUserTrid(table.getFirstUserTrid())
    , m_currentColumnSetId(table.getCurrentColumnSetId())
    , m_description(table.getDescription())
{
}

std::size_t TableRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getVarIntSize(static_cast<std::uint32_t>(m_type)) + ::getSerializedSize(m_name)
           + ::getVarIntSize(m_firstUserTrid) + ::getVarIntSize(m_currentColumnSetId)
           + ::getSerializedSize(m_description);
}

std::uint8_t* TableRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const noexcept
{
    std::memcpy(buffer, kClassUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_type), buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::encodeVarInt(m_firstUserTrid, buffer);
    buffer = ::encodeVarInt(m_currentColumnSetId, buffer);
    buffer = ::serializeUnchecked(m_description, buffer);
    return buffer;
}

std::size_t TableRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    std::uint32_t type = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, type);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "type", consumed);
    totalConsumed += consumed;
    m_type = static_cast<TableType>(type);

    try {
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, "name", ex.what());
    }

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_firstUserTrid);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "firstUserTrid", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_currentColumnSetId);
    if (consumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "currentColumnSetId", consumed);
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
