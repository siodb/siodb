// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDefinitionRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnDefinitionConstraintList.h"

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid ColumnDefinitionRecord::s_classUuid =
        boost::lexical_cast<Uuid>("4f3b6d57-cbcd-4df0-8efd-5910c5392ade");

ColumnDefinitionRecord::ColumnDefinitionRecord(const ColumnDefinition& columnDefinition)
    : m_id(columnDefinition.getId())
    , m_columnId(columnDefinition.getColumnId())
{
    for (const auto& columnDefinitionConstraint :
            columnDefinition.getConstraints().byConstraintId()) {
        m_constraints.emplace(*columnDefinitionConstraint);
    }
}

std::size_t ColumnDefinitionRecord::getSerializedSize(unsigned version) const noexcept
{
    auto result = Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
                  + ::getVarIntSize(m_columnId)
                  + ::getVarIntSize(static_cast<std::uint32_t>(m_constraints.size()));
    for (const auto& constraint : m_constraints.byId())
        result += constraint.getSerializedSize();
    return result;
}

std::uint8_t* ColumnDefinitionRecord::serializeUnchecked(
        std::uint8_t* buffer, unsigned version) const noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_columnId, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_constraints.size()), buffer);
    for (const auto& constraint : m_constraints.byId())
        buffer = constraint.serializeUnchecked(buffer);
    return buffer;
}

std::size_t ColumnDefinitionRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnId);
    if (consumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "columnDefinitionId", consumed);
    totalConsumed += consumed;

    std::uint32_t constraintCount = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, constraintCount);
    if (consumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "constraints.size", consumed);
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
