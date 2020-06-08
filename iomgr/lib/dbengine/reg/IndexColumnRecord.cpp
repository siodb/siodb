// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IndexColumnRecord.h"

// Project headers
#include "Helpers.h"
#include "../IndexColumn.h"

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid IndexColumnRecord::s_classUuid =
        boost::lexical_cast<Uuid>("b5bdd7f5-0e28-42d1-9bd9-a1eca39079d5");

IndexColumnRecord::IndexColumnRecord(const IndexColumn& indexColumn) noexcept
    : m_id(indexColumn.getId())
    , m_indexId(indexColumn.getIndexId())
    , m_columnDefinitionId(indexColumn.getColumnDefinitionId())
    , m_sortDescending(indexColumn.isDescendingSortOrder())
{
}

std::size_t IndexColumnRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getVarIntSize(m_indexId) + ::getVarIntSize(m_columnDefinitionId) + 1U;
}

std::uint8_t* IndexColumnRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const
        noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_indexId, buffer);
    buffer = ::encodeVarInt(m_columnDefinitionId, buffer);
    *buffer++ = m_sortDescending ? 1 : 0;
    return buffer;
}

std::size_t IndexColumnRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_indexId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "indexId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_columnDefinitionId);
    if (consumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "columnDefinitionId", consumed);
    totalConsumed += consumed;

    if (length - 1 < totalConsumed)
        helpers::reportInvalidOrNotEnoughData(kClassName, "sortDescending", consumed);
    m_sortDescending = static_cast<bool>(buffer[totalConsumed++]);

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
