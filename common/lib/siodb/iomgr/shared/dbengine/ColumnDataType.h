// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Variant_Type.h"

// Common project headers
#include <siodb/common/proto/ColumnDataType.pb.h>

// STL headers
#include <string>

namespace siodb::iomgr::dbengine {

/**
 * Checks that given column data type is numeric type.
 * @param dataType Column data type.
 * @return true if column data type is numeric, false otherwise.
 */
inline constexpr bool isNumericType(ColumnDataType dataType)
{
    switch (dataType) {
        case COLUMN_DATA_TYPE_INT8:
        case COLUMN_DATA_TYPE_UINT8:
        case COLUMN_DATA_TYPE_INT16:
        case COLUMN_DATA_TYPE_UINT16:
        case COLUMN_DATA_TYPE_INT32:
        case COLUMN_DATA_TYPE_UINT32:
        case COLUMN_DATA_TYPE_INT64:
        case COLUMN_DATA_TYPE_UINT64:
        case COLUMN_DATA_TYPE_FLOAT:
        case COLUMN_DATA_TYPE_DOUBLE: return true;
        default: return false;
    }
}

/**
 * Checks that given column data type is integer type.
 * @param dataType Column data type.
 * @return true if column data type is integer, false otherwise.
 */
inline constexpr bool isIntegerType(ColumnDataType dataType)
{
    switch (dataType) {
        case COLUMN_DATA_TYPE_INT8:
        case COLUMN_DATA_TYPE_UINT8:
        case COLUMN_DATA_TYPE_INT16:
        case COLUMN_DATA_TYPE_UINT16:
        case COLUMN_DATA_TYPE_INT32:
        case COLUMN_DATA_TYPE_UINT32:
        case COLUMN_DATA_TYPE_INT64:
        case COLUMN_DATA_TYPE_UINT64: return true;
        default: return false;
    }
}

/**
 * Checks that given column data type is floating point type
 * @param dataType Column data type.
 * @return true if column data type is floating point, false otherwise.
 */
inline constexpr bool isFloatingPointType(ColumnDataType dataType)
{
    switch (dataType) {
        case COLUMN_DATA_TYPE_FLOAT:
        case COLUMN_DATA_TYPE_DOUBLE: return true;
        default: return false;
    }
}

/**
 * Checks that given column data type is signed numeric type.
 * @param dataType Column data type.
 * @return true if @ref dataType is signed numeric type, false otherwise.
 */
inline constexpr bool isSignedType(ColumnDataType dataType)
{
    switch (dataType) {
        case COLUMN_DATA_TYPE_INT8:
        case COLUMN_DATA_TYPE_INT16:
        case COLUMN_DATA_TYPE_INT32:
        case COLUMN_DATA_TYPE_INT64:
        case COLUMN_DATA_TYPE_FLOAT:
        case COLUMN_DATA_TYPE_DOUBLE: return true;
        default: return false;
    }
}

/**
 * Return signed column data for selected numeric data type.
 * @param dataType Column data type.
 * @return Signed column data type.
 */
inline constexpr ColumnDataType getSignedType(ColumnDataType dataType)
{
    switch (dataType) {
        case COLUMN_DATA_TYPE_UINT8: return COLUMN_DATA_TYPE_INT8;
        case COLUMN_DATA_TYPE_UINT16: return COLUMN_DATA_TYPE_INT16;
        case COLUMN_DATA_TYPE_UINT32: return COLUMN_DATA_TYPE_INT16;
        case COLUMN_DATA_TYPE_UINT64: return COLUMN_DATA_TYPE_INT16;
        default: return dataType;
    }
}

/**
 * Checks that given column data type is unsigned integer type.
 * @param dataType Column data type.
 * @return true if @ref dataType is unsigned integer type, false otherwise.
 */
inline constexpr bool isUIntType(ColumnDataType dataType)
{
    switch (dataType) {
        case COLUMN_DATA_TYPE_UINT8:
        case COLUMN_DATA_TYPE_UINT16:
        case COLUMN_DATA_TYPE_UINT32:
        case COLUMN_DATA_TYPE_UINT64: return true;
        default: return false;
    }
}

/**
 * Return unsigned column data for selected integer data type.
 * @param dataType Column data type.
 * @return Unsigned column data type.
 */
inline constexpr ColumnDataType getUIntType(ColumnDataType dataType)
{
    switch (dataType) {
        case COLUMN_DATA_TYPE_INT8: return COLUMN_DATA_TYPE_UINT8;
        case COLUMN_DATA_TYPE_INT16: return COLUMN_DATA_TYPE_UINT16;
        case COLUMN_DATA_TYPE_INT32: return COLUMN_DATA_TYPE_UINT16;
        case COLUMN_DATA_TYPE_INT64: return COLUMN_DATA_TYPE_UINT16;
        default: return dataType;
    }
}

/**
 * Returns numeric operation result type.
 * @param leftType Left value type.
 * @param rightType Right value type.
 * @return Result value type.
 */
inline constexpr ColumnDataType getNumericResultType(
        ColumnDataType leftType, ColumnDataType rightType)
{
    return std::max(std::max(leftType, rightType), COLUMN_DATA_TYPE_INT32);
}

/**
 * Converts variant value type to an appropriate column data type.
 * @param type Variant value type.
 * @return Corresponding column data type.
 */
ColumnDataType convertVariantTypeToColumnDataType(VariantType type);

/**
 * Converts column data type to an appropriate variant value type.
 * @param type Column data type.
 * @return Corresponding column variant value type.
 *         NOTE: BINARY and TEXT are alaways converted to Binary and String.
 */
VariantType convertColumnDataTypeToVariantType(ColumnDataType type);

/**
 * Returns textual name of the column data type.
 * @param dataType Column data type.
 * @return Textual name of the column data type.
 */
const char* getColumnDataTypeName(ColumnDataType dataType);

/**
 * Returns column data type for the textual name.
 * @param dataTypeName Column data type name.
 * @return Column data type that matches textual name.
 * @throw std::invalid_argument if @ref dataTypeName doesn't match to any column data type.
 */
ColumnDataType getColumnDataTypeByName(const std::string& dataTypeName);

}  // namespace siodb::iomgr::dbengine
