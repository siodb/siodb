// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Column.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDataBlock.h"
#include "ColumnDefinitionConstraintList.h"
#include "DatabaseObjectName.h"
#include "IndexColumn.h"
#include "LobChunkHeader.h"
#include "ThrowDatabaseError.h"
#include "lob/ColumnBlobStream.h"
#include "lob/ColumnClobStream.h"
#include "uli/UInt64UniqueLinearIndex.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/Base128VariantEncoding.h>
#include <siodb/common/utils/ByteOrder.h>
#include <siodb/common/utils/FsUtils.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// Boost headers
#include <boost/format.hpp>

namespace siodb::iomgr::dbengine {

const std::array<std::uint32_t, ColumnDataType_MAX> Column::m_minRequiredBlockFreeSpaces {
        1,  // COLUMN_DATA_TYPE_BOOL
        1,  // COLUMN_DATA_TYPE_INT8
        1,  // COLUMN_DATA_TYPE_UINT8
        2,  // COLUMN_DATA_TYPE_INT16
        2,  // COLUMN_DATA_TYPE_UINT16
        4,  // COLUMN_DATA_TYPE_INT32
        4,  // COLUMN_DATA_TYPE_UINT32
        8,  // COLUMN_DATA_TYPE_INT64
        8,  // COLUMN_DATA_TYPE_UINT64
        4,  // COLUMN_DATA_TYPE_FLOAT
        8,  // COLUMN_DATA_TYPE_DOUBLE
        10,  // COLUMN_DATA_TYPE_TEXT (LOB header size)
        10,  // COLUMN_DATA_TYPE_BINARY (LOB header size)
        RawDateTime::kMaxSerializedSize  // COLUMN_DATA_TYPE_TIMESTAMP
        // TODO: Support more data types in this mapping
        // COLUMN_DATA_TYPE_DATE
        // COLUMN_DATA_TYPE_TIME
        // COLUMN_DATA_TYPE_TIME_WITH_TZ
        // COLUMN_DATA_TYPE_TIMESTAMP_WITH_TZ
        // COLUMN_DATA_TYPE_DATE_INTERVAL
        // COLUMN_DATA_TYPE_TIME_INTERVAL
        // COLUMN_DATA_TYPE_STRUCT
        // COLUMN_DATA_TYPE_XML
        // COLUMN_DATA_TYPE_JSON
        // COLUMN_DATA_TYPE_UUID
};

const std::unordered_set<std::string> Column::m_wellKnownIgnorableFiles {
        Column::kInitializationFlagFile,
        Column::kMainIndexIdFile,
        Column::kTridCounterFile,
};

Column::Column(Table& table, ColumnSpecification&& spec, std::uint64_t firstUserTrid)
    : m_table(table)
    , m_name(validateColumnName(std::move(spec.m_name)))
    , m_description(std::move(spec.m_description))
    , m_dataType(validateColumnDataType(spec.m_dataType))
    , m_id(getDatabase().generateNextColumnId(m_table.isSystemTable()))
    , m_dataBlockDataAreaSize(spec.m_dataBlockDataAreaSize)
    , m_dataDir(ensureDataDir(true))
    , m_masterColumnData(maybeCreateMasterColumnData(true, firstUserTrid))
    , m_columnDefinitionCache(kColumnDefinitionCacheCapacity)
    , m_currentColumnDefinition(createColumnDefinitionUnlocked())
    , m_notNull(false)
    , m_blockRegistry(*this, true)
    , m_lastBlockId(m_blockRegistry.getLastBlockId())
    , m_blockCache(getDatabase().getInstance().getBlockCacheCapacity())
{
    if (isMasterColumn()) {
        if (!spec.m_constraints.empty()) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorExplicitConstraintsForMasterColumnProhibited,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id);
        }

        createMasterColumnConstraints();
        m_currentColumnDefinition->markClosedForModification();

        createMasterColumnMainIndex();
        getDatabase().registerIndex(*m_masterColumnData->m_mainIndex);
    } else {
        for (auto& constraintSpec : spec.m_constraints) {
            BinaryValue serializedConstraintExpression;
            if (constraintSpec.m_expression) {
                serializedConstraintExpression.resize(
                        constraintSpec.m_expression->getSerializedSize());
                constraintSpec.m_expression->serializeUnchecked(
                        serializedConstraintExpression.data());
            }
            const auto constraintDefinition = getDatabase().findOrCreateConstraintDefinition(
                    m_table.isSystemTable(), constraintSpec.m_type, serializedConstraintExpression);
            m_currentColumnDefinition->addConstraint(
                    m_table.createConstraint(std::move(constraintSpec.m_name), constraintDefinition,
                            this, std::move(constraintSpec.m_description)));
        }
        m_currentColumnDefinition->markClosedForModification();
    }

    m_notNull = m_currentColumnDefinition->isNotNull();

    createInitializationFlagFile();
}

Column::Column(Table& table, const ColumnRecord& columnRecord, std::uint64_t firstUserTrid)
    : m_table(validateTable(table, columnRecord))
    , m_name(validateColumnName(std::string(columnRecord.m_name)))
    , m_description(columnRecord.m_description)
    , m_dataType(validateColumnDataType(columnRecord.m_dataType))
    , m_id(columnRecord.m_id)
    , m_dataBlockDataAreaSize(columnRecord.m_dataBlockDataAreaSize)
    , m_dataDir(ensureDataDir())
    , m_masterColumnData(maybeCreateMasterColumnData(false, firstUserTrid))
    , m_columnDefinitionCache(kColumnDefinitionCacheCapacity)
    , m_currentColumnDefinition(getColumnDefinitionChecked(
              getDatabase().getLatestColumnDefinitionIdForColumn(m_table.getId(), m_id)))
    , m_notNull(m_currentColumnDefinition->isNotNull())
    , m_blockRegistry(*this)
    , m_lastBlockId(m_blockRegistry.getLastBlockId())
    , m_blockCache(table.getDatabase().getInstance().getBlockCacheCapacity())
{
    checkDataConsistency();
}

std::string Column::getDisplayName() const
{
    std::ostringstream oss;
    oss << '\'' << getDatabaseName() << "'.'" << m_table.getName() << "'.'" << m_name << '\'';
    return oss.str();
}

std::string Column::getDisplayCode() const
{
    std::ostringstream oss;
    oss << getDatabaseUuid() << '.' << m_table.getId() << '.' << m_id;
    return oss.str();
}

ColumnDefinitionPtr Column::getColumnDefinitionChecked(std::uint64_t columnDefinitionId)
{
    std::lock_guard lock(m_mutex);
    const auto cachedColumnDef = m_columnDefinitionCache.get(columnDefinitionId);
    if (cachedColumnDef) return *cachedColumnDef;
    return loadColumnDefinitionUnlocked(columnDefinitionId);
}

ColumnDefinitionPtr Column::getCurrentColumnDefinition() const
{
    std::lock_guard lock(m_mutex);
    return m_currentColumnDefinition;
}

ColumnDefinitionPtr Column::getPrevColumnDefinition() const
{
    std::lock_guard lock(m_mutex);
    return m_prevColumnDefinition;
}

ColumnDataBlockPtr Column::createBlock(std::uint64_t prevBlockId, ColumnDataBlockState state)
{
    std::lock_guard lock(m_mutex);
    auto block = std::make_shared<ColumnDataBlock>(*this, prevBlockId, state);
    m_blockCache.emplace(block->getId(), block);
    m_blockRegistry.recordBlockAndNextBlock(block->getId(), prevBlockId);
    return block;
}

std::uint64_t Column::getPrevBlockId(std::uint64_t blockId) const
{
    std::lock_guard lock(m_mutex);
    return m_blockRegistry.getPrevBlockId(blockId);
}

void Column::updateBlockState(std::uint64_t blockId, ColumnDataBlockState state) const
{
    std::lock_guard lock(m_mutex);
    m_blockRegistry.updateBlockState(blockId, state);
}

void Column::readRecord(
        const ColumnDataAddress& addr, Variant& value, bool lobStreamsMustHoldSource)
{
    //DBG_LOG_DEBUG("Column::readRecord(): " << getDisplayName() << " at " << addr);

    // Handle NULL value
    if (addr.isNullValueAddress()) {
        value.clear();
        return;
    }

    std::lock_guard lock(m_mutex);
    auto block = getExistingBlock(addr.getBlockId());
    std::uint32_t requiredLength = m_minRequiredBlockFreeSpaces[m_dataType];
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
    auto block = getExistingBlock(addr.getBlockId());
    std::uint8_t recordSizeBuffer[2];
    auto offset = addr.getOffset();
    block->readData(recordSizeBuffer, 1, offset++);
    if (recordSizeBuffer[0] > 0x80) block->readData(recordSizeBuffer + 1, 1, offset++);
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

std::pair<ColumnDataAddress, ColumnDataAddress> Column::putRecord(Variant&& value)
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
    std::uint32_t requiredLength = m_minRequiredBlockFreeSpaces[m_dataType];

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
        throwDatabaseError(IOManagerMessageId::kErrorIncompatibleDataType, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id,
                static_cast<int>(ex.getDestValueType()), static_cast<int>(ex.getSourceValueType()));
    } catch (std::logic_error&) {
        throwDatabaseError(IOManagerMessageId::kErrorIncompatibleDataType2, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id,
                static_cast<int>(m_dataType), static_cast<int>(value.getValueType()));
    }

    // If no data so far, take value from origin.
    if (v.isNull()) v.swap(value);

    // Get available block
    auto block = selectAvailableBlock(requiredLength);

    // Find available block info before we have written something
    auto itBlock = m_availableDataBlocks.find(block->getId());
    if (itBlock == m_availableDataBlocks.end()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotFindAvailableBlockRecord,
                getDatabaseName(), m_table.getName(), m_name, block->getId(), getDatabaseUuid(),
                m_table.getId(), m_id);
    }

    const auto pos = block->getNextDataPos();
    std::uint32_t actualLength = requiredLength;

    // Store data
    switch (m_dataType) {
        case COLUMN_DATA_TYPE_BOOL: {
            const std::uint8_t value = v.getBool() ? 1 : 0;
            block->writeData(&value, sizeof(value));
            break;
        }

        case COLUMN_DATA_TYPE_INT8: {
            const std::int8_t value = v.getInt8();
            block->writeData(&value, sizeof(value));
            break;
        }

        case COLUMN_DATA_TYPE_UINT8: {
            const std::uint8_t value = v.getUInt8();
            block->writeData(&value, sizeof(value));
            break;
        }

        case COLUMN_DATA_TYPE_INT16: {
            std::uint8_t buffer[2];
            ::pbeEncodeInt16(v.getInt16(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            break;
        }

        case COLUMN_DATA_TYPE_UINT16: {
            std::uint8_t buffer[2];
            ::pbeEncodeUInt16(v.getUInt16(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            break;
        }

        case COLUMN_DATA_TYPE_INT32: {
            std::uint8_t buffer[4];
            ::pbeEncodeInt32(v.getInt32(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            break;
        }

        case COLUMN_DATA_TYPE_UINT32: {
            std::uint8_t buffer[4];
            ::pbeEncodeUInt32(v.getUInt32(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            break;
        }

        case COLUMN_DATA_TYPE_INT64: {
            std::uint8_t buffer[8];
            ::pbeEncodeInt64(v.getInt64(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            break;
        }

        case COLUMN_DATA_TYPE_UINT64: {
            std::uint8_t buffer[8];
            ::pbeEncodeUInt64(v.getUInt64(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            break;
        }

        case COLUMN_DATA_TYPE_FLOAT: {
            std::uint8_t buffer[4];
            ::pbeEncodeFloat(v.getFloat(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            break;
        }

        case COLUMN_DATA_TYPE_DOUBLE: {
            std::uint8_t buffer[8];
            ::pbeEncodeDouble(v.getDouble(), buffer);
            block->writeData(&buffer, sizeof(buffer));
            break;
        }

        case COLUMN_DATA_TYPE_TEXT: {
            if (v.isString()) {
                const auto& s = v.getString();
                storeBuffer(s.c_str(), s.length(), block);
            } else {
                assert(v.isClob());
                return storeClob(v.getClob(), block);
            }
            break;
        }

        case COLUMN_DATA_TYPE_BINARY: {
            if (v.isBinary()) {
                const auto& s = v.getBinary();
                storeBuffer(s.data(), s.size(), block);
            } else {
                assert(v.isBlob());
                return storeBlob(v.getBlob(), block);
            }
            break;
        }

        case COLUMN_DATA_TYPE_TIMESTAMP: {
            std::uint8_t buffer[RawDateTime::kMaxSerializedSize];
            block->writeData(&buffer, v.getDateTime().serialize(buffer) - buffer);
            break;
        }

        default: {
            // Should never happen, but make compiler happy
            throw std::logic_error("invalid data type");
        }
    }  // switch

    // Update block free space
    block->incNextDataPos(actualLength);
    itBlock->second = block->getFreeDataSpace();

    //DBG_LOG_DEBUG("Column::putRecord(): " << getDisplayName() << " at "
    //                                       << ColumnDataAddress(block->getId(), pos));

    return std::make_pair(ColumnDataAddress(block->getId(), pos),
            ColumnDataAddress(block->getId(), block->getNextDataPos()));
}

std::pair<ColumnDataAddress, ColumnDataAddress> Column::putMasterColumnRecord(
        const MasterColumnRecord& record)
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
    auto block = selectAvailableBlock(recordSizeWithSizeTag);

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
    std::uint8_t indexValue[12];
    ::pbeEncodeUInt64(block->getId(), indexValue);
    ::pbeEncodeUInt32(pos, indexValue + 8);

    switch (record.getAtomicOperationType()) {
        case DmlOperationType::kInsert: {
            m_masterColumnData->m_mainIndex->insert(indexKey, indexValue, true);
            break;
        }
        case DmlOperationType::kDelete: {
            m_masterColumnData->m_mainIndex->markAsDeleted(indexKey, indexValue);
            break;
        }
        case DmlOperationType::kUpdate: {
            m_masterColumnData->m_mainIndex->update(indexKey, indexValue);
            break;
        }
    }

    // Update block free space
    block->incNextDataPos(recordSizeWithSizeTag);
    itBlock->second = block->getFreeDataSpace();

    DBG_LOG_DEBUG("Column::putMasterColumnRecord(): " << getDisplayName() << ": MCR: " << record
                                                      << " at "
                                                      << ColumnDataAddress(block->getId(), pos));

    return std::make_pair(ColumnDataAddress(block->getId(), pos),
            ColumnDataAddress(block->getId(), block->getNextDataPos()));
}

void Column::eraseFromMasterColumnRecordMainIndex(std::uint64_t trid)
{
    // Check that this is master column
    if (!isMasterColumn()) {
        throwDatabaseError(IOManagerMessageId::kErrorNotMasterColumn, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id);
    }

    std::lock_guard lock(m_mutex);

    // Update main index
    std::uint8_t indexKey[8];
    ::pbeEncodeUInt64(trid, indexKey);
    m_masterColumnData->m_mainIndex->erase(indexKey);
}

void Column::rollbackToAddress(
        const ColumnDataAddress& addr, const std::uint64_t firstAvailableBlockId)
{
    std::lock_guard lock(m_mutex);

    // Check first available data block
    if (m_availableDataBlocks.count(firstAvailableBlockId) == 0) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidCurrentDataBlock, getDatabaseName(),
                m_table.getName(), m_name, firstAvailableBlockId, getDatabaseUuid(),
                m_table.getId(), m_id);
    }

    // Check rollback block ID
    if (addr.getBlockId() > firstAvailableBlockId) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidRollbackDataBlockPosition,
                getDatabaseName(), m_table.getName(), m_name, addr.getBlockId(), getDatabaseUuid(),
                m_table.getId(), m_id, addr.getOffset(), firstAvailableBlockId);
    }

    // Check that target block really exists
    auto block = loadBlock(addr.getBlockId());
    if (!block) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist, getDatabaseName(),
                m_table.getName(), m_name, addr.getBlockId(), getDatabaseUuid(), m_table.getId(),
                m_id);
    }

    // Validate offset
    if (addr.getOffset() >= m_dataBlockDataAreaSize) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDataBlockPosition, getDatabaseName(),
                m_table.getName(), getName(), addr.getBlockId(), getDatabaseUuid(), m_table.getId(),
                m_id, addr.getOffset());
    }

    // Walk through blocks
    auto currentBlockId = firstAvailableBlockId;
    while (currentBlockId != addr.getBlockId()) {
        // Load block
        block = loadBlock(currentBlockId);
        if (!block) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist,
                    getDatabaseName(), m_table.getName(), m_name, addr.getBlockId(),
                    getDatabaseUuid(), m_table.getId(), m_id);
        }

        // Adjust block metadata
        block->setNextDataPos(0);
        block->resetFillTimestamp();
        block->saveHeader();

        // Update block free space info
        updateAvailableBlock(*block);

        // Move to next block
        currentBlockId = block->getPrevBlockId();
        if (currentBlockId == 0) {
            throwDatabaseError(IOManagerMessageId::kErrorUnreachableRollbackDataBlockPosition,
                    getDatabaseName(), m_table.getName(), m_name, addr.getBlockId(),
                    getDatabaseUuid(), m_table.getId(), m_id);
        }
    }

    // Adjust block metadata
    block->setNextDataPos(addr.getOffset());
    if (block->getId() != firstAvailableBlockId) {
        block->resetFillTimestamp();
        block->saveHeader();
    }

    updateAvailableBlock(*block);
}

std::uint32_t Column::loadLobChunkHeader(
        std::uint64_t blockId, std::uint32_t offset, LobChunkHeader& header)
{
    std::lock_guard lock(m_mutex);
    auto block = getExistingBlock(blockId);
    return loadLobChunkHeaderUnlocked(*block, offset, header);
}

void Column::readData(
        std::uint64_t blockId, std::uint32_t offset, void* buffer, std::size_t bufferSize)
{
    std::lock_guard lock(m_mutex);
    auto block = getExistingBlock(blockId);
    block->readData(buffer, bufferSize, offset);
}

std::uint64_t Column::generateNextUserTrid()
{
    if (!m_masterColumnData) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotGenerateUserTridUsingNonMasterColumn,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id);
    }

    if (m_masterColumnData->m_tridCounters->m_lastUserTrid
            == std::numeric_limits<std::uint64_t>::max()) {
        throwDatabaseError(
                IOManagerMessageId::kErrorUserTridExhausted, getDatabaseName(), m_table.getName());
    }

    return ++m_masterColumnData->m_tridCounters->m_lastUserTrid;
}

std::uint64_t Column::generateNextSystemTrid()
{
    if (!m_masterColumnData) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotGenerateSystemTridUsingNonMasterColumn,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id);
    }

    if (m_masterColumnData->m_tridCounters->m_lastSystemTrid
            == m_masterColumnData->m_firstUserTrid - 1) {
        throwDatabaseError(
                IOManagerMessageId::kErrorSystemTridExhausted, getDatabaseName(), m_name);
    }

    return ++m_masterColumnData->m_tridCounters->m_lastSystemTrid;
}

void Column::setLastSystemTrid(std::uint64_t lastSystemTrid)
{
    if (lastSystemTrid >= m_masterColumnData->m_firstUserTrid) {
        throwDatabaseError(
                IOManagerMessageId::kErrorSystemTridExhausted, getDatabaseName(), m_name);
    }
    m_masterColumnData->m_tridCounters->m_lastSystemTrid = lastSystemTrid;
}

int Column::createTridCountersFile(std::uint64_t firstUserTrid)
{
    const auto tridCounterFilePath = utils::constructPath(m_dataDir, kTridCounterFile);
    FileDescriptorGuard fd(::open(tridCounterFilePath.c_str(),
            O_CREAT | O_RDWR | O_DSYNC | O_CLOEXEC, kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateTridCountersFile,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, errorCode, std::strerror(errorCode));
    }
    TridCounters data(firstUserTrid);
    writeFullTridCounters(fd.getFd(), data);
    return fd.release();
}

int Column::openTridCountersFile()
{
    const auto tridCounterFilePath = utils::constructPath(m_dataDir, kTridCounterFile);
    FileDescriptorGuard fd(::open(tridCounterFilePath.c_str(), O_RDWR | O_DSYNC | O_CLOEXEC));
    if (!fd.isValidFd()) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenTridCounterFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }
    TridCounters data(0);
    if (readExact(fd.getFd(), &data, sizeof(data), kIgnoreSignals) != sizeof(data)) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadTridCounterFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }
    if (data.m_marker != TridCounters::kMarker) {
        if (boost::endian::endian_reverse(data.m_marker) != TridCounters::kMarker) {
            throwDatabaseError(IOManagerMessageId::kErrorTridCounterFileCorrupted,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id);
        }
        const auto tridCounterMigrationFilePath =
                tridCounterFilePath + kTridCounterMigrationFileExt;
        if (::rename(tridCounterFilePath.c_str(), tridCounterMigrationFilePath.c_str())) {
            const auto errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotReadTridCounterFile,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id, errorCode, std::strerror(errorCode));
        }
        utils::reverseByteOrder(data.m_lastUserTrid);
        utils::reverseByteOrder(data.m_lastSystemTrid);
        writeFullTridCounters(fd.getFd(), data);
        if (::rename(tridCounterMigrationFilePath.c_str(), tridCounterFilePath.c_str())) {
            const auto errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotReadTridCounterFile,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id, errorCode, std::strerror(errorCode));
        }
    }
    return fd.release();
}

void Column::loadMasterColumnMainIndex()
{
    if (m_masterColumnData->m_mainIndex) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnMainIndexAlreadyExists,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id);
    }

    // Open file
    const auto mainIndexIdFilePath = utils::constructPath(m_dataDir, kMainIndexIdFile);
    FileDescriptorGuard fd(::open(mainIndexIdFilePath.c_str(),
            O_RDWR | O_DSYNC | O_CLOEXEC | O_NOATIME, kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenMainIndexIdFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }

    // Read data
    std::uint64_t indexId0 = 0;
    if (readExact(fd.getFd(), &indexId0, sizeof(indexId0), kIgnoreSignals) != sizeof(indexId0)) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadMainIndexIdFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }
    fd.reset();

    // Decode data
    std::uint64_t indexId = 0;
    ::pbeDecodeUInt64(reinterpret_cast<const std::uint8_t*>(&indexId0), &indexId);

    // Create index
    const auto indexRecord = getDatabase().getIndexRecord(indexId);
    m_masterColumnData->m_mainIndex = std::make_shared<UInt64UniqueLinearIndex>(
            m_table, indexRecord, kMasterColumnNameMainIndexValueSize);
}

// ----- internal -----

Table& Column::validateTable(Table& table, const ColumnRecord& columnRecord)
{
    if (columnRecord.m_tableId == table.getId()) return table;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnTable, columnRecord.m_id,
            columnRecord.m_tableId, table.getDatabaseName(), table.getName(),
            table.getDatabaseUuid(), table.getId());
}

std::string&& Column::validateColumnName(std::string&& columnName) const
{
    if (isValidDatabaseObjectName(columnName)) return std::move(columnName);
    throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnNameInTableColumn, getDatabaseName(),
            m_table.getName(), columnName);
}

ColumnDataType Column::validateColumnDataType(ColumnDataType dataType) const
{
    if (dataType < 0 || dataType >= COLUMN_DATA_TYPE_MAX) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnDataTypeInTableColumn,
                static_cast<int>(dataType), getDatabaseName(), m_table.getName(), m_name);
    }
    if (isMasterColumnName() && dataType != kMasterColumnDataType) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnDataType, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id);
    }
    return dataType;
}

void Column::checkDataConsistency()
{
    struct BlockInfo {
        std::uint64_t m_currentBlockId;
        std::uint64_t m_prevBlockId;
        ColumnDataBlockHeader::Digest m_prevBlockDigest;
    };

    std::stack<BlockInfo> stack;

    // Start with very first block of the column, physically present on disk.
    BlockInfo blockInfo;
    blockInfo.m_currentBlockId = findFirstBlock();
    if (blockInfo.m_currentBlockId != 0) {
        blockInfo.m_prevBlockId = 0;
        blockInfo.m_prevBlockDigest = ColumnDataBlockHeader::kInitialPrevBlockDigest;
        stack.push(blockInfo);
    }

    while (!stack.empty()) {
        // Get next block
        blockInfo = stack.top();
        stack.pop();

        while (true) {
            // Load block
            //DBG_LOG_DEBUG("Column::checkDataConsistency(): "
            //              << getDisplayName() << ": Checking block " << blockInfo.m_currentBlockId);
            const auto currentBlock = getExistingBlock(blockInfo.m_currentBlockId);

            // Ensure previous block ID saved in block is correct
            if (currentBlock->getPrevBlockId() != blockInfo.m_prevBlockId) {
                throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockConsistencyMismatch,
                        getDatabaseName(), m_table.getName(), m_name, blockInfo.m_currentBlockId,
                        getDatabaseUuid(), m_table.getId(), m_id, "previous block ID mismatch");
            }

            // We can check only closed blocks
            if (currentBlock->getState() != ColumnDataBlockState::kClosed) break;

            // Check block digest based on data in block
            ColumnDataBlockHeader::Digest currentBlockDigest;
            currentBlock->computeDigest(blockInfo.m_prevBlockDigest, currentBlockDigest);
            if (currentBlock->getDigest() == currentBlockDigest) {
                LOG_DEBUG << "block digest mismatch";
                throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockConsistencyMismatch,
                        getDatabaseName(), m_table.getName(), getName(), blockInfo.m_currentBlockId,
                        getDatabaseUuid(), m_table.getId(), m_id, "block digest mismatch");
            }

            // Collect block into available block list, if it has enough free space
            if (currentBlock->getFreeDataSpace() >= m_minRequiredBlockFreeSpaces[m_dataType]) {
                m_availableDataBlocks.emplace(
                        currentBlock->getId(), currentBlock->getFreeDataSpace());
            }

            // Determine next blocks
            const auto nextBlockIds = m_blockRegistry.getNextBlockIds(blockInfo.m_currentBlockId);
            if (nextBlockIds.empty()) break;
            blockInfo.m_prevBlockId = blockInfo.m_currentBlockId;
            blockInfo.m_prevBlockDigest = currentBlockDigest;
            if (nextBlockIds.size() == 1) {
                blockInfo.m_currentBlockId = nextBlockIds.front();
                continue;
            }
            for (auto rit = nextBlockIds.rbegin(); rit != nextBlockIds.rend(); ++rit) {
                blockInfo.m_currentBlockId = *rit;
                stack.push(blockInfo);
            }
            break;
        }
    }
}

ColumnDefinitionPtr Column::createColumnDefinitionUnlocked()
{
    auto columnDefinition = std::make_shared<ColumnDefinition>(*this);
    m_columnDefinitionCache.emplace(columnDefinition->getId(), columnDefinition);
    getDatabase().registerColumnDefinition(*columnDefinition);
    return columnDefinition;
}

ColumnDefinitionPtr Column::createColumnDefinitionUnlocked(
        const ColumnDefinitionRecord& columnDefinitionRecord)
{
    auto columnDefinition = std::make_shared<ColumnDefinition>(*this, columnDefinitionRecord);
    m_columnDefinitionCache.emplace(columnDefinition->getId(), columnDefinition);
    return columnDefinition;
}

ColumnDataBlockPtr Column::loadBlock(std::uint64_t blockId)
{
    std::lock_guard lock(m_mutex);
    auto block = m_blockCache.get(blockId).value_or(nullptr);
    if (!block) {
        block = std::make_shared<ColumnDataBlock>(*this, blockId);
        m_blockCache.emplace(block->getId(), block);
    }
    return block;
}

ColumnDataBlockPtr Column::selectAvailableBlock(std::size_t requiredLength)
{
    // If there are no available blocks, just create new one
    if (m_availableDataBlocks.empty()) {
        auto block = createBlock(0, ColumnDataBlockState::kCurrent);
        m_availableDataBlocks.emplace(block->getId(), block->getFreeDataSpace());
        return block;
    }

    // Try to find some block that has enough room
    std::pair<std::uint64_t, std::uint32_t> minFreeSpaceBlock = *m_availableDataBlocks.begin();
    for (const auto& block : m_availableDataBlocks) {
        if (block.second >= requiredLength) return loadBlock(block.first);
        if (minFreeSpaceBlock.second < block.second) minFreeSpaceBlock = block;
    }

    // Create new block chained to the block with minimum free space
    ColumnDataBlockPtr block = loadBlock(minFreeSpaceBlock.first);
    m_availableDataBlocks.erase(block->getId());
    block = createOrGetNextBlock(*block, requiredLength);
    return block;
}

void Column::updateAvailableBlock(const ColumnDataBlock& block)
{
    const auto freeSpace = block.getFreeDataSpace();
    const auto it = m_availableDataBlocks.find(block.getId());
    if (it == m_availableDataBlocks.end())
        m_availableDataBlocks.emplace(block.getId(), freeSpace);
    else
        it->second = freeSpace;
}

ColumnDataBlockPtr Column::createOrGetNextBlock(
        ColumnDataBlock& block, std::size_t requiredFreeSpace)
{
    // Validate requiredFreeSpace
    if (requiredFreeSpace == 0) throw std::invalid_argument("requiredFreeSpace is zero");
    if (requiredFreeSpace > m_dataBlockDataAreaSize)
        throw std::invalid_argument("requiredFreeSpace is too large");

    ColumnDataBlockPtr nextBlock;

    // Get existing next blocks
    const auto nextBlockIds = m_blockRegistry.getNextBlockIds(block.getId());
    if (!nextBlockIds.empty()) {
        // Iterare existing next blocks in the reverse order because there's higher probability
        // to get block with necessary free space this way.
        for (auto rit = nextBlockIds.rbegin(); rit != nextBlockIds.rend(); ++rit) {
            const auto nextBlockId = *rit;
            auto nextBlockCandidate = loadBlock(nextBlockId);
            if (!nextBlockCandidate) {
                throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist,
                        getDatabaseName(), m_table.getName(), m_name, nextBlockId,
                        getDatabaseUuid(), m_table.getId(), m_id);
            }
            const auto state = nextBlockCandidate->getState();
            if ((state == ColumnDataBlockState::kCurrent
                        || state == ColumnDataBlockState::kAvailable)
                    && nextBlockCandidate->getFreeDataSpace() >= requiredFreeSpace) {
                nextBlock = nextBlockCandidate;
                break;
            }
        }
    }

    if (!nextBlock) {
        // There are either no existing next blocks or no one of them has matched
        // So create new block
        nextBlock = createBlock(block.getId());
    }

    // Obtain previous block header
    ColumnDataBlockHeader::Digest prevBlockDigest;
    const auto prevBlockId = block.getPrevBlockId();
    if (prevBlockId == 0)
        prevBlockDigest = ColumnDataBlockHeader::kInitialPrevBlockDigest;
    else {
        auto prevBlock = m_blockCache.get(prevBlockId).value_or(nullptr);
        if (!prevBlock) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockNotAvailable,
                    getDatabaseName(), m_table.getName(), m_name, prevBlockId, getDatabaseUuid(),
                    m_table.getId(), m_id);
        }
        prevBlockDigest = prevBlock->getDigest();
    }

    block.finalize(prevBlockDigest);
    m_availableDataBlocks.erase(block.getId());
    updateAvailableBlock(*nextBlock);
    return nextBlock;
}

ColumnDataBlockPtr Column::getExistingBlock(std::uint64_t blockId)
{
    auto block = loadBlock(blockId);
    if (!block) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist, getDatabaseName(),
                m_table.getName(), m_name, blockId, getDatabaseUuid(), m_table.getId(), m_id);
    }
    return block;
}

std::uint64_t Column::findFirstBlock() const
{
    constexpr auto kColumnDataBlockFilePrefixLength = ct_strlen(ColumnDataBlock::kBlockFilePrefix);
    constexpr auto kColumnDataBlockFileExtensionLength = ct_strlen(kDataFileExtension);
    constexpr auto kColumnDataBlockFileStaticLength =
            kColumnDataBlockFilePrefixLength + kColumnDataBlockFileExtensionLength;

    // Find first available block
    auto firstBlockId = std::numeric_limits<std::uint64_t>::max();
    for (const auto& entry : fs::directory_iterator(m_dataDir)) {
        if (!fs::is_regular_file(entry)) continue;
        const auto fileName1 = entry.path().filename();
        const auto& fileName = fileName1.string();

        bool fileIgnored = true;
        if (fileName.length() > kColumnDataBlockFileStaticLength
                && std::strncmp(fileName.c_str(), ColumnDataBlock::kBlockFilePrefix,
                           kColumnDataBlockFilePrefixLength)
                           == 0
                && std::strcmp(fileName.c_str() + fileName.length()
                                       - kColumnDataBlockFileExtensionLength,
                           kDataFileExtension)
                           == 0) {
            const auto blockIdStr = fileName.substr(kColumnDataBlockFilePrefixLength,
                    fileName.length() - kColumnDataBlockFileStaticLength);
            std::uint64_t blockId;
            try {
                std::size_t idx = 0;
                blockId = std::stoull(blockIdStr, &idx, 10);
                if (blockIdStr.c_str()[idx] == '\0') {
                    fileIgnored = false;
                    firstBlockId = std::min(firstBlockId, blockId);
                }
            } catch (std::exception&) {
                // do nothing here
            }
        }
        if (fileIgnored && m_wellKnownIgnorableFiles.count(fileName) == 0) {
            LOG_WARNING << utils::format(
                    "Consistency check for column '%1%'.'%2%'.'%3%': file '%4%' ignored",
                    getDatabaseName(), m_table.getName(), m_name, fileName);
        }
    }

    if (firstBlockId == std::numeric_limits<std::uint64_t>::max()) firstBlockId = 0;
    return firstBlockId;
}

std::pair<ColumnDataAddress, ColumnDataAddress> Column::storeLob(
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
                header.serialize(headerBuffer);
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
                auto actuallyRead = lob.read(data, remainingToRead);
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

std::pair<ColumnDataAddress, ColumnDataAddress> Column::storeBuffer(
        const void* src, std::uint32_t length, ColumnDataBlockPtr block)
{
    // Remember initial address
    ColumnDataAddress result;

    // Initialize chunk counters
    std::uint32_t chunkId = 1;
    std::uint8_t headerBuffer[LobChunkHeader::kSerializedSize];
    std::unique_ptr<std::uint8_t> dataBuffer;
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
                header.serialize(headerBuffer);
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

void Column::loadText(const ColumnDataAddress& addr, Variant& value, bool lobStreamsMustHoldSource)
{
    auto block = getExistingBlock(addr.getBlockId());
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
    auto block = getExistingBlock(addr.getBlockId());
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

void Column::createMasterColumnMainIndex()
{
    // Create index
    auto indexName = composeMasterColumnMainIndexName();
    // NOTE: We specify here index ID = 0, and this will be replaced by actual index ID,
    // when index object is created.
    const IndexColumnSpecification indexColumnSpec(m_currentColumnDefinition, false);
    m_masterColumnData->m_mainIndex = std::make_shared<UInt64UniqueLinearIndex>(m_table,
            std::move(indexName), kMasterColumnNameMainIndexValueSize, indexColumnSpec,
            m_table.isSystemTable() ? kSystemTableDataFileDataAreaSize
                                    : kDefaultDataFileDataAreaSize,
            kMasterColumnMainIndexDescription);

    // Prepare index ID file content
    std::uint64_t indexId = 0;
    ::pbeEncodeUInt64(
            m_masterColumnData->m_mainIndex->getId(), reinterpret_cast<std::uint8_t*>(&indexId));

    // Write index ID file
    const auto mainIndexIdFilePath = utils::constructPath(m_dataDir, kMainIndexIdFile);
    FileDescriptorGuard fd(::open(mainIndexIdFilePath.c_str(),
            O_CREAT | O_RDWR | O_DSYNC | O_CLOEXEC | O_NOATIME, kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateMainIndexIdFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }
    if (writeExact(fd.getFd(), &indexId, sizeof(indexId), kIgnoreSignals) != sizeof(indexId)) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteMainIndexIdFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }
}

void Column::createMasterColumnConstraints()
{
    m_currentColumnDefinition->addConstraint(
            m_table.createConstraint(std::string(), m_table.getSystemNotNullConstraintDefinition(),
                    this, kMasterColumnNotNullConstraintDescription));
}

void Column::writeFullTridCounters(int fd, const TridCounters& data)
{
    if (::pwriteExact(fd, &data, sizeof(data), 0, kIgnoreSignals) != TridCounters::kDataSize) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteTridCounterFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }
}

std::string Column::composeMasterColumnMainIndexName() const
{
    std::ostringstream oss;
    oss << "$MCMI$" << m_table.getId() << '$' << m_id;
    return oss.str();
}

std::string Column::ensureDataDir(bool create) const
{
    // NOTE: use isMasterColumnName() here, since master column data may be N/A at the moment
    auto dataDir = utils::constructPath(m_table.getDataDir(),
            isMasterColumnName() ? kMasterColumnDataDirPrefix : kColumnDataDirPrefix, m_id);
    const auto initFlagFile = utils::constructPath(dataDir, kInitializationFlagFile);
    const auto initFlagFileExists = fs::exists(initFlagFile);
    if (create) {
        // Check initialization flag
        if (initFlagFileExists) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnAlreadyExists, getDatabaseName(),
                    m_table.getName(), m_name);
        }

        // Create data directory
        try {
            const fs::path dataDirPath(dataDir);
            if (fs::exists(dataDirPath)) fs::remove_all(dataDirPath);
            fs::create_directories(dataDirPath);
        } catch (fs::filesystem_error& ex) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateColumnDataDir, dataDir,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id, ex.code().value(), ex.code().message());
        }
    } else {
        // Check that data directory exists
        if (!boost::filesystem::exists(dataDir)) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnDataFolderDoesNotExist,
                    getDatabaseName(), m_table.getName(), m_name, dataDir);
        }

        // Check initialization flag
        if (!initFlagFileExists) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnInitFileDoesNotExist,
                    getDatabaseName(), m_table.getName(), m_name, initFlagFile);
        }
    }
    return dataDir;
}

void Column::createInitializationFlagFile() const
{
    const auto initFlagFile = utils::constructPath(m_dataDir, kInitializationFlagFile);
    std::ofstream ofs(initFlagFile);
    if (!ofs.is_open()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateColumnInitializationFlagFile,
                initFlagFile, getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                m_table.getId(), m_id, "create file failed");
    }
    ofs.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        ofs << std::time(nullptr) << std::flush;
    } catch (std::exception& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateColumnInitializationFlagFile,
                initFlagFile, getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                m_table.getId(), m_id, "write failed");
    }
}

int Column::compareEncodedTableRowId(const void* left, const void* right) noexcept
{
    std::uint64_t l = 0, r = 0;
    ::pbeDecodeUInt64(reinterpret_cast<const uint8_t*>(left), &l);
    ::pbeDecodeUInt64(reinterpret_cast<const uint8_t*>(right), &r);
    return (l == r) ? 0 : ((l < r) ? -1 : 1);
}

std::unique_ptr<Column::MasterColumnData> Column::maybeCreateMasterColumnData(
        bool create, std::uint64_t firstUserTrid)
{
    if (!isMasterColumnName()) return nullptr;
    return std::make_unique<MasterColumnData>(*this, create, firstUserTrid);
}

/////////////////// struct Column::MasterColumnData ///////////////////////////////////////////////

Column::MasterColumnData::MasterColumnData(
        Column& parent, bool createCounters, std::uint64_t firstUserTrid)
    : m_firstUserTrid(firstUserTrid)
    , m_file(createCounters ? parent.createTridCountersFile(firstUserTrid)
                            : parent.openTridCountersFile(),
              true, PROT_READ | PROT_WRITE, MAP_POPULATE, 0, sizeof(DatabaseMetadata))
    , m_tridCounters(reinterpret_cast<TridCounters*>(m_file.getMappingAddress()))
{
}

}  // namespace siodb::iomgr::dbengine
