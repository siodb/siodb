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

/** Cipher key information */
struct CipherKeyRecord {
    /** Initializes object of class CipherKeyRecord */
    CipherKeyRecord() noexcept
        : m_id(0)
    {
    }

    /** 
     * Initializes object of class CipherKeyRecord.
     * @param id Record identifier.
     * @param cipherId Cipher identifier.
     * @param key Cipher key.
     */
    CipherKeyRecord(std::uint64_t id, std::string&& cipherId, BinaryValue&& key) noexcept
        : m_id(id)
        , m_cipherId(std::move(cipherId))
        , m_key(std::move(key))
    {
    }

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const CipherKeyRecord& other) const noexcept
    {
        return m_id == other.m_id && m_cipherId == other.m_cipherId && m_key == other.m_key;
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

    /** Record identifier */
    std::uint64_t m_id;

    /** Cipher ID. */
    std::string m_cipherId;

    /** Cipher key */
    BinaryValue m_key;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "CipherKeyRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
