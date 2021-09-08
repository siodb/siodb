// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
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

const Uuid UserRecord::s_classUuid =
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

    for (const auto& token : user.getTokens())
        m_tokens.emplace(*token);

    for (const auto& e : user.getGrantedPermissions())
        m_permissions.emplace(m_id, e.first, e.second);
}

std::size_t UserRecord::getSerializedSize(unsigned version) const noexcept
{
    std::size_t result = Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
                         + ::getSerializedSize(m_name) + ::getSerializedSize(m_realName)
                         + ::getSerializedSize(m_description) + 1u
                         + ::getVarIntSize(static_cast<std::uint32_t>(m_accessKeys.size()));
    for (const auto& accessKey : m_accessKeys.byId())
        result += accessKey.getSerializedSize();
    if (version >= 1) {
        result += ::getVarIntSize(static_cast<std::uint32_t>(m_tokens.size()));
        for (const auto& token : m_tokens.byId())
            result += token.getSerializedSize();
    }
    if (version >= 2) {
        result += ::getVarIntSize(static_cast<std::uint32_t>(m_permissions.size()));
        for (const auto& permission : m_permissions.byId())
            result += permission.getSerializedSize();
    }
    return result;
}

std::uint8_t* UserRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
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
    if (version >= 1) {
        buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_tokens.size()), buffer);
        for (const auto& token : m_tokens.byId())
            buffer = token.serializeUnchecked(buffer);
    }
    if (version >= 2) {
        buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_permissions.size()), buffer);
        for (const auto& permission : m_permissions.byId())
            buffer = permission.serializeUnchecked(buffer);
    }
    return buffer;
}

std::size_t UserRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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

    m_accessKeys.clear();
    std::uint32_t index = 0;
    try {
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

    if (classVersion >= 1) {
        std::uint32_t tokenCount = 0;
        consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, tokenCount);
        if (consumed < 1)
            helpers::reportInvalidOrNotEnoughData(kClassName, "tokens.size", tokenCount);
        totalConsumed += consumed;

        index = 0;
        m_tokens.clear();
        try {
            for (; index < tokenCount; ++index) {
                UserTokenRecord r;
                totalConsumed += r.deserialize(buffer + totalConsumed, length - totalConsumed);
                m_tokens.insert(std::move(r));
            }
        } catch (std::exception& ex) {
            std::ostringstream err;
            err << "tokens[" << index << ']';
            helpers::reportDeserializationFailure(kClassName, err.str().c_str(), ex.what());
        }
    }

    if (classVersion >= 2) {
        std::uint32_t permissionCount = 0;
        consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, permissionCount);
        if (consumed < 1)
            helpers::reportInvalidOrNotEnoughData(kClassName, "permissions.size", permissionCount);
        totalConsumed += consumed;

        m_permissions.clear();
        index = 0;
        try {
            for (; index < permissionCount; ++index) {
                UserPermissionRecord r;
                totalConsumed += r.deserialize(buffer + totalConsumed, length - totalConsumed);
                m_permissions.insert(std::move(r));
            }
        } catch (std::exception& ex) {
            std::ostringstream err;
            err << "permissions[" << index << ']';
            helpers::reportDeserializationFailure(kClassName, err.str().c_str(), ex.what());
        }
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
