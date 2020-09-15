// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDataRecord.h"
#include "MasterColumnRecordPtr.h"

// Common project headers
#include <siodb/iomgr/shared/dbengine/DmlOperationType.h>

// STL headers
#include <vector>

namespace siodb::iomgr::dbengine {

class Table;

/** Master column record data structure */
class MasterColumnRecord {
public:
    /** Maximum allowed master column record size */
    static constexpr std::size_t kMaxSerializedSize = 0x8000;
    /** Master column record size tag size */
    static constexpr std::size_t kMaxSizeTagSize = 2;

public:
    /**
     * Initializes new object of class MasterColumnRecord.
     * (Intentionally no any explicit initialization).
     */
    MasterColumnRecord() noexcept = default;

    /**
     * Initializes new object of class MasterColumnRecord.
     * @param transactionId Incremental number for a transaction.
     * @param createTimestamp Timestamp of the transaction that created this TRID
     *                        at microseconds precision.
     * @param updateTimestamp Timestamp of the transaction at microseconds precision.
     * @param version Record version.
     * @param operationType Operation type.
     * @param userId The user ID of the author of the transaction.
     * @param tableRowId Unique row identifier.
     * @param columnSetId Column set identifier.
     * @param previousVersionAddress Address of the previous version.
     */
    MasterColumnRecord(std::uint64_t transactionId, std::uint64_t createTimestamp,
            std::uint64_t updateTimestamp, std::uint64_t version, DmlOperationType operationType,
            std::uint32_t userId, std::uint64_t tableRowId, std::uint64_t columnSetId,
            const ColumnDataAddress& previousVersionAddress) noexcept
        : m_transactionId(transactionId)
        , m_createTimestamp(createTimestamp)
        , m_updateTimestamp(updateTimestamp)
        , m_version(version)
        , m_operationType(operationType)
        , m_userId(userId)
        , m_tableRowId(tableRowId)
        , m_columnSetId(columnSetId)
        , m_privateDataExpirationTimestamp(0)
        , m_previousVersionAddress(previousVersionAddress)
    {
    }

    /**
     * Initializes new object of class MasterColumnRecord.
     * @param table Table for which this MCR is created.
     * @param transactionId Incremental number for a transaction.
     * @param createTimestamp Timestamp of the transaction that created this TRID
     *                        at microseconds precision.
     * @param updateTimestamp Timestamp of the transaction at microseconds precision.
     * @param version Record version.
     * @param operationType Operation type.
     * @param userId The user ID of the author of the transaction.
     * @param tableRowId Unique row identifier.
     * @param columnSetId Column set identifier.
     * @param previousVersionAddress Address of the previous version.
     */
    MasterColumnRecord(Table& table, std::uint64_t transactionId, std::uint64_t createTimestamp,
            std::uint64_t updateTimestamp, std::uint64_t version, DmlOperationType operationType,
            std::uint32_t userId, std::uint64_t tableRowId, std::uint64_t columnSetId,
            const ColumnDataAddress& previousVersionAddress);

    /**
     * Returns transaction ID.
     * @return Transaction ID.
     */
    std::uint64_t getTransactionId() const noexcept
    {
        return m_transactionId;
    }

    /**
     * Returns create TRID timestamp.
     * @return Create TRID timestamp.
     */
    std::uint64_t getCreateTimestamp() const noexcept
    {
        return m_createTimestamp;
    }

    /**
     * Returns update timestamp.
     * @return Update timestamp.
     */
    std::uint64_t getUpdateTimestamp() const noexcept
    {
        return m_updateTimestamp;
    }

    /**
     * Returns record version.
     * @return Record version.
     */
    std::uint64_t getVersion() const noexcept
    {
        return m_version;
    }

    /**
     * Returns atomic operation type.
     * @return Atomic operation type.
     */
    DmlOperationType getOperationType() const noexcept
    {
        return m_operationType;
    }

    /**
     * Returns user ID.
     * @return User ID.
     */
    std::uint32_t getUserId() const noexcept
    {
        return m_userId;
    }

    /**
     * Returns table row ID.
     * @return Table row ID.
     */
    std::uint64_t getTableRowId() const noexcept
    {
        return m_tableRowId;
    }

    /**
     * Returns column set ID.
     * @return Column set ID.
     */
    std::uint64_t getColumnSetId() const noexcept
    {
        return m_columnSetId;
    }

    /**
     * Returns private data expiration timestamp.
     * @return Private data expiration timestamp.
     */
    std::uint64_t getPrivateDataExpirationTimestamp() const noexcept
    {
        return m_privateDataExpirationTimestamp;
    }

    /**
     * Sets private data expiration timestamp.
     * @param privateDataExpirationTimestamp New private data expiration timestamp.
     */
    void setPrivateDataExpirationTimestamp(std::uint64_t privateDataExpirationTimestamp) noexcept
    {
        m_privateDataExpirationTimestamp = privateDataExpirationTimestamp;
    }

    /**
     * Returns number of column addresses.
     * @return Number of column addresses.
     */
    auto getColumnCount() const noexcept
    {
        return m_columnRecords.size();
    }

    /**
     * Returns collection of column records.
     * @return Collection of column records.
     */
    const auto& getColumnRecords() const noexcept
    {
        return m_columnRecords;
    }

    /**
     * Sets column addresses.
     * @param columnRecords Collection of column records.
     */
    void setColumnRecords(std::vector<ColumnDataRecord>&& columnRecords) noexcept
    {
        m_columnRecords = std::move(columnRecords);
    }

    /**
     * Adds column record.
     * @param columnRecords Column address.
     */
    void addColumnRecord(const ColumnDataRecord& columnRecord)
    {
        m_columnRecords.push_back(columnRecord);
    }

    /**
     * Adds column record.
     * @param address Data address.
     * @param createTimestamp Record creation timestamp.
     * @param updateTimestamp Record update timestamp.
     */
    void addColumnRecord(const ColumnDataAddress& address, std::uint64_t createTimestamp,
            std::uint64_t updateTimestamp)
    {
        m_columnRecords.emplace_back(address, createTimestamp, updateTimestamp);
    }

    /**
     * Reserve storage for specified number of column records.
     * @param count Number of column records.
     */
    void reserveColumnRecords(std::size_t count)
    {
        m_columnRecords.reserve(count);
    }

    /**
     * Returns previous version address.
     * @return Previous version address. Null value address means no previous version.
     */
    const ColumnDataAddress& getPreviousVersionAddress() const noexcept
    {
        return m_previousVersionAddress;
    }

    /**
     * Returns serialized size.
     * @return Actual serialized size.
     */
    std::size_t getSerializedSize() const noexcept;

    /**
     * Returns serialized size including leading size tag.
     * @param size Record size.
     * @return Actual serialized size.
     */
    static std::size_t getSerializedSizeWithSizeTag(std::uint16_t size) noexcept;

    /**
     * Serializes this object into a memory buffer. Checks buffer size.
     * @param buffer Buffer address.
     * @param sizeTag Record size tag.
     * @return Address after the last written byte.
     */
    std::uint8_t* serializeUncheckedWithSizeTag(
            std::uint8_t* buffer, std::uint16_t sizeTag) const noexcept;

    /**
     * De-serializes master column with length from a memory buffer.
     * @param buffer Buffer address.
     * @param dataSize Available data size in buffer.
     * @param masterColumn Master column data structure to fill.
     * @return Number of consumed bytes or zero if data cannot be read.
     */
    std::size_t deserialize(const std::uint8_t* buffer, std::size_t dataSize) noexcept;

    /**
     * Dumps column addresses to stream.
     * @param os Output stream.
     */
    void dumpColumnAddresses(std::ostream& os) const;

    /**
     * Dumps column addresses to string.
     * @return String with dumped column addresses.
     */
    std::string dumpColumnAddresses() const;

private:
    /** Incremental number for a transaction */
    std::uint64_t m_transactionId;

    /** Timestamp of the transaction that created this TRID, at the microsecond precision */
    std::uint64_t m_createTimestamp;

    /** Timestamp of the transaction, at the microsecond precision */
    std::uint64_t m_updateTimestamp;

    /** Record version (incremented at each operation) */
    std::uint64_t m_version;

    /** Operation type that has created or changed this record */
    DmlOperationType m_operationType;

    /** The user ID of the author of the transaction (for the time being 1) */
    std::uint32_t m_userId;

    /**
     * The unique ID generated at each new transaction of **type INSERT only**.
     * It identifies uniquely a Row across multiple columns of the same table.
     */
    std::uint64_t m_tableRowId;

    /** Column set that is effective for this record. */
    std::uint64_t m_columnSetId;

    /**
     * Private data expiration timestamp.
     * Zero value means no expiration.
     */
    std::uint64_t m_privateDataExpirationTimestamp;

    /** Column addresses */
    std::vector<ColumnDataRecord> m_columnRecords;

    /** Previous version address */
    ColumnDataAddress m_previousVersionAddress;
};

/**
 * Outputs master column record to stream.
 * @param os Output stream.
 * @param mcr Master column record.
 * @return Output stream.
 */
std::ostream& operator<<(std::ostream& os, const MasterColumnRecord& mcr);

}  // namespace siodb::iomgr::dbengine
