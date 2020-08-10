// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserTokenRecord.h"

// Project headers
#include "Helpers.h"
#include "../UserToken.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid UserTokenRecord::s_classUuid =
        boost::lexical_cast<Uuid>("d11de371-260e-4a36-971c-5cab1cbca3b7");

UserTokenRecord::UserTokenRecord(const UserToken& token)
    : m_id(token.getId())
    , m_userId(token.getUserId())
    , m_name(token.getName())
    , m_value(token.getValue())
    , m_expirationTimestamp(token.getExpirationTimestamp())
    , m_description(token.getDescription())
{
}

std::size_t UserTokenRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getVarIntSize(m_userId) + ::getSerializedSize(m_name) + ::getSerializedSize(m_value)
           + ::getSerializedSize(m_expirationTimestamp) + ::getSerializedSize(m_description);
}

std::uint8_t* UserTokenRecord::serializeUnchecked(
        std::uint8_t* buffer, unsigned version) const noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_userId, buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::serializeUnchecked(m_value, buffer);
    buffer = ::serializeUnchecked(m_expirationTimestamp, buffer);
    buffer = ::serializeUnchecked(m_description, buffer);
    return buffer;
}

std::size_t UserTokenRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_userId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "userId", consumed);
    totalConsumed += consumed;

    const char* field;
    try {
        field = "name";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);

        field = "value";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_value);

        field = "expirationTimestamp";
        totalConsumed += ::deserializeObject(
                buffer + totalConsumed, length - totalConsumed, m_expirationTimestamp);

        field = "description";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_description);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, field, ex.what());
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
