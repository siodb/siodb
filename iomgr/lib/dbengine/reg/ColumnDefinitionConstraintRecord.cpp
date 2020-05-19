// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDefinitionConstraintRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnDefinitionConstraint.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

namespace siodb::iomgr::dbengine {

ColumnDefinitionConstraintRecord::ColumnDefinitionConstraintRecord(
        const ColumnDefinitionConstraint& columnDefinitionConstraint) noexcept
    : m_id(columnDefinitionConstraint.getId())
    , m_columnDefinitionId(columnDefinitionConstraint.getColumnDefinition().getId())
    , m_constraintId(columnDefinitionConstraint.getConstraint().getId())
{
}

std::size_t ColumnDefinitionConstraintRecord::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_id) + ::getVarIntSize(m_columnDefinitionId)
           + ::getVarIntSize(m_constraintId);
}

std::uint8_t* ColumnDefinitionConstraintRecord::serializeUnchecked(std::uint8_t* buffer) const
        noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_columnDefinitionId, buffer);
    buffer = ::encodeVarInt(m_constraintId, buffer);
    return buffer;
}

std::size_t ColumnDefinitionConstraintRecord::deserialize(
        const std::uint8_t* buffer, std::size_t length)
{
    std::size_t totalConsumed = 0;

    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnDefinitionId);
    if (consumed < 1)
        helpers::reportDeserializationFailure(kClassName, "columnDefinitionId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_constraintId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "constraintId", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
