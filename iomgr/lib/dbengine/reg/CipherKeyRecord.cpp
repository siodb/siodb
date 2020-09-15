// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "CipherKeyRecord.h"

// Project headers
#include "Helpers.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid CipherKeyRecord::s_classUuid =
        boost::lexical_cast<Uuid>("dffb2d5a-a781-428b-bdb5-54633e3ab8dd");

std::size_t CipherKeyRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getSerializedSize(m_cipherId) + ::getSerializedSize(m_key);
}

std::uint8_t* CipherKeyRecord::serializeUnchecked(
        std::uint8_t* buffer, unsigned version) const noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::serializeUnchecked(m_cipherId, buffer);
    buffer = ::serializeUnchecked(m_key, buffer);
    return buffer;
}

std::size_t CipherKeyRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
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
        field = "cipherId";
        totalConsumed +=
                ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_cipherId);

        field = "key";
        totalConsumed += ::deserializeObject(buffer + totalConsumed, length - totalConsumed, m_key);
    } catch (std::exception& ex) {
        helpers::reportDeserializationFailure(kClassName, field, ex.what());
    }

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
