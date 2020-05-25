// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserAccessKeyRecord.h"

// Project headers
#include "Helpers.h"
#include "../UserAccessKey.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid UserAccessKeyRecord::kClassUuid =
        boost::lexical_cast<Uuid>("9e75e8e0-3e32-4a2b-a011-6b689b213c61");

UserAccessKeyRecord::UserAccessKeyRecord(const UserAccessKey& accessKey)
    : m_id(accessKey.getId())
    , m_userId(accessKey.getUserId())
    , m_name(accessKey.getName())
    , m_text(accessKey.getText())
    , m_description(accessKey.getDescription())
    , m_active(accessKey.isActive())
{
}

std::size_t UserAccessKeyRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getVarIntSize(m_userId) + ::getSerializedSize(m_name) + ::getSerializedSize(m_text)
           + ::getSerializedSize(m_description) + 1u;
}

std::uint8_t* UserAccessKeyRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const
        noexcept
{
    std::memcpy(buffer, kClassUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_userId, buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::serializeUnchecked(m_text, buffer);
    buffer = ::serializeUnchecked(m_description, buffer);
    *buffer++ = m_active ? 1 : 0;
    return buffer;
}

std::size_t UserAccessKeyRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_userId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "userId", consumed);
    totalConsumed += consumed;

    const char* field;
    try {
        field = "name";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);

        field = "text";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_text);

        field = "description";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_description);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, field, ex.what());
    }

    if (length - totalConsumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "active", consumed);
    m_active = static_cast<bool>(buffer[totalConsumed++]);

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
