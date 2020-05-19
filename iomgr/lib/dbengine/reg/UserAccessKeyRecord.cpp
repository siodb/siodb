// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserAccessKeyRecord.h"

// Project headers
#include "Helpers.h"
#include "../UserAccessKey.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

namespace siodb::iomgr::dbengine {

UserAccessKeyRecord::UserAccessKeyRecord(const UserAccessKey& accessKey)
    : m_id(accessKey.getId())
    , m_userId(accessKey.getUserId())
    , m_name(accessKey.getName())
    , m_text(accessKey.getText())
    , m_active(accessKey.isActive())
{
}

std::size_t UserAccessKeyRecord::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_id) + getVarIntSize(m_userId) + ::getSerializedSize(m_name)
           + ::getSerializedSize(m_text) + 1;
}

std::uint8_t* UserAccessKeyRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_userId, buffer);
    buffer = ::serializeUnchecked(m_name, buffer);
    buffer = ::serializeUnchecked(m_text, buffer);
    *buffer++ = m_active ? 1 : 0;
    return buffer;
}

std::size_t UserAccessKeyRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    std::size_t totalConsumed = consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_userId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "userId", consumed);
    totalConsumed += consumed;

    const char* field;
    try {
        field = "name";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_name);

        field = "text";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_text);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, field, ex.what());
    }

    if (length - totalConsumed < 1)
        helpers::reportDeserializationFailure(kClassName, "active", consumed);
    m_active = static_cast<bool>(buffer[totalConsumed++]);

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
