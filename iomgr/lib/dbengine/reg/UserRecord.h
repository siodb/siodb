// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "UserAccessKeyRegistry.h"

namespace siodb::iomgr::dbengine {

class User;

/** User reqistry record */
struct UserRecord {
    /** Initializes object of class UserRecord */
    UserRecord()
        : m_id(0)
        , m_active(false)
    {
    }

    /** 
     * Initializes object of class UserRecord.
     * @param id User ID.
     * @param name User name.
     * @param realName Real name.
     * @param description User description.
     * @param active User state.
     * @param accessKeys Access keys.
     */
    UserRecord(std::uint32_t id, std::string&& name, std::optional<std::string>&& realName,
            std::optional<std::string>&& description, bool active,
            UserAccessKeyRegistry&& accessKeys)
        : m_id(id)
        , m_name(std::move(name))
        , m_realName(std::move(realName))
        , m_description(std::move(description))
        , m_active(active)
        , m_accessKeys(std::move(accessKeys))
    {
    }

    /**
     * Initializes object of class UserRecord.
     * @param user User object.
     */
    explicit UserRecord(const User& user);

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const UserRecord& other) const noexcept
    {
        return m_id == other.m_id && m_name == other.m_name && m_realName == other.m_realName
               && m_description == other.m_description && m_active == other.m_active
               && m_accessKeys == other.m_accessKeys;
    }

    /**
     * Returns buffer size required to serialize this object.
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

    /** User ID */
    std::uint32_t m_id;

    /** User name */
    std::string m_name;

    /** Real name */
    std::optional<std::string> m_realName;

    /** Access key description */
    std::optional<std::string> m_description;

    /** User state */
    bool m_active;

    /** User access keys */
    UserAccessKeyRegistry m_accessKeys;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "UserRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
