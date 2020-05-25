// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DatabaseRecord.h"

// Project headers
#include "Helpers.h"
#include "../Database.h"

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid DatabaseRecord::kClassUuid =
        boost::lexical_cast<Uuid>("34623147-9211-46dd-a5cc-83f88c001476");

DatabaseRecord::DatabaseRecord(const Database& database)
    : m_id(database.getId())
    , m_uuid(database.getUuid())
    , m_name(database.getName())
    , m_cipherId(database.getCipherId())
    , m_cipherKey(database.getCipherKey())
    , m_description(database.getDescription())
{
}

std::size_t DatabaseRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + Uuid::static_size() + ::getSerializedSize(m_name) + ::getSerializedSize(m_cipherId)
           + ::getSerializedSize(m_cipherKey) + ::getSerializedSize(m_description);
}

std::uint8_t* DatabaseRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const
        noexcept
{
    std::memcpy(buffer, kClassUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    std::memcpy(buffer, m_uuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::serializeUnchecked(m_cipherId, buffer);
    buffer = ::serializeUnchecked(m_cipherKey, buffer);
    buffer = ::serializeUnchecked(m_description, buffer);
    return buffer;
}

std::size_t DatabaseRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    if (length - totalConsumed < Uuid::static_size())
        helpers::reportInvalidOrNotEnoughData(kClassName, "uuid", 0);
    std::memcpy(m_uuid.data, buffer + totalConsumed, Uuid::static_size());
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

        field = "description";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_description);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, field, ex.what());
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
