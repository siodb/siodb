// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>

// CRT headers
#include <cstdint>

// STL headers
#include <optional>
#include <string>

namespace siodb::iomgr::dbengine {

class UserAccessKey;

/** User acess key reqistry record */
struct UserAccessKeyRecord {
    /** Initializes object of class UserAccessKeyRecord */
    UserAccessKeyRecord()
        : m_id(0)
        , m_userId(0)
        , m_active(false)
    {
    }

    /** 
     * Initializes object of class UserAccessKeyRecord.
     * @param id Access key ID.
     * @param userId User ID.
     * @param name Access key name.
     * @param text Access key text.
     * @param description User acess key description.
     * @param active Indication that access key is active.
     */
    UserAccessKeyRecord(std::uint64_t id, std::uint32_t userId, std::string&& name,
            std::string&& text, std::optional<std::string>&& description, bool active)
        : m_id(id)
        , m_userId(userId)
        , m_name(std::move(name))
        , m_text(std::move(text))
        , m_description(std::move(description))
        , m_active(active)
    {
    }

    /**
     * Initializes object of class UserAccessKeyRecord.
     * @param accessKey Access key object.
     */
    explicit UserAccessKeyRecord(const UserAccessKey& accessKey);

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const UserAccessKeyRecord& other) const noexcept
    {
        return m_id == other.m_id && m_userId == other.m_userId && m_name == other.m_name
               && m_text == other.m_text && m_description == other.m_description
               && m_active == other.m_active;
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
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer, unsigned version = kClassVersion) const
            noexcept;

    /**
     * Deserializes object from buffer.
     * @param buffer Input buffer.
     * @param length Length of data in the buffer.
     * @return Number of bytes consumed.
     */
    std::size_t deserialize(const std::uint8_t* buffer, std::size_t length);

    /** Access key ID */
    std::uint64_t m_id;

    /** User ID */
    std::uint32_t m_userId;

    /** Access key name */
    std::string m_name;

    /** Access key text */
    std::string m_text;

    /** Access key description */
    std::optional<std::string> m_description;

    /** Indication that key is active */
    bool m_active;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "UserAccessKeyRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
