// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserRecord.h"

// Project headers
#include "Helpers.h"
#include "../User.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

namespace siodb::iomgr::dbengine {

UserRecord::UserRecord(const User& user)
    : m_id(user.getId())
    , m_name(user.getName())
    , m_realName(user.getRealName())
    , m_active(user.isActive())
{
    for (const auto& accessKey : user.getAccessKeys())
        m_accessKeys.emplace(*accessKey);
}

std::size_t UserRecord::getSerializedSize() const noexcept
{
    std::size_t result = ::getVarIntSize(m_id) + ::getSerializedSize(m_name)
                         + ::getSerializedSize(m_realName) + 1;
    for (const auto& accessKey : m_accessKeys.byId())
        result += accessKey.getSerializedSize();
    return result;
}

std::uint8_t* UserRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::serializeUnchecked(m_realName, buffer);
    *buffer++ = m_active ? 1 : 0;
    for (const auto& accessKey : m_accessKeys.byId())
        buffer = accessKey.serializeUnchecked(buffer);
    return buffer;
}

std::size_t UserRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    std::size_t totalConsumed = consumed;

    const char* field;
    try {
        field = "name";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);

        field = "realName";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_realName);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, field, ex.what());
    }

    if (length - totalConsumed < 1)
        helpers::reportDeserializationFailure(kClassName, "active", consumed);
    m_active = static_cast<bool>(buffer[totalConsumed++]);

    std::size_t accessKeyCount = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, accessKeyCount);
    if (consumed < 1)
        helpers::reportDeserializationFailure(kClassName, "accessKeys.size", accessKeyCount);
    totalConsumed += consumed;

    std::size_t index = 0;
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
