// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "MasterColumnRecord.h"

// Project headers
#include "Table.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

MasterColumnRecord::MasterColumnRecord(Table& table, std::uint64_t tableRowId,
        std::uint64_t transactionId, std::uint64_t createTimestamp, std::uint64_t updateTimestamp,
        std::uint64_t version, std::uint64_t operationId, DmlOperationType operationType,
        std::uint32_t userId, std::uint64_t columnSetId,
        const ColumnDataAddress& previousVersionAddress)
    : m_tableRowId(tableRowId ? tableRowId : table.generateNextUserTrid())
    , m_transactionId(transactionId)
    , m_createTimestamp(createTimestamp)
    , m_updateTimestamp(updateTimestamp)
    , m_version(version)
    , m_operationId(operationId)
    , m_operationType(operationType)
    , m_userId(userId)
    , m_columnSetId(columnSetId)
    , m_privateDataExpirationTimestamp(0)
    , m_previousVersionAddress(previousVersionAddress)
{
    m_columnRecords.reserve(table.getColumnCount());
}

std::size_t MasterColumnRecord::getSerializedSize() const noexcept
{
    std::size_t size = ::getVarIntSize(m_tableRowId) + ::getVarIntSize(m_transactionId)
                       + ::getVarIntSize(m_createTimestamp) + ::getVarIntSize(m_updateTimestamp)
                       + ::getVarIntSize(m_version) + ::getVarIntSize(m_operationId)
                       + 1  // Atomic operation type is always 1 byte
                       + ::getVarIntSize(m_userId) + ::getVarIntSize(m_columnSetId)
                       + ::getVarIntSize(m_privateDataExpirationTimestamp)
                       + ::getVarIntSize(static_cast<std::uint32_t>(m_columnRecords.size()))
                       + m_previousVersionAddress.getSerializedSize();

    for (const auto& r : m_columnRecords)
        size += r.getSerializedSize();

    return size;
}

std::size_t MasterColumnRecord::getSerializedSizeWithSizeTag(std::uint16_t size) noexcept
{
    return size + ::getVarUInt16Size(size);
}

std::uint8_t* MasterColumnRecord::serializeUncheckedWithSizeTag(
        std::uint8_t* buffer, std::uint16_t sizeTag) const noexcept
{
    buffer = ::encodeVarUInt16(sizeTag, buffer);
    buffer = ::encodeVarInt(m_tableRowId, buffer);
    buffer = ::encodeVarInt(m_transactionId, buffer);
    buffer = ::encodeVarInt(m_createTimestamp, buffer);
    buffer = ::encodeVarInt(m_updateTimestamp, buffer);
    buffer = ::encodeVarInt(m_version, buffer);
    buffer = ::encodeVarInt(m_operationId, buffer);
    *buffer++ = static_cast<std::uint8_t>(m_operationType);
    buffer = ::encodeVarInt(m_userId, buffer);
    buffer = ::encodeVarInt(m_columnSetId, buffer);
    buffer = ::encodeVarInt(m_privateDataExpirationTimestamp, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_columnRecords.size()), buffer);

    for (const auto& r : m_columnRecords)
        buffer = r.serializeUnchecked(buffer);

    buffer = m_previousVersionAddress.serializeUnchecked(buffer);

    return buffer;
}

std::size_t MasterColumnRecord::deserialize(
        const std::uint8_t* buffer, std::size_t dataSize) noexcept
{
    int consumed = ::decodeVarInt(buffer, dataSize, m_tableRowId);
    if (consumed < 1) return 0;
    std::size_t totalConsumed = consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_transactionId);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_createTimestamp);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_updateTimestamp);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_version);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_operationId);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    if (totalConsumed >= dataSize) return 0;
    m_operationType = static_cast<DmlOperationType>(buffer[totalConsumed]);
    ++totalConsumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_userId);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, m_columnSetId);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    consumed = ::decodeVarInt(
            buffer + totalConsumed, dataSize - totalConsumed, m_privateDataExpirationTimestamp);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    std::uint32_t columnRecordCount = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, dataSize - totalConsumed, columnRecordCount);
    if (consumed < 1) return 0;
    totalConsumed += consumed;

    std::vector<ColumnDataRecord> columnRecords;
    if (SIODB_LIKELY(columnRecordCount > 0)) {
        columnRecords.resize(columnRecordCount);
        for (std::uint32_t i = 0; i < columnRecordCount; ++i) {
            const auto consumed1 =
                    columnRecords[i].deserialize(buffer + totalConsumed, dataSize - totalConsumed);
            if (consumed1 < 1) return 0;
            totalConsumed += consumed1;
        }
    }
    m_columnRecords.swap(columnRecords);

    auto consumed1 =
            m_previousVersionAddress.deserialize(buffer + totalConsumed, dataSize - totalConsumed);
    if (consumed1 < 1) return 0;
    totalConsumed += consumed1;

    return totalConsumed;
}

void MasterColumnRecord::dumpColumnAddresses(std::ostream& os) const
{
    os << '[';
    bool first = true;
    for (const auto& r : m_columnRecords) {
        if (first)
            first = false;
        else
            os << ", ";
        os << r.getAddress();
    }
    os << ']';
}

std::string MasterColumnRecord::dumpColumnAddresses() const
{
    std::ostringstream oss;
    dumpColumnAddresses(oss);
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const MasterColumnRecord& mcr)
{
    return os << "TRID: " << mcr.getTableRowId() << ", txnid: " << mcr.getTransactionId()
              << ", opid: " << mcr.getOperationId()
              << ", op: " << static_cast<int>(mcr.getOperationType())
              << ", version: " << mcr.getVersion() << ", user_id: " << mcr.getUserId()
              << ", columns: " << mcr.dumpColumnAddresses();
}

}  // namespace siodb::iomgr::dbengine
