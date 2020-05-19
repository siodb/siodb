// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// STL headers
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
     * @param active Indication that access key is active.
     */
    UserAccessKeyRecord(std::uint64_t id, std::uint32_t userId, std::string&& name,
            std::string&& text, bool active)
        : m_id(id)
        , m_userId(userId)
        , m_name(std::move(name))
        , m_text(std::move(text))
        , m_active(active)
    {
    }

    /**
     * Initializes object of class UserAccessKeyRecord.
     * @param accessKey Access key object.
     */
    explicit UserAccessKeyRecord(const UserAccessKey& accessKey);

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

    /** Access key ID */
    std::uint64_t m_id;

    /** User ID */
    std::uint32_t m_userId;

    /** Access key name */
    std::string m_name;

    /** Access key text */
    std::string m_text;

    /** Indication that key is active */
    bool m_active;

    /** Structure name */
    static constexpr const char* kClassName = "UserAccessKeyRecord";
};

}  // namespace siodb::iomgr::dbengine
