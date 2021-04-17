// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ColumnDataAddress.h"

namespace siodb::iomgr::dbengine {

/** Column data record structure */
class ColumnDataRecord {
public:
    /** Maximum serialized size */
    static constexpr std::size_t kMaxSerializedSize = ColumnDataAddress::kMaxSerializedSize + 18;

public:
    /** Initializes new object of class ColumnDataAddress */
    constexpr ColumnDataRecord() noexcept
        : m_createTimestamp(0)
        , m_updateTimestamp(0)
    {
    }

    /**
     * Initializes new object of class ColumnDataAddress.
     * @param address Data address.
     * @param createTimestamp Record creation timestamp.
     * @param updateTimestamp Record update timestamp.
     */
    constexpr ColumnDataRecord(const ColumnDataAddress& address, std::uint64_t createTimestamp,
            std::uint64_t updateTimestamp) noexcept
        : m_address(address)
        , m_createTimestamp(createTimestamp)
        , m_updateTimestamp(updateTimestamp)
    {
    }

    /**
     * Returns data address.
     * @return Data address.
     */
    const auto& getAddress() const noexcept
    {
        return m_address;
    }

    /**
     * Sets data address.
     * @param address Data address.
     */
    void setAddress(const ColumnDataAddress& address) noexcept
    {
        m_address = address;
    }

    /**
     * Returns create timstamp.
     * @return Create timestamp.
     */
    std::uint64_t getCreateTimestamp() const noexcept
    {
        return m_createTimestamp;
    }

    /**
     * Returns update timstamp.
     * @return Update timestamp.
     */
    std::uint64_t getUpdateTimestamp() const noexcept
    {
        return m_updateTimestamp;
    }

    /**
     * Sets update timstamp.
     * @param updateTimestamp Update timestamp.
     */
    void setUpdateTimestamp(std::uint64_t updateTimestamp) noexcept
    {
        m_updateTimestamp = updateTimestamp;
    }

    /**
     * Returns indication that column data is null value.
     * @return true if column data is null value, false otherwise.
     */
    bool isNullValue() const noexcept
    {
        return m_address.isNullValueAddress();
    }

    /**
     * Returns actual serialized size.
     * @return Actual serialized size.
     */
    std::size_t getSerializedSize() const noexcept;

    /**
     * Serializes this object into a memory buffer  using variable length encoding.
     * Doesn't check buffer size.
     * @param buffer Buffer address.
     * @return Address after the last written byte.
     *         NOTE: if buffer is too small, this function may crash.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer) const noexcept;

    /**
     * De-serializes object from a memory buffer using variable length encoding.
     * @param buffer Buffer address.
     * @param dataSize Available data size in buffer.
     * @return Number of bytes consumed or zero if data can't be read.
     */
    std::size_t deserialize(const std::uint8_t* buffer, std::size_t dataSize) noexcept;

private:
    /** Column data address */
    ColumnDataAddress m_address;

    /** Create timestamp */
    std::uint64_t m_createTimestamp;

    /** Update time */
    std::uint64_t m_updateTimestamp;
};

}  // namespace siodb::iomgr::dbengine
