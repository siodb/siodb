// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDataType.h"

// Common project headers
#include <siodb/common/stl_ext/algorithm_ext.h>

// STL headers
#include <array>
#include <unordered_map>

namespace siodb::iomgr::dbengine {

namespace {

/** Textual names of the Siodb column data types */
const std::array<const char*, COLUMN_DATA_TYPE_MAX> kColumnDataTypeNames {
        "BOOLEAN",
        "INT8",
        "UINT8",
        "INT16",
        "UINT16",
        "INT32",
        "UINT32",
        "INT64",
        "UINT64",
        "FLOAT",
        "DOUBLE",
        "TEXT",
        "NTEXT",
        "BINARY",
        "DATE",
        "TIME",
        "TIME WITH TIME ZONE",
        "TIMESTAMP",
        "TIMESTAMP WITH TIME ZONE",
        "DATE INTERVAL",
        "TIME INTERVAL",
        "STRUCT",
        "XML",
        "JSON",
        "UUID",
};

/** Map of column data type names */
const std::unordered_map<std::string, int> kColumnDataTypesByName =
        stdext::range_to_value_map<std::unordered_map<std::string, int>>(
                kColumnDataTypeNames.cbegin(), kColumnDataTypeNames.cend(), 0);

/** Mapping from variant value type to column data type */
const std::array<ColumnDataType, kVariantTypeCount> kVariantTypeToColumnDataTypeMapping {
        COLUMN_DATA_TYPE_UNKNOWN,  // kNull
        COLUMN_DATA_TYPE_BOOL,  // kBool
        COLUMN_DATA_TYPE_INT8,  // kInt8
        COLUMN_DATA_TYPE_UINT8,  // kUInt8
        COLUMN_DATA_TYPE_INT16,  // kInt16
        COLUMN_DATA_TYPE_UINT16,  // kUInt16
        COLUMN_DATA_TYPE_INT32,  // kInt32
        COLUMN_DATA_TYPE_UINT32,  // kUInt32
        COLUMN_DATA_TYPE_INT64,  // kInt64
        COLUMN_DATA_TYPE_UINT64,  // kUInt64
        COLUMN_DATA_TYPE_FLOAT,  // kFloat
        COLUMN_DATA_TYPE_DOUBLE,  // kDouble
        COLUMN_DATA_TYPE_DATE,  // kDate
        COLUMN_DATA_TYPE_TIME,  // kTime
        COLUMN_DATA_TYPE_TIMESTAMP,  // kDateTime
        COLUMN_DATA_TYPE_TIME_WITH_TZ,  // kTimeWithTZ
        COLUMN_DATA_TYPE_TIMESTAMP_WITH_TZ,  // kDateTimeWithTZ
        COLUMN_DATA_TYPE_DATE_INTERVAL,  // kDateInterval
        COLUMN_DATA_TYPE_TIME_INTERVAL,  // kTimeInterval
        COLUMN_DATA_TYPE_TEXT,  // kString
        COLUMN_DATA_TYPE_BINARY,  // kBinary
        COLUMN_DATA_TYPE_TEXT,  // kClob
        COLUMN_DATA_TYPE_BINARY,  // kBlob
};

/** Mapping from column data type to variant value type */
const std::unordered_map<ColumnDataType, VariantType> kColumnDataTypeToVariantTypeMapping {
        {COLUMN_DATA_TYPE_UNKNOWN, VariantType::kNull}, {COLUMN_DATA_TYPE_BOOL, VariantType::kBool},
        {COLUMN_DATA_TYPE_INT8, VariantType::kInt8}, {COLUMN_DATA_TYPE_UINT8, VariantType::kUInt8},
        {COLUMN_DATA_TYPE_INT16, VariantType::kInt16},
        {COLUMN_DATA_TYPE_UINT16, VariantType::kUInt16},
        {COLUMN_DATA_TYPE_INT32, VariantType::kInt32},
        {COLUMN_DATA_TYPE_UINT32, VariantType::kUInt32},
        {COLUMN_DATA_TYPE_INT64, VariantType::kInt64},
        {COLUMN_DATA_TYPE_UINT64, VariantType::kUInt64},
        {COLUMN_DATA_TYPE_FLOAT, VariantType::kFloat},
        {COLUMN_DATA_TYPE_DOUBLE, VariantType::kDouble},
        {COLUMN_DATA_TYPE_TEXT, VariantType::kString},
        {COLUMN_DATA_TYPE_BINARY, VariantType::kBinary},
        {COLUMN_DATA_TYPE_TIMESTAMP, VariantType::kDateTime},
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

}  // namespace

ColumnDataType convertVariantTypeToColumnDataType(VariantType type)
{
    return kVariantTypeToColumnDataTypeMapping.at(static_cast<std::size_t>(type));
}

VariantType convertColumnDataTypeToVariantType(ColumnDataType type)
{
    const auto it = kColumnDataTypeToVariantTypeMapping.find(type);
    if (it != kColumnDataTypeToVariantTypeMapping.end()) return it->second;
    throw std::invalid_argument(
            "Column data type #" + std::to_string(static_cast<int>(type)) + " is not supported");
}

const char* getColumnDataTypeName(ColumnDataType dataType)
{
    return kColumnDataTypeNames.at(dataType);
}

ColumnDataType getColumnDataTypeByName(const std::string& dataTypeName)
{
    const auto it = kColumnDataTypesByName.find(dataTypeName);
    if (it != kColumnDataTypesByName.cend()) return static_cast<ColumnDataType>(it->second);
    throw std::invalid_argument("Invalid column data type name");
}

}  // namespace siodb::iomgr::dbengine
