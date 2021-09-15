// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RowDecoder.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_ext/bitmask.h>
#include <siodb/common/stl_ext/sstream_ext.h>
#include <siodb/common/utils/Base128VariantEncoding.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// Boost headers
#include <boost/endian/conversion.hpp>

namespace siodb::iomgr::dbengine::util {

namespace {

[[noreturn]] void notEnoughData1(std::size_t requiredLength, std::size_t availableLength,
        std::size_t columnIndex, ColumnDataType dataType)
{
    throw std::invalid_argument(stdext::concat("Not enough data (need ", requiredLength,
            " bytes, but only only ", availableLength, " bytes available) at column index ",
            columnIndex, ", data type ", static_cast<int>(dataType)));
}

[[noreturn]] void notEnoughData2(
        std::size_t availableLength, std::size_t columnIndex, ColumnDataType dataType)
{
    throw std::invalid_argument(stdext::concat("Not enough data (only ", availableLength,
            " bytes available) at column index ", columnIndex, ", data type ",
            static_cast<int>(dataType)));
}

[[noreturn]] void dataCorruption(std::size_t columnIndex, ColumnDataType dataType)
{
    throw std::invalid_argument(stdext::concat("Data corrupted at column index ", columnIndex,
            ", data type ", static_cast<int>(dataType)));
}

[[noreturn]] void unsupportedDataType(std::size_t columnIndex, ColumnDataType dataType)
{
    throw std::invalid_argument(stdext::concat("Unsupported data type ", static_cast<int>(dataType),
            " at column index ", columnIndex));
}

}  // anonymous namespace

std::vector<Variant> decodeRow(const std::uint8_t* buffer, std::size_t length,
        std::size_t totalColumnCount, std::size_t columnsToDecodeCount,
        const ColumnDataType* dataTypes, bool hasNullableColumns)
{
    if (totalColumnCount == 0 || columnsToDecodeCount == 0) std::vector<Variant>();

    stdext::bitmask nullBitmask;
    if (hasNullableColumns) {
        nullBitmask.resize(totalColumnCount);
        const auto n = nullBitmask.size();
        if (length < n) {
            throw std::invalid_argument(stdext::concat("Not enough data for null bitmask (", n,
                    " required, but only ", length, " bytes available)"));
        }
        std::copy_n(buffer, n, nullBitmask.data());
        buffer += n;
        length -= n;
    }

    std::vector<Variant> result;
    result.reserve(columnsToDecodeCount);
    for (std::size_t i = 0; i < columnsToDecodeCount; ++i, ++dataTypes) {
        const auto dataType = *dataTypes;
        if (dataType < 0 || dataType >= COLUMN_DATA_TYPE_MAX) {
            throw std::invalid_argument(stdext::concat(
                    "Invalid data type ", static_cast<int>(dataType), " at column index ", i));
        }

        if (hasNullableColumns && nullBitmask.get(i)) {
            result.emplace_back();
            continue;
        }

        switch (dataType) {
            case COLUMN_DATA_TYPE_BOOL: {
                constexpr std::size_t kRequiredLength = 1;
                if (length < kRequiredLength) notEnoughData1(kRequiredLength, length, i, dataType);
                result.emplace_back(*buffer != 0);
                ++buffer;
                --length;
                break;
            }

            case COLUMN_DATA_TYPE_INT8: {
                constexpr std::size_t kRequiredLength = 1;
                if (length < kRequiredLength) notEnoughData1(kRequiredLength, length, i, dataType);
                result.emplace_back(*reinterpret_cast<const std::int8_t*>(buffer));
                ++buffer;
                --length;
                break;
            }

            case COLUMN_DATA_TYPE_UINT8: {
                constexpr std::size_t kRequiredLength = 1;
                if (length < kRequiredLength) notEnoughData1(kRequiredLength, length, i, dataType);
                result.emplace_back(*buffer);
                ++buffer;
                --length;
                break;
            }

            case COLUMN_DATA_TYPE_INT16: {
                constexpr std::size_t kRequiredLength = 2;
                if (length < kRequiredLength) notEnoughData1(kRequiredLength, length, i, dataType);
                std::int16_t value = 0;
                buffer = pbeDecodeInt16(buffer, &value);
                length -= kRequiredLength;
                value = boost::endian::little_to_native(value);
                result.emplace_back(value);
                break;
            }

            case COLUMN_DATA_TYPE_UINT16: {
                constexpr std::size_t kRequiredLength = 2;
                if (length < kRequiredLength) notEnoughData1(kRequiredLength, length, i, dataType);
                std::uint16_t value = 0;
                buffer = pbeDecodeUInt16(buffer, &value);
                length -= kRequiredLength;
                value = boost::endian::little_to_native(value);
                result.emplace_back(value);
                break;
            }

            case COLUMN_DATA_TYPE_INT32: {
                std::int32_t value = 0;
                const int consumed =
                        decodeVarUInt32(buffer, length, reinterpret_cast<std::uint32_t*>(&value));
                if (consumed == 0) notEnoughData2(length, i, dataType);
                if (consumed == -1) dataCorruption(i, dataType);
                buffer += consumed;
                length -= consumed;
                result.emplace_back(value);
                break;
            }

            case COLUMN_DATA_TYPE_UINT32: {
                std::uint32_t value = 0;
                const int consumed = decodeVarUInt32(buffer, length, &value);
                if (consumed == 0) notEnoughData2(length, i, dataType);
                if (consumed == -1) dataCorruption(i, dataType);
                buffer += consumed;
                length -= consumed;
                result.emplace_back(value);
                break;
            }

            case COLUMN_DATA_TYPE_INT64: {
                std::int64_t value = 0;
                const int consumed =
                        decodeVarUInt64(buffer, length, reinterpret_cast<std::uint64_t*>(&value));
                if (consumed == 0) notEnoughData2(length, i, dataType);
                if (consumed == -1) dataCorruption(i, dataType);
                buffer += consumed;
                length -= consumed;
                result.emplace_back(value);
                break;
            }

            case COLUMN_DATA_TYPE_UINT64: {
                std::uint64_t value = 0;
                const int consumed = decodeVarUInt64(buffer, length, &value);
                if (consumed == 0) notEnoughData2(length, i, dataType);
                if (consumed == -1) dataCorruption(i, dataType);
                buffer += consumed;
                length -= consumed;
                result.emplace_back(value);
                break;
            }

            case COLUMN_DATA_TYPE_FLOAT: {
                constexpr std::size_t kRequiredLength = 4;
                if (length < kRequiredLength) notEnoughData1(kRequiredLength, length, i, dataType);
                float value = 0;
                buffer = pbeDecodeUInt32LE(buffer, reinterpret_cast<std::uint32_t*>(&value));
                length -= kRequiredLength;
                result.emplace_back(value);
                break;
            }

            case COLUMN_DATA_TYPE_DOUBLE: {
                constexpr std::size_t kRequiredLength = 8;
                if (length < kRequiredLength) notEnoughData1(kRequiredLength, length, i, dataType);
                double value = 0;
                buffer = pbeDecodeUInt64LE(buffer, reinterpret_cast<std::uint64_t*>(&value));
                length -= kRequiredLength;
                result.emplace_back(value);
                break;
            }

            case COLUMN_DATA_TYPE_TEXT: {
                // Read length
                std::uint32_t clobLength = 0;
                const int consumed = decodeVarUInt32(buffer, length, &clobLength);
                if (consumed == 0) notEnoughData2(length, i, dataType);
                if (consumed == -1) dataCorruption(i, dataType);
                buffer += consumed;
                length -= consumed;
                if (clobLength == 0) {
                    result.emplace_back(std::string());
                    break;
                }
                // Read text
                if (clobLength > length) notEnoughData1(clobLength, length, i, dataType);
                std::string value;
                value.append(reinterpret_cast<const char*>(buffer), clobLength);
                buffer += clobLength;
                length -= clobLength;
                result.emplace_back(std::move(value));
                break;
            }

            case COLUMN_DATA_TYPE_BINARY: {
                // Read length
                std::uint32_t blobLength = 0;
                const int consumed = decodeVarUInt32(buffer, length, &blobLength);
                if (consumed == 0) notEnoughData2(length, i, dataType);
                if (consumed == -1) dataCorruption(i, dataType);
                buffer += consumed;
                length -= consumed;
                if (blobLength == 0) {
                    result.emplace_back(BinaryValue());
                    break;
                }
                // Read data
                if (blobLength > length) notEnoughData1(blobLength, length, i, dataType);
                BinaryValue value(blobLength);
                std::copy_n(buffer, blobLength, value.data());
                buffer += blobLength;
                length -= blobLength;
                result.emplace_back(std::move(value));
                break;
            }

            case COLUMN_DATA_TYPE_TIMESTAMP: {
                if (length < RawDateTime::kDatePartSerializedSize)
                    notEnoughData1(RawDateTime::kDatePartSerializedSize, length, i, dataType);
                RawDateTime value;
                value.deserializeDatePart(buffer);
                length -= RawDateTime::kDatePartSerializedSize;
                if (value.m_datePart.m_hasTimePart) {
                    if (length < RawDateTime::kTimePartSerializedSize)
                        notEnoughData1(RawDateTime::kTimePartSerializedSize, length, i, dataType);
                    value.deserialize(buffer, RawDateTime::kSerializedSize);
                    length -= RawDateTime::kTimePartSerializedSize;
                    buffer += RawDateTime::kSerializedSize;
                } else {
                    buffer += RawDateTime::kDatePartSerializedSize;
                }
                result.emplace_back(value);
                break;
            }

            default: unsupportedDataType(i, dataType);
        }
#if 0
        LOG_INFO << "decodeRow: column #" << i << " [" << static_cast<int>(dataType)
                 << "]: " << *result.back().asString();
#endif
    }
    return result;
}

}  // namespace siodb::iomgr::dbengine::util
