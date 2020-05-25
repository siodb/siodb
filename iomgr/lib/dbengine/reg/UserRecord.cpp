// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserRecord.h"

// Project headers
#include "Helpers.h"
#include "../User.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid UserRecord::kClassUuid =
        boost::lexical_cast<Uuid>("4f1950d5-01ea-457f-945e-b704894a70b9");

UserRecord::UserRecord(const User& user)
    : m_id(user.getId())
    , m_name(user.getName())
    , m_realName(user.getRealName())
    , m_description(user.getDescription())
    , m_active(user.isActive())
{
    for (const auto& accessKey : user.getAccessKeys())
        m_accessKeys.emplace(*accessKey);
}

std::size_t UserRecord::getSerializedSize(unsigned version) const noexcept
{
    std::size_t result = Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
                         + ::getSerializedSize(m_name) + ::getSerializedSize(m_realName)
                         + ::getSerializedSize(m_description) + 1u
                         + ::getVarIntSize(static_cast<std::uint32_t>(m_accessKeys.size()));
    for (const auto& accessKey : m_accessKeys.byId())
        result += accessKey.getSerializedSize();
    return result;
}

std::uint8_t* UserRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const noexcept
{
    std::memcpy(buffer, kClassUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::serializeUnchecked(m_realName, buffer);
    buffer = ::serializeUnchecked(m_description, buffer);
    *buffer++ = m_active ? 1 : 0;
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_accessKeys.size()), buffer);
    for (const auto& accessKey : m_accessKeys.byId())
        buffer = accessKey.serializeUnchecked(buffer);
    return buffer;
}

std::size_t UserRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    const char* field;
    try {
        field = "name";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);

        field = "realName";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_realName);

        field = "description";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_description);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, field, ex.what());
    }

    if (length - totalConsumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "active", consumed);
    m_active = static_cast<bool>(buffer[totalConsumed++]);

    std::uint32_t accessKeyCount = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, accessKeyCount);
    if (consumed < 1)
        helpers::reportInvalidOrNotEnoughData(kClassName, "accessKeys.size", accessKeyCount);
    totalConsumed += consumed;

    std::uint32_t index = 0;
    try {
        m_accessKeys.clear();
        for (; index < accessKeyCount; ++index) {
            UserAccessKeyRecord r;
            totalConsumed += r.deserialize(buffer + totalConsumed, length - totalConsumed);
            m_accessKeys.insert(std::move(r));
        }
    } catch (std::exception& ex) {
        std::ostringstream err;
        err << "accessKeys[" << index << ']';
        helpers::reportDeserializationFailure(kClassName, err.str().c_str(), ex.what());
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
