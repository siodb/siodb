// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConstraintRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnConstraint.h"

namespace siodb::iomgr::dbengine {

ConstraintRecord::ConstraintRecord(const Constraint& constraint)
    : m_id(constraint.getId())
    , m_name(constraint.getName())
    , m_state(constraint.getState())
    , m_tableId(constraint.getTableId())
    , m_columnId(constraint.getColumn() ? constraint.getColumn()->getId() : 0)
    , m_constraintDefinitionId(constraint.getDefinitionId())
{
}

std::size_t ConstraintRecord::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_id) + ::getSerializedSize(m_name)
           + ::getVarIntSize(static_cast<std::uint32_t>(m_state)) + ::getVarIntSize(m_tableId)
           + ::getVarIntSize(m_columnId) + ::getVarIntSize(m_constraintDefinitionId);
}

std::uint8_t* ConstraintRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_state), buffer);
    buffer = ::encodeVarInt(m_tableId, buffer);
    buffer = ::encodeVarInt(m_columnId, buffer);
    buffer = ::encodeVarInt(m_constraintDefinitionId, buffer);
    return buffer;
}

std::size_t ConstraintRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "state", consumed);
    m_state = static_cast<ConstraintState>(uv32);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_tableId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "tableId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "columnId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(
            buffer + totalConsumed, length - totalConsumed, m_constraintDefinitionId);
    if (consumed < 1)
        helpers::reportDeserializationFailure(kClassName, "constraintDefinitionId", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
