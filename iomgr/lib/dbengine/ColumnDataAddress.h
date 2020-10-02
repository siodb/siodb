// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/ComparisonOperations.h>

// CRT headers
#include <cstdint>
#include <cstring>

// STL Headers
#include <ostream>

namespace siodb::iomgr::dbengine {

/** Column address data structure */
class ColumnDataAddress : public utils::ComparisonOperations<ColumnDataAddress> {
public:
    /** Maximum serialized size */
    static constexpr std::size_t kMaxSerializedSize = 14;

public:
    /** Initializes new object of class ColumnDataAddress */
    constexpr ColumnDataAddress() noexcept
        : m_blockId(0)
        , m_offset(0)
    {
    }

    /**
     * Initializes new object of class ColumnDataAddress.
     * @param blockId Block file ID.
     * @param offset Offset in the block file.
     */
    constexpr ColumnDataAddress(std::uint64_t blockId, std::uint32_t offset) noexcept
        : m_blockId(blockId)
        , m_offset(offset)
    {
    }

    /**
     * Returns block file ID.
     * @return Block file ID.
     */
    std::uint64_t getBlockId() const noexcept
    {
        return m_blockId;
    }

    /**
     * Returns offset in block file.
     * @return Offset in block file.
     */
    std::uint32_t getOffset() const noexcept
    {
        return m_offset;
    }

    /**
     * Returns indication that address is null value address.
     * @return true if address is null value address, false otherwise.
     */
    bool isNullValueAddress() const noexcept
    {
        return m_blockId == 0 && m_offset == 0;
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

    /**
     * De-serializes object from a memory buffer using plain binary encoding.
     * @param buffer Buffer address.
     * @param dataSize Available data size in buffer.
     * @return Address after the last read byte or nullptr if data can't be read
     *         (data size is too small or data corrupted).
     */
    const std::uint8_t* pbeDeserialize(const std::uint8_t* buffer, std::size_t dataSize) noexcept;

private:
    /** Column data block ID */
    std::uint64_t m_blockId;

    /** Offset in the data section of the block file */
    std::uint32_t m_offset;
};

/** Null value address marker */
static constexpr ColumnDataAddress kNullValueAddress = ColumnDataAddress();

/** Default value address marker */
static constexpr ColumnDataAddress kDefaultValueAddress = ColumnDataAddress(0, 1);

/**
 * Outputs ColumnDataAddress object representation to stream.
 * @param os Output stream.
 * @param addr ColumnDataAddress object.
 */
inline std::ostream& operator<<(std::ostream& os, const ColumnDataAddress& addr)
{
    os << '(' << addr.getBlockId() << ", " << addr.getOffset() << ')';
    return os;
}

}  // namespace siodb::iomgr::dbengine
