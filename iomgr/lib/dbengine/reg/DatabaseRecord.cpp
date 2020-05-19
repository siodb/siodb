// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DatabaseRecord.h"

// Project headers
#include "Helpers.h"
#include "../Database.h"

namespace siodb::iomgr::dbengine {

DatabaseRecord::DatabaseRecord(const Database& database)
    : m_id(database.getId())
    , m_uuid(database.getUuid())
    , m_name(database.getName())
    , m_cipherId(database.getCipherId())
    , m_cipherKey(database.getCipherKey())
{
}

std::size_t DatabaseRecord::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_id) + m_uuid.size() + ::getSerializedSize(m_name)
           + ::getSerializedSize(m_cipherId) + ::getSerializedSize(m_cipherKey);
}

std::uint8_t* DatabaseRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    std::memcpy(buffer, m_uuid.data, m_uuid.size());
    buffer += m_uuid.size();
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::serializeUnchecked(m_cipherId, buffer);
    buffer = ::serializeUnchecked(m_cipherKey, buffer);
    return buffer;
}

std::size_t DatabaseRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    std::size_t totalConsumed = consumed;

    if (length - totalConsumed < m_uuid.size())
        helpers::reportDeserializationFailure(kClassName, "uuid", 0);
    std::memcpy(m_uuid.data, buffer, m_uuid.size());
    totalConsumed += m_uuid.size();

    const char* field;
    try {
        field = "name";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);

        field = "cipherId";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_cipherId);

        field = "cipherKey";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_cipherKey);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, field, ex.what());
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
