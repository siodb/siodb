// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConstraintRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnConstraint.h"

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid ConstraintRecord::s_classUuid =
        boost::lexical_cast<Uuid>("88f04e4b-b6bb-4101-b52f-340aac0053d1");

ConstraintRecord::ConstraintRecord(const Constraint& constraint)
    : m_id(constraint.getId())
    , m_name(constraint.getName())
    , m_state(constraint.getState())
    , m_tableId(constraint.getTableId())
    , m_columnId(constraint.getColumn() ? constraint.getColumn()->getId() : 0)
    , m_constraintDefinitionId(constraint.getDefinitionId())
    , m_description(constraint.getDescription())
{
}

std::size_t ConstraintRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getSerializedSize(m_name) + ::getVarIntSize(static_cast<std::uint32_t>(m_state))
           + ::getVarIntSize(m_tableId) + ::getVarIntSize(m_columnId)
           + ::getVarIntSize(m_constraintDefinitionId) + ::getSerializedSize(m_description);
}

std::uint8_t* ConstraintRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const
        noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_state), buffer);
    buffer = ::encodeVarInt(m_tableId, buffer);
    buffer = ::encodeVarInt(m_columnId, buffer);
    buffer = ::encodeVarInt(m_constraintDefinitionId, buffer);
    buffer = ::serializeUnchecked(m_description, buffer);
    return buffer;
}

std::size_t ConstraintRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    try {
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, "name", ex.what());
    }

    std::uint32_t state = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, state);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "state", consumed);
    m_state = static_cast<ConstraintState>(state);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_tableId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "tableId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "columnId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(
            buffer + totalConsumed, length - totalConsumed, m_constraintDefinitionId);
    if (consumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "constraintDefinitionId", consumed);
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
