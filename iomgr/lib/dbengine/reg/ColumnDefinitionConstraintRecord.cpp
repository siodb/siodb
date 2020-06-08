// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDefinitionConstraintRecord.h"

// Project headers
#include "Helpers.h"
#include "../ColumnDefinitionConstraint.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid ColumnDefinitionConstraintRecord::s_classUuid =
        boost::lexical_cast<Uuid>("0ae9921a-637f-4146-80f3-5215f4b9d325");

ColumnDefinitionConstraintRecord::ColumnDefinitionConstraintRecord(
        const ColumnDefinitionConstraint& columnDefinitionConstraint) noexcept
    : m_id(columnDefinitionConstraint.getId())
    , m_columnDefinitionId(columnDefinitionConstraint.getColumnDefinition().getId())
    , m_constraintId(columnDefinitionConstraint.getConstraint().getId())
{
}

std::size_t ColumnDefinitionConstraintRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getVarIntSize(m_columnDefinitionId) + ::getVarIntSize(m_constraintId);
}

std::uint8_t* ColumnDefinitionConstraintRecord::serializeUnchecked(
        std::uint8_t* buffer, unsigned version) const noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_columnDefinitionId, buffer);
    buffer = ::encodeVarInt(m_constraintId, buffer);
    return buffer;
}

std::size_t ColumnDefinitionConstraintRecord::deserialize(
        const std::uint8_t* buffer, std::size_t length)
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

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnDefinitionId);
    if (consumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "columnDefinitionId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_constraintId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "constraintId", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
