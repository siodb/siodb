// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/BinaryValue.h>
#include <siodb/common/utils/Uuid.h>

// CRT headers
#include <cstdint>
#include <ctime>

// STL headers
#include <optional>
#include <string>

namespace siodb::iomgr::dbengine {

class UserToken;

/** User acess key reqistry record */
struct UserTokenRecord {
    /** Initializes object of class UserTokenRecord */
    UserTokenRecord() noexcept
        : m_id(0)
        , m_userId(0)
    {
    }

    /** 
     * Initializes object of class UserTokenRecord.
     * @param id Token ID.
     * @param userId User ID.
     * @param name Token name.
     * @param value Token value.
     * @param expirationTimestamp Token expiration timestamp.
     * @param description User acess key description.
     */
    UserTokenRecord(std::uint64_t id, std::uint32_t userId, std::string&& name, BinaryValue&& value,
            std::optional<std::int64_t>&& expirationTimestamp,
            std::optional<std::string>&& description) noexcept
        : m_id(id)
        , m_userId(userId)
        , m_name(std::move(name))
        , m_value(std::move(value))
        , m_expirationTimestamp(std::move(expirationTimestamp))
        , m_description(std::move(description))
    {
    }

    /**
     * Initializes object of class UserTokenRecord.
     * @param token Token object.
     */
    explicit UserTokenRecord(const UserToken& token);

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const UserTokenRecord& other) const noexcept
    {
        return m_id == other.m_id && m_userId == other.m_userId && m_name == other.m_name
               && m_value == other.m_value && m_expirationTimestamp == other.m_expirationTimestamp
               && m_description == other.m_description;
    }

    /**
     * Returns buffer size required to serialize this object.
     * @param version Target version.
     * @return Number of bytes.
     */
    std::size_t getSerializedSize(unsigned version = kClassVersion) const noexcept;

    /**
     * Serializes object into buffer. Assumes buffer is big enough.
     * @param buffer Output buffer.
     * @param version Target version.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serializeUnchecked(
            std::uint8_t* buffer, unsigned version = kClassVersion) const noexcept;

    /**
     * Deserializes object from buffer.
     * @param buffer Input buffer.
     * @param length Length of data in the buffer.
     * @return Number of bytes consumed.
     */
    std::size_t deserialize(const std::uint8_t* buffer, std::size_t length);

    /** Token ID */
    std::uint64_t m_id;

    /** User ID */
    std::uint32_t m_userId;

    /** Token name */
    std::string m_name;

    /** Token value */
    BinaryValue m_value;

    /** Token expiration timestamp */
    std::optional<std::int64_t> m_expirationTimestamp;

    /** Token description */
    std::optional<std::string> m_description;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "UserTokenRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
