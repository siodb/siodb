// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/BinaryValue.h>
#include <siodb/common/utils/Uuid.h>

// STL headers
#include <string>

namespace siodb::iomgr::dbengine {

class Database;

/** In-memory database registry record */
struct DatabaseRecord {
    /** Initializes object of class DatabaseRecord */
    DatabaseRecord() noexcept
        : m_id(0)
        , m_uuid(utils::getZeroUuid())
    {
    }

    /**
     * Initializes object of class DatabaseRecord.
     * @param id Database ID.
     * @param uuid Database UUID.
     * @param name Database name.
     * @param cipherId Cipher identifier.
     * @param cipherKey Cipher key.
     * @param description Database description.
     */
    DatabaseRecord(std::uint32_t id, const Uuid& uuid, std::string&& name, std::string&& cipherId,
            BinaryValue&& cipherKey, std::optional<std::string>&& description)
        : m_id(id)
        , m_uuid(uuid)
        , m_name(std::move(name))
        , m_cipherId(std::move(cipherId))
        , m_cipherKey(std::move(cipherKey))
        , m_description(std::move(description))
    {
    }

    /**
     * Initializes object of class DatabaseRecord.
     * @param database Database object.
     */
    DatabaseRecord(const Database& database);

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const DatabaseRecord& other) const noexcept
    {
        return m_id == other.m_id && m_uuid == other.m_uuid && m_name == other.m_name
               && m_cipherId == other.m_cipherId && m_cipherKey == other.m_cipherKey
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
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer, unsigned version = kClassVersion) const
            noexcept;

    /**
     * Deserializes object from buffer.
     * @param buffer Input buffer.
     * @param length Length of data in the buffer.
     * @return Number of bytes consumed.
     */
    std::size_t deserialize(const std::uint8_t* buffer, std::size_t length);

    /** Database ID */
    std::uint32_t m_id;

    /** Database UUID */
    Uuid m_uuid;

    /** Database name */
    std::string m_name;

    /** Cipher ID */
    std::string m_cipherId;

    /** Cipher key */
    BinaryValue m_cipherKey;

    /** Database description */
    std::optional<std::string> m_description;

    /** Structure UUID */
    static const Uuid kClassUuid;

    /** Structure name */
    static constexpr const char* kClassName = "DatabaseRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
