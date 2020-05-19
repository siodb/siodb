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
     * @param active User state.
     * @param accessKeys Access keys.
     */
    UserRecord(std::uint32_t id, std::string&& name, std::string&& realName, bool active,
            UserAccessKeyRegistry&& accessKeys)
        : m_id(id)
        , m_name(std::move(name))
        , m_realName(std::move(realName))
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
     * Returns buffer size required to serialize this object.
     * @return Number of bytes.
     */
    std::size_t getSerializedSize() const noexcept;

    /**
     * Serializes object into buffer. Assumes buffer is big enough.
     * @param buffer Output buffer.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer) const noexcept;

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
    std::string m_realName;

    /** User state */
    bool m_active;

    /** User access keys */
    UserAccessKeyRegistry m_accessKeys;

    /** Structure name */
    static constexpr const char* kClassName = "UserRecord";
};

}  // namespace siodb::iomgr::dbengine
