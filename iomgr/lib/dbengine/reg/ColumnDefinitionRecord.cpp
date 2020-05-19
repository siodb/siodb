// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDefinitionRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnDefinitionConstraintList.h"

namespace siodb::iomgr::dbengine {

ColumnDefinitionRecord::ColumnDefinitionRecord(const ColumnDefinition& columnDefinition)
    : m_id(columnDefinition.getId())
    , m_columnId(columnDefinition.getColumnId())
{
    for (const auto& columnDefinitionConstraint :
            columnDefinition.getConstraints().byConstraintId()) {
        m_constraints.emplace(*columnDefinitionConstraint);
    }
}

std::size_t ColumnDefinitionRecord::getSerializedSize() const noexcept
{
    auto result = ::getVarIntSize(m_id) + ::getVarIntSize(m_columnId)
                  + ::getVarIntSize(m_constraints.size());
    for (const auto& constraint : m_constraints.byId())
        result += constraint.getSerializedSize();
    return result;
}

std::uint8_t* ColumnDefinitionRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_columnId, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_constraints.size()), buffer);
    for (const auto& constraint : m_constraints.byId())
        buffer = constraint.serializeUnchecked(buffer);
    return buffer;
}

std::size_t ColumnDefinitionRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    std::size_t totalConsumed = 0;

    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnId);
    if (consumed < 1)
        helpers::reportDeserializationFailure(kClassName, "columnDefinitionId", consumed);
    totalConsumed += consumed;

    std::uint32_t constraintCount = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, constraintCount);
    if (consumed < 1)
        helpers::reportDeserializationFailure(kClassName, "constraints.size", consumed);
    totalConsumed += consumed;

    std::uint32_t index = 0;
    try {
        m_constraints.clear();
        for (; index < constraintCount; ++index) {
            ColumnDefinitionConstraintRecord r;
            totalConsumed += r.deserialize(buffer + totalConsumed, length - totalConsumed);
            m_constraints.insert(std::move(r));
        }
    } catch (std::exception& ex) {
        std::ostringstream err;
        err << "constraints[" << index << ']';
        helpers::reportDeserializationFailure(kClassName, err.str().c_str(), ex.what());
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
