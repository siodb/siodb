// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Column.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDataBlock.h"
#include "Index.h"
#include "LobChunkHeader.h"
#include "ThrowDatabaseError.h"
#include "lob/ColumnBlobStream.h"
#include "lob/ColumnClobStream.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {
void Column::readRecord(
        const ColumnDataAddress& addr, Variant& value, bool lobStreamsMustHoldSource)
{
    //DBG_LOG_DEBUG("Column::readRecord(): " << makeDisplayName() << " at " << addr);

    // Handle NULL value
    if (!addr) {
        value.clear();
        return;
    }

    std::lock_guard lock(m_mutex);
    auto block = findExistingBlock(addr.getBlockId());
    std::uint32_t requiredLength = s_minRequiredBlockFreeSpaces[m_dataType];
    if (addr.getOffset() + requiredLength >= m_dataBlockDataAreaSize) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDataBlockPosition, getDatabaseName(),
                m_table.getName(), m_name, addr.getBlockId(), getDatabaseUuid(), m_table.getId(),
                m_id, addr.getOffset());
    }

    switch (m_dataType) {
        case COLUMN_DATA_TYPE_BOOL: {
            std::uint8_t v = 0;
            block->readData(&v, sizeof(v), addr.getOffset());
            value = (v != 0);
            break;
        }
        case COLUMN_DATA_TYPE_INT8: {
            std::int8_t v = 0;
            block->readData(&v, sizeof(v), addr.getOffset());
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_UINT8: {
            std::uint8_t v = 0;
            block->readData(&v, sizeof(v), addr.getOffset());
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_INT16: {
            std::uint8_t buffer[2];
            block->readData(buffer, sizeof(buffer), addr.getOffset());
            std::int16_t v = 0;
            ::pbeDecodeInt16(buffer, &v);
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_UINT16: {
            std::uint8_t buffer[2];
            block->readData(buffer, sizeof(buffer), addr.getOffset());
            std::uint16_t v = 0;
            ::pbeDecodeUInt16(buffer, &v);
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_INT32: {
            std::uint8_t buffer[4];
            block->readData(buffer, sizeof(buffer), addr.getOffset());
            std::int32_t v = 0;
            ::pbeDecodeInt32(buffer, &v);
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_UINT32: {
            std::uint8_t buffer[4];
            block->readData(buffer, sizeof(buffer), addr.getOffset());
            std::uint32_t v = 0;
            ::pbeDecodeUInt32(buffer, &v);
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_INT64: {
            std::uint8_t buffer[8];
            block->readData(buffer, sizeof(buffer), addr.getOffset());
            std::int64_t v = 0;
            ::pbeDecodeInt64(buffer, &v);
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_UINT64: {
            std::uint8_t buffer[8];
            block->readData(buffer, sizeof(buffer), addr.getOffset());
            std::uint64_t v = 0;
            ::pbeDecodeUInt64(buffer, &v);
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_FLOAT: {
            std::uint8_t buffer[4];
            block->readData(buffer, sizeof(buffer), addr.getOffset());
            float v = 0;
            ::pbeDecodeFloat(buffer, &v);
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_DOUBLE: {
            std::uint8_t buffer[8];
            block->readData(buffer, sizeof(buffer), addr.getOffset());
            double v = 0;
            ::pbeDecodeDouble(buffer, &v);
            value = v;
            break;
        }
        case COLUMN_DATA_TYPE_TEXT: {
            loadText(addr, value, lobStreamsMustHoldSource);
            break;
        }
        case COLUMN_DATA_TYPE_BINARY: {
            loadBinary(addr, value, lobStreamsMustHoldSource);
            break;
        }
        case COLUMN_DATA_TYPE_TIMESTAMP: {
            std::uint8_t buffer[RawDateTime::kMaxSerializedSize];
            block->readData(buffer, RawDateTime::kDatePartSerializedSize, addr.getOffset());
            RawDateTime v;
            v.deserializeDatePart(buffer);
            if (v.m_datePart.m_hasTimePart) {
                block->readData(buffer + 4, sizeof(buffer) - RawDateTime::kDatePartSerializedSize,
                        addr.getOffset() + RawDateTime::kDatePartSerializedSize);
                v.deserialize(buffer, sizeof(buffer));
            }

            value = v;
            break;
        }
        default: throw std::logic_error("invalid data type");
    }  // switch
}

void Column::readMasterColumnRecord(const ColumnDataAddress& addr, MasterColumnRecord& record)
{
    // Read MCR size
    auto block = findExistingBlock(addr.getBlockId());
    std::uint8_t recordSizeBuffer[2];
    auto offset = addr.getOffset();
    block->readData(recordSizeBuffer, 1, offset++);
    // Here was the bug (issue #126)
    if (recordSizeBuffer[0] >= 0x80) block->readData(recordSizeBuffer + 1, 1, offset++);
    std::uint16_t recordSize = 0;
    ::decodeVarUInt16(recordSizeBuffer, sizeof(recordSizeBuffer), &recordSize);

    // Validate MCR size
    if (recordSize > MasterColumnRecord::kMaxSerializedSize) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnRecordSize,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, addr.getBlockId(), addr.getOffset(), recordSize);
    }

    // Read and decode MCR
    std::unique_ptr<std::uint8_t[]> buffer(new std::uint8_t[recordSize]);
    block->readData(buffer.get(), recordSize, offset);
    record.deserialize(buffer.get(), recordSize);
}

std::pair<ColumnDataAddress, ColumnDataAddress> Column::writeRecord(Variant&& value)
{
    std::lock_guard lock(m_mutex);

    // Handle NULL value
    if (value.isNull()) {
        if (m_notNull) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotInsertNullValue, getDatabaseName(),
                    m_table.getName(), m_name);
        } else
            return std::make_pair(kNullValueAddress, kNullValueAddress);
    }

    Variant v;
    std::uint32_t requiredLength = s_minRequiredBlockFreeSpaces[m_dataType];

    try {
        // Cast value to column data type
        switch (m_dataType) {
            case COLUMN_DATA_TYPE_BOOL: {
                if (value.getValueType() != VariantType::kBool) v = value.asBool();
                break;
            }

            case COLUMN_DATA_TYPE_INT8: {
                if (value.getValueType() != VariantType::kInt8) v = value.asInt8();
                break;
            }

            case COLUMN_DATA_TYPE_UINT8: {
                if (value.getValueType() != VariantType::kUInt8) v = value.asUInt8();
                break;
            }

            case COLUMN_DATA_TYPE_INT16: {
                if (value.getValueType() != VariantType::kInt16) v = value.asInt16();
                break;
            }

            case COLUMN_DATA_TYPE_UINT16: {
                if (value.getValueType() != VariantType::kUInt16) v = value.asUInt16();
                break;
            }

            case COLUMN_DATA_TYPE_INT32: {
                if (value.getValueType() != VariantType::kInt32) v = value.asInt32();
                break;
            }

            case COLUMN_DATA_TYPE_UINT32: {
                if (value.getValueType() != VariantType::kUInt32) v = value.asUInt32();
                break;
            }

            case COLUMN_DATA_TYPE_INT64: {
                if (value.getValueType() != VariantType::kInt64) v = value.asInt64();
                break;
            }

            case COLUMN_DATA_TYPE_UINT64: {
                if (value.getValueType() != VariantType::kUInt64) v = value.asUInt64();
                break;
            }

            case COLUMN_DATA_TYPE_FLOAT: {
                if (value.getValueType() != VariantType::kFloat) v = value.asFloat();
                break;
            }

            case COLUMN_DATA_TYPE_DOUBLE: {
                if (value.getValueType() != VariantType::kDouble) v = value.asDouble();
                break;
            }

            case COLUMN_DATA_TYPE_TEXT: {
                switch (value.getValueType()) {
                    case VariantType::kString:
                    case VariantType::kClob: break;
                    case VariantType::kBinary: {
                        if (value.getBinary().size() <= kMaxStringLength / 2)
                            v = value.asString().release();
                        else
                            v = value.asClob().release();
                        break;
                    }
                    case VariantType::kBlob: {
                        if (value.getBlob().getRemainingSize() > kMaxClobLength / 2)
                            throw std::logic_error("BLOB is too long");
                        v = value.asClob().release();
                        break;
                    }
                    default: {
                        v = value.asString().release();
                        break;
                    }
                }
                break;
            }

            case COLUMN_DATA_TYPE_BINARY: {
                switch (value.getValueType()) {
                    case VariantType::kBinary:
                    case VariantType::kBlob: break;
                    case VariantType::kClob: {
                        v = value.asBlob().release();
                        break;
                    }
                    default: {
                        v = value.asBinary().release();
                        break;
                    }
                }
                break;
            }

            case COLUMN_DATA_TYPE_TIMESTAMP: {
                if (value.getValueType() != VariantType::kDateTime) v = value.asDateTime();
                requiredLength = 12;
                break;
            }

            default: throw std::logic_error("invalid data type");
        }  // switch
    } catch (VariantTypeCastError& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorIncompatibleDataType1, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id,
                getColumnDataTypeName(convertVariantTypeToColumnDataType(ex.getDestValueType())),
                static_cast<int>(ex.getDestValueType()),
                getColumnDataTypeName(convertVariantTypeToColumnDataType(ex.getSourceValueType())),
                static_cast<int>(ex.getSourceValueType()));
    } catch (std::logic_error&) {
        throwDatabaseError(IOManagerMessageId::kErrorIncompatibleDataType2, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id,
                getColumnDataTypeName(m_dataType), static_cast<int>(m_dataType),
                getColumnDataTypeName(convertVariantTypeToColumnDataType(value.getValueType())),
                static_cast<int>(value.getValueType()));
    }

    // If no data so far, take value from origin.
    if (v.isNull()) v = std::move(value);

    // Get available block
    auto block = selectAvailableBlockUnlocked(requiredLength);

    // Find available block info before we have written something
    auto itBlock = m_availableDataBlocks.find(block->getId());
    if (itBlock == m_availableDataBlocks.end()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotFindAvailableBlockRecord,
                getDatabaseName(), m_table.getName(), m_name, block->getId(), getDatabaseUuid(),
                m_table.getId(), m_id);
    }

    const auto pos = block->getNextDataPos();
    DBG_LOG_DEBUG(makeDisplayName() << ": writeRecord: block=" << block->getId() << " pos=" << pos);

    // Store data
    switch (m_dataType) {
        case COLUMN_DATA_TYPE_BOOL: {
            const std::uint8_t value = v.getBool() ? 1 : 0;
            block->writeData(&value, sizeof(value));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_INT8: {
            const std::int8_t value = v.getInt8();
            block->writeData(&value, sizeof(value));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_UINT8: {
            const std::uint8_t value = v.getUInt8();
            block->writeData(&value, sizeof(value));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_INT16: {
            std::uint8_t buffer[2];
            ::pbeEncodeInt16(v.getInt16(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_UINT16: {
            std::uint8_t buffer[2];
            ::pbeEncodeUInt16(v.getUInt16(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_INT32: {
            std::uint8_t buffer[4];
            ::pbeEncodeInt32(v.getInt32(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_UINT32: {
            std::uint8_t buffer[4];
            ::pbeEncodeUInt32(v.getUInt32(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_INT64: {
            std::uint8_t buffer[8];
            ::pbeEncodeInt64(v.getInt64(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_UINT64: {
            std::uint8_t buffer[8];
            ::pbeEncodeUInt64(v.getUInt64(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_FLOAT: {
            std::uint8_t buffer[4];
            ::pbeEncodeFloat(v.getFloat(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_DOUBLE: {
            std::uint8_t buffer[8];
            ::pbeEncodeDouble(v.getDouble(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            block->incNextDataPos(requiredLength);
            break;
        }

        case COLUMN_DATA_TYPE_TEXT: {
            if (v.isString()) {
                const auto& s = v.getString();
                writeBuffer(s.c_str(), s.length(), block);
            } else {
                assert(v.isClob());
                return writeLob(v.getClob(), block);
            }
            break;
        }

        case COLUMN_DATA_TYPE_BINARY: {
            if (v.isBinary()) {
                const auto& s = v.getBinary();
                writeBuffer(s.data(), s.size(), block);
            } else {
                assert(v.isBlob());
                return writeLob(v.getBlob(), block);
            }
            break;
        }

        case COLUMN_DATA_TYPE_TIMESTAMP: {
            std::uint8_t buffer[RawDateTime::kMaxSerializedSize];
            block->writeData(&buffer, v.getDateTime().serialize(buffer) - buffer);
            block->incNextDataPos(requiredLength);
            break;
        }

        default: {
            // Should never happen, but make compiler happy
            throw std::logic_error("invalid data type");
        }
    }  // switch

    // Update block free space
    itBlock->second = block->getFreeDataSpace();

    //DBG_LOG_DEBUG("Column::writeRecord(): " << makeDisplayName() << " at "
    //                                       << ColumnDataAddress(block->getId(), pos));

    return std::make_pair(ColumnDataAddress(block->getId(), pos),
            ColumnDataAddress(block->getId(), block->getNextDataPos()));
}

std::pair<ColumnDataAddress, ColumnDataAddress> Column::writeMasterColumnRecord(
        const MasterColumnRecord& record, bool updateMainIndex)
{
    // Check that this is master column
    if (!isMasterColumn()) {
        throwDatabaseError(IOManagerMessageId::kErrorNotMasterColumn, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id);
    }

    // Check that MCR fits to the size limit
    const auto recordSize = record.getSerializedSize();
    const auto recordSizeWithSizeTag = record.getSerializedSizeWithSizeTag(recordSize);
    if (recordSizeWithSizeTag > MasterColumnRecord::kMaxSerializedSize) {
        throwDatabaseError(IOManagerMessageId::kErrorTooManyColumns, getDatabaseName(),
                m_table.getName(), getDatabaseUuid(), m_table.getId());
    }

    std::unique_ptr<std::uint8_t[]> buffer(new std::uint8_t[recordSizeWithSizeTag]);

    std::lock_guard lock(m_mutex);

    // Get available block
    auto block = selectAvailableBlockUnlocked(recordSizeWithSizeTag);

    // Find available block info before we have written something
    auto itBlock = m_availableDataBlocks.find(block->getId());
    if (itBlock == m_availableDataBlocks.end()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotFindAvailableBlockRecord,
                getDatabaseName(), m_table.getName(), m_name, block->getId(), getDatabaseUuid(),
                m_table.getId(), m_id);
    }

    // Store data
    const auto pos = block->getNextDataPos();
    const auto end = record.serializeUncheckedWithSizeTag(buffer.get(), recordSize);
    if (SIODB_UNLIKELY(static_cast<std::size_t>(end - buffer.get()) != recordSizeWithSizeTag))
        throw std::runtime_error("Invalid MCR serialization");
    block->writeData(buffer.get(), recordSizeWithSizeTag);

    // Update main index
    std::uint8_t indexKey[8];
    ::pbeEncodeUInt64(record.getTableRowId(), indexKey);
    IndexValue indexValue;
    ::pbeEncodeUInt64(block->getId(), indexValue.m_data);
    ::pbeEncodeUInt32(pos, indexValue.m_data + 8);

    if (SIODB_LIKELY(updateMainIndex)) {
        switch (record.getOperationType()) {
            case DmlOperationType::kInsert: {
                if (!m_masterColumnData->m_mainIndex->insert(indexKey, indexValue.m_data)) {
                    throwDatabaseError(IOManagerMessageId::kErrorCannotInsertDuplicateTrid,
                            getDatabaseName(), getTableName(), m_name, record.getTableRowId());
                }
                break;
            }
            case DmlOperationType::kDelete: {
                m_masterColumnData->m_mainIndex->erase(indexKey);
                break;
            }
            case DmlOperationType::kUpdate: {
                m_masterColumnData->m_mainIndex->update(indexKey, indexValue.m_data);
                break;
            }
        }
    }

    // Update block free space
    block->incNextDataPos(recordSizeWithSizeTag);
    itBlock->second = block->getFreeDataSpace();

    DBG_LOG_DEBUG("Column::writeMasterColumnRecord(): " << makeDisplayName() << ": MCR: " << record
                                                        << " at "
                                                        << ColumnDataAddress(block->getId(), pos));

    return std::make_pair(ColumnDataAddress(block->getId(), pos),
            ColumnDataAddress(block->getId(), block->getNextDataPos()));
}

/// --- internals ---

std::pair<ColumnDataAddress, ColumnDataAddress> Column::writeBuffer(
        const void* src, std::uint32_t length, ColumnDataBlockPtr block)
{
    // Remember initial address
    ColumnDataAddress result;

    // Initialize chunk counters
    std::uint32_t chunkId = 1;
    std::uint8_t headerBuffer[LobChunkHeader::kSerializedSize];
    std::uint32_t lastHeaderPos = 0;
    LobChunkHeader lastHeader(0, 0);
    ColumnDataBlockPtr lastBlock;

    do {
        // Compute potential chunk length
        auto freeSpace = block->getFreeDataSpace();
        LobChunkHeader header(length, std::min(freeSpace, length));

        // Check if chunk header fits into remaining free space in the block
        if (freeSpace < LobChunkHeader::kSerializedSize) {
            auto newBlock = createOrGetNextBlock(
                    *block, LobChunkHeader::kSerializedSize + kBlockFreeSpaceThresholdForLob);
            if (chunkId == 1)
                result = ColumnDataAddress(block->getId(), block->getNextDataPos());
            else {
                // Update next chunk position in the header of the last chunk
                lastHeader.m_nextChunkBlockId = newBlock->getId();
                lastHeader.m_nextChunkOffset = newBlock->getNextDataPos();
                lastHeader.serialize(headerBuffer);
                block->writeData(headerBuffer, LobChunkHeader::kSerializedSize, lastHeaderPos);
            }
            block = newBlock;
            freeSpace = block->getFreeDataSpace();
            header.m_chunkLength = std::min(freeSpace, length);
        }

        // Compute actual chunk length
        const auto availableSpace =
                static_cast<std::uint32_t>(freeSpace - LobChunkHeader::kSerializedSize);
        header.m_chunkLength = std::min(availableSpace, length);

        // Write header
        lastHeaderPos = block->getNextDataPos();
        header.serialize(headerBuffer);
        block->writeData(headerBuffer, LobChunkHeader::kSerializedSize);
        block->incNextDataPos(LobChunkHeader::kSerializedSize);
        lastHeader = header;
        lastBlock = block;

        if (header.m_chunkLength > 0) {
            block->writeData(src, header.m_chunkLength);
            block->incNextDataPos(header.m_chunkLength);
            src = static_cast<const uint8_t*>(src) + header.m_chunkLength;
            length -= header.m_chunkLength;
        }

        // Update counters
        ++chunkId;
    } while (length > 0);

    return std::make_pair(result, ColumnDataAddress(block->getId(), block->getNextDataPos()));
}

std::pair<ColumnDataAddress, ColumnDataAddress> Column::writeLob(
        LobStream& lob, ColumnDataBlockPtr block)
{
    ColumnDataAddress result;

    // Initialize chunk counters
    std::uint32_t chunkId = 1;
    std::uint8_t headerBuffer[LobChunkHeader::kSerializedSize];
    std::unique_ptr<std::uint8_t[]> dataBuffer;
    std::uint32_t lastHeaderPos = 0;
    LobChunkHeader lastHeader(0, 0);
    ColumnDataBlockPtr lastBlock;

    do {
        // Compute potential chunk length
        const auto remainingLobSize = lob.getRemainingSize();
        auto freeSpace = block->getFreeDataSpace();
        LobChunkHeader header(remainingLobSize, std::min(freeSpace, remainingLobSize));

        // Check if chunk header fits into remaining free space in the block
        if (freeSpace < LobChunkHeader::kSerializedSize) {
            auto newBlock = createOrGetNextBlock(
                    *block, LobChunkHeader::kSerializedSize + kBlockFreeSpaceThresholdForLob);
            if (chunkId == 1)
                result = ColumnDataAddress(block->getId(), block->getNextDataPos());
            else {
                // Update next chunk position in the header of the last chunk
                lastHeader.m_nextChunkBlockId = newBlock->getId();
                lastHeader.m_nextChunkOffset = newBlock->getNextDataPos();
                lastHeader.serialize(headerBuffer);
                block->writeData(headerBuffer, LobChunkHeader::kSerializedSize, lastHeaderPos);
            }
            block = newBlock;
            freeSpace = block->getFreeDataSpace();
            header.m_chunkLength = std::min(freeSpace, remainingLobSize);
        }

        // Compute actual chunk length
        const auto availableSpace =
                static_cast<std::uint32_t>(freeSpace - LobChunkHeader::kSerializedSize);
        header.m_chunkLength = std::min(availableSpace, remainingLobSize);

        // Write header
        lastHeaderPos = block->getNextDataPos();
        header.serialize(headerBuffer);
        block->writeData(headerBuffer, LobChunkHeader::kSerializedSize);
        block->incNextDataPos(LobChunkHeader::kSerializedSize);
        lastHeader = header;
        lastBlock = block;

        if (header.m_chunkLength > 0) {
            // Read LOB into data buffer
            if (!dataBuffer) dataBuffer.reset(new std::uint8_t[m_dataBlockDataAreaSize]);
            auto data = dataBuffer.get();
            std::size_t remainingToRead = header.m_chunkLength;
            while (remainingToRead > 0) {
                const auto actuallyRead = lob.read(data, remainingToRead);
                if (actuallyRead < 1) {
                    throwDatabaseError(IOManagerMessageId::kErrorLobReadFailed, getDatabaseName(),
                            m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id);
                }
                data += actuallyRead;
                remainingToRead -= actuallyRead;
            }
            // Write data
            block->writeData(dataBuffer.get(), header.m_chunkLength);
            // Advance next data position
            block->incNextDataPos(header.m_chunkLength);
        }

        // Update counters
        ++chunkId;
    } while (lob.getRemainingSize() > 0);

    return std::make_pair(result, ColumnDataAddress(block->getId(), block->getNextDataPos()));
}

void Column::loadText(const ColumnDataAddress& addr, Variant& value, bool lobStreamsMustHoldSource)
{
    auto block = findExistingBlock(addr.getBlockId());
    LobChunkHeader chunkHeader;
    loadLobChunkHeaderUnlocked(*block, addr.getOffset(), chunkHeader);
    if (chunkHeader.m_remainingLobLength == 0) {
        // Empty string
        value = std::string();
    } else if (chunkHeader.m_remainingLobLength < kSmallLobSizeLimit) {
        // Small text, load into string
        std::unique_ptr<char[]> buffer(new char[chunkHeader.m_remainingLobLength + 1]);
        ColumnClobStream stream(*this, addr, lobStreamsMustHoldSource);
        stream.read(buffer.get(), chunkHeader.m_remainingLobLength);
        buffer.get()[chunkHeader.m_remainingLobLength] = '\0';
        value = std::string(buffer.get());
    } else {
        // Big text, need CLOB stream
        auto stream = std::make_unique<ColumnClobStream>(*this, addr, lobStreamsMustHoldSource);
        value.clear();
        value = stream.release();
    }
}

void Column::loadBinary(
        const ColumnDataAddress& addr, Variant& value, bool lobStreamsMustHoldSource)
{
    auto block = findExistingBlock(addr.getBlockId());
    LobChunkHeader chunkHeader;
    loadLobChunkHeaderUnlocked(*block, addr.getOffset(), chunkHeader);
    if (chunkHeader.m_remainingLobLength == 0) {
        // Empty binary
        value = BinaryValue();
    } else if (chunkHeader.m_remainingLobLength < kSmallLobSizeLimit) {
        // Small binary, load into buffer
        BinaryValue bv(chunkHeader.m_remainingLobLength);
        ColumnBlobStream stream(*this, addr, lobStreamsMustHoldSource);
        stream.read(bv.data(), bv.size());
        value = std::move(bv);
    } else {
        // Big binary, need BLOB stream
        auto stream = std::make_unique<ColumnBlobStream>(*this, addr, lobStreamsMustHoldSource);
        value.clear();
        value = stream.release();
    }
}

std::uint32_t Column::loadLobChunkHeaderUnlocked(
        ColumnDataBlock& block, std::uint32_t offset, LobChunkHeader& chunkHeader)
{
    if (m_dataBlockDataAreaSize - offset < LobChunkHeader::kSerializedSize) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeaderAddress,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, block.getId(), offset);
    }
    std::uint8_t buffer[LobChunkHeader::kSerializedSize];
    block.readData(buffer, LobChunkHeader::kSerializedSize, offset);
    auto endOfHeader = chunkHeader.deserialize(buffer);
    if (!endOfHeader) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, block.getId(),
                offset, "header data format error");
    }
    auto offsetInBlock = offset + (endOfHeader - buffer);
    if (chunkHeader.m_chunkLength > chunkHeader.m_remainingLobLength) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, block.getId(),
                offset, "invalid chunk length");
    }
    if (chunkHeader.m_chunkLength > m_dataBlockDataAreaSize - offsetInBlock) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, block.getId(),
                offset, "chunk length is greater than available data in the block");
    }
    if (chunkHeader.m_nextChunkBlockId > m_blockRegistry.getLastBlockId()) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, block.getId(),
                offset, "invalid next chunk block ID");
    }
    if (chunkHeader.m_nextChunkOffset > m_dataBlockDataAreaSize - LobChunkHeader::kSerializedSize) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, block.getId(),
                offset, "invalid next chunk offset");
    }
    return offsetInBlock;
}

}  // namespace siodb::iomgr::dbengine
