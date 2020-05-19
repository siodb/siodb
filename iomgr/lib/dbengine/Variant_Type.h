// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstddef>

// STL headers
#include <algorithm>

namespace siodb::iomgr::dbengine {

/** Variant value type */
enum class VariantType {
    kNull = 0,
    kBool,
    kInt8,
    kUInt8,
    kInt16,
    kUInt16,
    kInt32,
    kUInt32,
    kInt64,
    kUInt64,
    kFloat,
    kDouble,
    kDate,  // NOT SUPPORTED YET
    kTime,  // NOT SUPPORTED YET
    kDateTime,  // NOT SUPPORTED YET
    kTimeWithTZ,  // NOT SUPPORTED YET
    kDateTimeWithTZ,  // NOT SUPPORTED YET
    kDateInterval,  // NOT SUPPORTED YET
    kTimeInterval,  // NOT SUPPORTED YET
    // All types below are non-primitive,
    kString,
    kBinary,
    kClob,
    kBlob,
    kMax
};

/** First non-primitive type */
constexpr VariantType kFirstNonPrimitiveVariantType = VariantType::kDate;

/** Number of variant value types */
constexpr std::size_t kVariantTypeCount = static_cast<std::size_t>(VariantType::kMax);

/**
 * Checks that given value type is null type.
 * @param valueType Value type.
 * @return true if valueType is null type, false otherwise.
 */
inline constexpr bool isNullType(VariantType valueType)
{
    return valueType == VariantType::kNull;
}

/**
 * Checks that given value type is boolean type.
 * @param valueType Value type.
 * @return true if valueType is boolean type, false otherwise.
 */
inline constexpr bool isBoolType(VariantType valueType)
{
    return valueType == VariantType::kBool;
}

/**
 * Checks that given value type is string type.
 * @param valueType Value type.
 * @return true if valueType is string type, false otherwise.
 */
inline constexpr bool isStringType(VariantType valueType)
{
    return valueType == VariantType::kString;
}

/**
 * Checks that given value type is binary type.
 * @param valueType Value type.
 * @return true if valueType is binary type, false otherwise.
 */
inline constexpr bool isBinaryType(VariantType valueType)
{
    return valueType == VariantType::kBinary;
}

/**
 * Checks that given value type is date/time type.
 * @param valueType Value type.
 * @return true if valueType is date/time type, false otherwise.
 */
inline constexpr bool isDateTimeType(VariantType valueType)
{
    return valueType == VariantType::kDateTime;
}

/**
 * Checks that given variant value type is numeric type.
 * @param valueType Value type.
 * @return true if valueType is numeric type, false otherwise.
 */
inline constexpr bool isNumericType(VariantType valueType)
{
    switch (valueType) {
        case VariantType::kInt8:
        case VariantType::kUInt8:
        case VariantType::kInt16:
        case VariantType::kUInt16:
        case VariantType::kInt32:
        case VariantType::kUInt32:
        case VariantType::kInt64:
        case VariantType::kUInt64:
        case VariantType::kFloat:
        case VariantType::kDouble: return true;
        default: return false;
    }
}

/**
 * Checks that given variant value type is integer type.
 * @param valueType Value type.
 * @return true if valueType is integer type, false otherwise.
 */
inline constexpr bool isIntegerType(VariantType valueType)
{
    switch (valueType) {
        case VariantType::kInt8:
        case VariantType::kUInt8:
        case VariantType::kInt16:
        case VariantType::kUInt16:
        case VariantType::kInt32:
        case VariantType::kUInt32:
        case VariantType::kInt64:
        case VariantType::kUInt64: return true;
        default: return false;
    }
}

/**
 * Checks that given variant value type is floating point type
 * @param valueType Value type.
 * @return true if valueType is floating point type, false otherwise.
 */
inline constexpr bool isFloatingPointType(VariantType valueType)
{
    switch (valueType) {
        case VariantType::kFloat:
        case VariantType::kDouble: return true;
        default: return false;
    }
}

/**
 * Checks that given variant value type is signed numeric type.
 * @param valueType Value type.
 * @return true if @ref valueType is signed numeric type, false otherwise.
 */
inline constexpr bool isSignedType(VariantType valueType)
{
    switch (valueType) {
        case VariantType::kInt8:
        case VariantType::kInt16:
        case VariantType::kInt32:
        case VariantType::kInt64:
        case VariantType::kFloat:
        case VariantType::kDouble: return true;
        default: return false;
    }
}

/**
 * Return signed value type for selected numeric value type.
 * @param valueType Value type. Assumes numeric value type.
 * @return Signed value type.
 */
inline constexpr VariantType getSignedType(VariantType type)
{
    switch (type) {
        case VariantType::kUInt8: return VariantType::kInt8;
        case VariantType::kUInt16: return VariantType::kInt16;
        case VariantType::kUInt32: return VariantType::kInt32;
        case VariantType::kUInt64: return VariantType::kInt64;
        default: return type;
    }
}

/**
 * Checks that given variant value type is unsigned integer type.
 * @param valueType Value type.
 * @return true if @ref valueType is unsigned integer type, false otherwise.
 */
inline constexpr bool isUIntType(VariantType valueType)
{
    switch (valueType) {
        case VariantType::kUInt8:
        case VariantType::kUInt16:
        case VariantType::kUInt32:
        case VariantType::kUInt64: return true;
        default: return false;
    }
}

/**
 * Return unsigned value type for selected numeric value type.
 * @param valueType Value type. Assumes integer value type.
 * @return Unigned value type.
 */
inline constexpr VariantType getUIntType(VariantType type)
{
    switch (type) {
        case VariantType::kInt8: return VariantType::kUInt8;
        case VariantType::kInt16: return VariantType::kUInt16;
        case VariantType::kInt32: return VariantType::kUInt32;
        case VariantType::kInt64: return VariantType::kUInt64;
        default: return type;
    }
}

/**
 * Returns numeric operation result type.
 * Assumes leftType and rightType are numeric types.
 * @param leftType Left value type.
 * @param rightType Right value type.
 * @return Result value type.
 */
inline constexpr VariantType getNumericResultType(VariantType leftType, VariantType rightType)
{
    return std::max(std::max(leftType, rightType), VariantType::kInt32);
}

/**
 * Returns textual name of the variant value type.
 * @param type Variant value type.
 * @return Textual name of the variant value type.
 * @throw std::range_error if type is out of range.
 */
const char* getVariantTypeName(VariantType type);

/**
 * Returns textual name of the variant value type.
 * @param type Variant value type.
 * @return Textual name of the variant value type.
 */
const char* getVariantTypeName2(VariantType type) noexcept;

}  // namespace siodb::iomgr::dbengine
