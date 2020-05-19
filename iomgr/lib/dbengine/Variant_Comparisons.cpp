// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant.h"

// Project headers
#include <siodb/common/utils/Utf8String.h>

namespace siodb::iomgr::dbengine {

bool Variant::operator==(const Variant& other) const noexcept
{
    if (SIODB_UNLIKELY(&other == this)) return true;
    if (m_valueType != other.m_valueType) return false;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool == other.m_value.m_bool;
        case VariantType::kInt8: return m_value.m_i8 == other.m_value.m_i8;
        case VariantType::kUInt8: return m_value.m_ui8 == other.m_value.m_ui8;
        case VariantType::kInt16: return m_value.m_i16 == other.m_value.m_i16;
        case VariantType::kUInt16: return m_value.m_ui16 == other.m_value.m_ui16;
        case VariantType::kInt32: return m_value.m_i32 == other.m_value.m_i32;
        case VariantType::kUInt32: return m_value.m_ui32 == other.m_value.m_ui32;
        case VariantType::kInt64: return m_value.m_i64 == other.m_value.m_i64;
        case VariantType::kUInt64: return m_value.m_ui64 == other.m_value.m_ui64;
        case VariantType::kFloat: return m_value.m_float == other.m_value.m_float;
        case VariantType::kDouble: return m_value.m_double == other.m_value.m_double;
        case VariantType::kString: return *m_value.m_string == *other.m_value.m_string;
        case VariantType::kBinary: return *m_value.m_binary == *other.m_value.m_binary;
        case VariantType::kClob: return false;
        case VariantType::kBlob: return false;
        default: return true;
    }
}

bool Variant::operator!=(const Variant& other) const noexcept
{
    if (SIODB_UNLIKELY(&other == this)) return false;
    if (m_valueType != other.m_valueType) return true;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool != other.m_value.m_bool;
        case VariantType::kInt8: return m_value.m_i8 != other.m_value.m_i8;
        case VariantType::kUInt8: return m_value.m_ui8 != other.m_value.m_ui8;
        case VariantType::kInt16: return m_value.m_i16 != other.m_value.m_i16;
        case VariantType::kUInt16: return m_value.m_ui16 != other.m_value.m_ui16;
        case VariantType::kInt32: return m_value.m_i32 != other.m_value.m_i32;
        case VariantType::kUInt32: return m_value.m_ui32 != other.m_value.m_ui32;
        case VariantType::kInt64: return m_value.m_i64 != other.m_value.m_i64;
        case VariantType::kUInt64: return m_value.m_ui64 != other.m_value.m_ui64;
        case VariantType::kFloat: return m_value.m_float != other.m_value.m_float;
        case VariantType::kDouble: return m_value.m_double != other.m_value.m_double;
        case VariantType::kString: return *m_value.m_string != *other.m_value.m_string;
        case VariantType::kBinary: return *m_value.m_binary != *other.m_value.m_binary;
        case VariantType::kClob: return true;
        case VariantType::kBlob: return true;
        default: return false;
    }
}

bool Variant::operator<(const Variant& other) const noexcept
{
    if (SIODB_UNLIKELY(&other == this)) return false;
    if (m_valueType != other.m_valueType) return m_valueType < other.m_valueType;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool < other.m_value.m_bool;
        case VariantType::kInt8: return m_value.m_i8 < other.m_value.m_i8;
        case VariantType::kUInt8: return m_value.m_ui8 < other.m_value.m_ui8;
        case VariantType::kInt16: return m_value.m_i16 < other.m_value.m_i16;
        case VariantType::kUInt16: return m_value.m_ui16 < other.m_value.m_ui16;
        case VariantType::kInt32: return m_value.m_i32 < other.m_value.m_i32;
        case VariantType::kUInt32: return m_value.m_ui32 < other.m_value.m_ui32;
        case VariantType::kInt64: return m_value.m_i64 < other.m_value.m_i64;
        case VariantType::kUInt64: return m_value.m_ui64 < other.m_value.m_ui64;
        case VariantType::kFloat: return m_value.m_float < other.m_value.m_float;
        case VariantType::kDouble: return m_value.m_double < other.m_value.m_double;
        case VariantType::kString: return *m_value.m_string < *other.m_value.m_string;
        case VariantType::kBinary: return *m_value.m_binary < *other.m_value.m_binary;
        case VariantType::kClob: {
            const auto size = m_value.m_clob->getSize();
            const auto otherSize = other.m_value.m_clob->getSize();
            return (size == otherSize) ? m_value.m_clob < other.m_value.m_clob : size < otherSize;
        }
        case VariantType::kBlob: {
            const auto size = m_value.m_blob->getSize();
            const auto otherSize = other.m_value.m_blob->getSize();
            return (size == otherSize) ? m_value.m_blob < other.m_value.m_blob : size < otherSize;
        }
        default: return false;
    }
}

bool Variant::operator<=(const Variant& other) const noexcept
{
    if (SIODB_UNLIKELY(&other == this)) return true;
    if (m_valueType != other.m_valueType) return m_valueType < other.m_valueType;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool <= other.m_value.m_bool;
        case VariantType::kInt8: return m_value.m_i8 <= other.m_value.m_i8;
        case VariantType::kUInt8: return m_value.m_ui8 <= other.m_value.m_ui8;
        case VariantType::kInt16: return m_value.m_i16 <= other.m_value.m_i16;
        case VariantType::kUInt16: return m_value.m_ui16 <= other.m_value.m_ui16;
        case VariantType::kInt32: return m_value.m_i32 <= other.m_value.m_i32;
        case VariantType::kUInt32: return m_value.m_ui32 <= other.m_value.m_ui32;
        case VariantType::kInt64: return m_value.m_i64 <= other.m_value.m_i64;
        case VariantType::kUInt64: return m_value.m_ui64 <= other.m_value.m_ui64;
        case VariantType::kFloat: return m_value.m_float <= other.m_value.m_float;
        case VariantType::kDouble: return m_value.m_double <= other.m_value.m_double;
        case VariantType::kString: return *m_value.m_string <= *other.m_value.m_string;
        case VariantType::kBinary: return *m_value.m_binary <= *other.m_value.m_binary;
        case VariantType::kClob: {
            const auto size = m_value.m_clob->getSize();
            const auto otherSize = other.m_value.m_clob->getSize();
            return (size == otherSize) ? m_value.m_clob < other.m_value.m_clob : size < otherSize;
        }
        case VariantType::kBlob: {
            const auto size = m_value.m_blob->getSize();
            const auto otherSize = other.m_value.m_blob->getSize();
            return (size == otherSize) ? m_value.m_blob < other.m_value.m_blob : size < otherSize;
        }
        default: return false;
    }
}

bool Variant::operator>(const Variant& other) const noexcept
{
    if (SIODB_UNLIKELY(&other == this)) return false;
    if (m_valueType != other.m_valueType) return m_valueType > other.m_valueType;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool > other.m_value.m_bool;
        case VariantType::kInt8: return m_value.m_i8 > other.m_value.m_i8;
        case VariantType::kUInt8: return m_value.m_ui8 > other.m_value.m_ui8;
        case VariantType::kInt16: return m_value.m_i16 > other.m_value.m_i16;
        case VariantType::kUInt16: return m_value.m_ui16 > other.m_value.m_ui16;
        case VariantType::kInt32: return m_value.m_i32 > other.m_value.m_i32;
        case VariantType::kUInt32: return m_value.m_ui32 > other.m_value.m_ui32;
        case VariantType::kInt64: return m_value.m_i64 > other.m_value.m_i64;
        case VariantType::kUInt64: return m_value.m_ui64 > other.m_value.m_ui64;
        case VariantType::kFloat: return m_value.m_float > other.m_value.m_float;
        case VariantType::kDouble: return m_value.m_double > other.m_value.m_double;
        case VariantType::kString: return *m_value.m_string > *other.m_value.m_string;
        case VariantType::kBinary: return *m_value.m_binary > *other.m_value.m_binary;
        case VariantType::kClob: {
            const auto size = m_value.m_clob->getSize();
            const auto otherSize = other.m_value.m_clob->getSize();
            return (size == otherSize) ? m_value.m_clob > other.m_value.m_clob : size > otherSize;
        }
        case VariantType::kBlob: {
            const auto size = m_value.m_blob->getSize();
            const auto otherSize = other.m_value.m_blob->getSize();
            return (size == otherSize) ? m_value.m_blob > other.m_value.m_blob : size > otherSize;
        }
        default: return false;
    }
}

bool Variant::operator>=(const Variant& other) const noexcept
{
    if (SIODB_UNLIKELY(&other == this)) return true;
    if (m_valueType != other.m_valueType) return m_valueType > other.m_valueType;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool >= other.m_value.m_bool;
        case VariantType::kInt8: return m_value.m_i8 >= other.m_value.m_i8;
        case VariantType::kUInt8: return m_value.m_ui8 >= other.m_value.m_ui8;
        case VariantType::kInt16: return m_value.m_i16 >= other.m_value.m_i16;
        case VariantType::kUInt16: return m_value.m_ui16 >= other.m_value.m_ui16;
        case VariantType::kInt32: return m_value.m_i32 >= other.m_value.m_i32;
        case VariantType::kUInt32: return m_value.m_ui32 >= other.m_value.m_ui32;
        case VariantType::kInt64: return m_value.m_i64 >= other.m_value.m_i64;
        case VariantType::kUInt64: return m_value.m_ui64 >= other.m_value.m_ui64;
        case VariantType::kFloat: return m_value.m_float >= other.m_value.m_float;
        case VariantType::kDouble: return m_value.m_double >= other.m_value.m_double;
        case VariantType::kString: return *m_value.m_string >= *other.m_value.m_string;
        case VariantType::kBinary: return *m_value.m_binary >= *other.m_value.m_binary;
        case VariantType::kClob: {
            const auto size = m_value.m_clob->getSize();
            const auto otherSize = other.m_value.m_clob->getSize();
            return (size == otherSize) ? m_value.m_clob > other.m_value.m_clob : size > otherSize;
        }
        case VariantType::kBlob: {
            const auto size = m_value.m_blob->getSize();
            const auto otherSize = other.m_value.m_blob->getSize();
            return (size == otherSize) ? m_value.m_blob > other.m_value.m_blob : size > otherSize;
        }
        default: return false;
    }
}

bool Variant::compatibleEqual(const Variant& other) const
{
    if (SIODB_UNLIKELY(&other == this)) return true;
    if (m_valueType == VariantType::kNull) return false;
    switch (m_valueType) {
        case VariantType::kNull: return false;
        case VariantType::kBool: {
            if (other.isBool()) {
                return m_value.m_bool == other.m_value.m_bool;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() == other.getInt8();
                case VariantType::kUInt8: return getInt8() == other.getUInt8();
                case VariantType::kInt16: return getInt8() == other.getInt16();
                case VariantType::kUInt16: return getInt8() == other.getUInt16();
                case VariantType::kInt32: return getInt8() == other.getInt32();
                case VariantType::kUInt32:
                    return getInt8() == static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt8() == other.getInt64();
                case VariantType::kUInt64:
                    return getInt8() == static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt8() == other.getFloat();
                case VariantType::kDouble: return getInt8() == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() == other.getInt8();
                case VariantType::kUInt8: return getUInt8() == other.getUInt8();
                case VariantType::kInt16: return getUInt8() == other.getInt16();
                case VariantType::kUInt16: return getUInt8() == other.getUInt16();
                case VariantType::kInt32: return getUInt8() == other.getInt32();
                case VariantType::kUInt32: return getUInt8() == other.getUInt32();
                case VariantType::kInt64: return getUInt8() == other.getInt64();
                case VariantType::kUInt64: return getUInt8() == other.getUInt64();
                case VariantType::kFloat: return getUInt8() == other.getFloat();
                case VariantType::kDouble: return getUInt8() == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() == other.getInt8();
                case VariantType::kUInt8: return getInt16() == other.getUInt8();
                case VariantType::kInt16: return getInt16() == other.getInt16();
                case VariantType::kUInt16: return getInt16() == other.getUInt16();
                case VariantType::kInt32: return getInt16() == other.getInt32();
                case VariantType::kUInt32:
                    return getInt16() == static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt16() == other.getInt64();
                case VariantType::kUInt64:
                    return getInt16() == static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt16() == other.getFloat();
                case VariantType::kDouble: return getInt16() == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() == other.getInt8();
                case VariantType::kUInt8: return getUInt16() == other.getUInt8();
                case VariantType::kInt16: return getUInt16() == other.getInt16();
                case VariantType::kUInt16: return getUInt16() == other.getUInt16();
                case VariantType::kInt32: return getUInt16() == other.getInt32();
                case VariantType::kUInt32: return getUInt16() == other.getUInt32();
                case VariantType::kInt64: return getUInt16() == other.getInt64();
                case VariantType::kUInt64: return getUInt16() == other.getUInt64();
                case VariantType::kFloat: return getUInt16() == other.getFloat();
                case VariantType::kDouble: return getUInt16() == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() == other.getInt8();
                case VariantType::kUInt8: return getInt32() == other.getUInt8();
                case VariantType::kInt16: return getInt32() == other.getInt16();
                case VariantType::kUInt16: return getInt32() == other.getUInt16();
                case VariantType::kInt32: return getInt32() == other.getInt32();
                case VariantType::kUInt32:
                    return getInt32() == static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt32() == other.getInt64();
                case VariantType::kUInt64:
                    return getInt32() == static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat:
                    return getInt32() == static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt32() == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int32_t>(getUInt32()) == other.getInt8();
                case VariantType::kUInt8: return getUInt32() == other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int32_t>(getUInt32()) == other.getInt16();
                case VariantType::kUInt16: return getUInt32() == other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int32_t>(getUInt32()) == other.getInt32();
                case VariantType::kUInt32: return getUInt32() == other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int32_t>(getUInt32()) == other.getInt64();
                case VariantType::kUInt64: return getUInt32() == other.getUInt64();
                case VariantType::kFloat:
                    return getUInt32() == static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt32() == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() == other.getInt8();
                case VariantType::kUInt8: return getInt64() == other.getUInt8();
                case VariantType::kInt16: return getInt64() == other.getInt16();
                case VariantType::kUInt16: return getInt64() == other.getUInt16();
                case VariantType::kInt32: return getInt64() == other.getInt32();
                case VariantType::kUInt32: return getInt64() == other.getUInt32();
                case VariantType::kInt64: return getInt64() == other.getInt64();
                case VariantType::kUInt64:
                    return getInt64() == static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat:
                    return getInt64() == static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt64() == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int64_t>(getUInt64()) == other.getInt8();
                case VariantType::kUInt8: return getUInt64() == other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int64_t>(getUInt64()) == other.getInt16();
                case VariantType::kUInt16: return getUInt64() == other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int64_t>(getUInt64()) == other.getInt32();
                case VariantType::kUInt32: return getUInt64() == other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int64_t>(getUInt64()) == other.getInt64();
                case VariantType::kUInt64: return getUInt64() == other.getUInt64();
                case VariantType::kFloat:
                    return getUInt64() == static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt64() == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kFloat: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getFloat() == other.getInt8();
                case VariantType::kUInt8: return getFloat() == other.getUInt8();
                case VariantType::kInt16: return getFloat() == other.getInt16();
                case VariantType::kUInt16: return getFloat() == other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<double>(getFloat()) == other.getInt32();
                case VariantType::kUInt32:
                    return static_cast<double>(getFloat()) == other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<double>(getFloat()) == other.getInt64();
                case VariantType::kUInt64:
                    return static_cast<double>(getFloat()) == other.getUInt64();
                case VariantType::kFloat: return getFloat() == other.getFloat();
                case VariantType::kDouble:
                    return static_cast<double>(getFloat()) == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kDouble: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getDouble() == other.getInt8();
                case VariantType::kUInt8: return getDouble() == other.getUInt8();
                case VariantType::kInt16: return getDouble() == other.getInt16();
                case VariantType::kUInt16: return getDouble() == other.getUInt16();
                case VariantType::kInt32: return getDouble() == other.getInt32();
                case VariantType::kUInt32: return getDouble() == other.getUInt32();
                case VariantType::kInt64: return getDouble() == other.getInt64();
                case VariantType::kUInt64: return getDouble() == other.getUInt64();
                case VariantType::kFloat:
                    return getDouble() == static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getDouble() == other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kString: {
            if (other.isString()) {
                return utils::utf8_strcmp(*m_value.m_string, *other.m_value.m_string) == 0;
            } else if (other.isDateTime()) {
                // Dates could be sent as simple strings like '1991-10-20'
                return asDateTime() == *other.m_value.m_dt;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kBinary: {
            if (other.isBinary()) {
                return *m_value.m_binary == *other.m_value.m_binary;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kDateTime: {
            if (other.isDateTime()) {
                return *m_value.m_dt == *other.m_value.m_dt;
            } else if (other.isString()) {
                return *m_value.m_dt == other.asDateTime();
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        default: {
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
    }
}

bool Variant::compatibleLess(const Variant& other) const
{
    if (SIODB_UNLIKELY(&other == this)) return false;
    switch (m_valueType) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() < other.getInt8();
                case VariantType::kUInt8: return getInt8() < other.getUInt8();
                case VariantType::kInt16: return getInt8() < other.getInt16();
                case VariantType::kUInt16: return getInt8() < other.getUInt16();
                case VariantType::kInt32: return getInt8() < other.getInt32();
                case VariantType::kUInt32:
                    return getInt8() < static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt8() < other.getInt64();
                case VariantType::kUInt64:
                    return getInt8() < static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt8() < other.getFloat();
                case VariantType::kDouble: return getInt8() < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() < other.getInt8();
                case VariantType::kUInt8: return getUInt8() < other.getUInt8();
                case VariantType::kInt16: return getUInt8() < other.getInt16();
                case VariantType::kUInt16: return getUInt8() < other.getUInt16();
                case VariantType::kInt32: return getUInt8() < other.getInt32();
                case VariantType::kUInt32: return getUInt8() < other.getUInt32();
                case VariantType::kInt64: return getUInt8() < other.getInt64();
                case VariantType::kUInt64: return getUInt8() < other.getUInt64();
                case VariantType::kFloat: return getUInt8() < other.getFloat();
                case VariantType::kDouble: return getUInt8() < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() < other.getInt8();
                case VariantType::kUInt8: return getInt16() < other.getUInt8();
                case VariantType::kInt16: return getInt16() < other.getInt16();
                case VariantType::kUInt16: return getInt16() < other.getUInt16();
                case VariantType::kInt32: return getInt16() < other.getInt32();
                case VariantType::kUInt32:
                    return getInt16() < static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt16() < other.getInt64();
                case VariantType::kUInt64:
                    return getInt16() < static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt16() < other.getFloat();
                case VariantType::kDouble: return getInt16() < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() < other.getInt8();
                case VariantType::kUInt8: return getUInt16() < other.getUInt8();
                case VariantType::kInt16: return getUInt16() < other.getInt16();
                case VariantType::kUInt16: return getUInt16() < other.getUInt16();
                case VariantType::kInt32: return getUInt16() < other.getInt32();
                case VariantType::kUInt32: return getUInt16() < other.getUInt32();
                case VariantType::kInt64: return getUInt16() < other.getInt64();
                case VariantType::kUInt64: return getUInt16() < other.getUInt64();
                case VariantType::kFloat: return getUInt16() < other.getFloat();
                case VariantType::kDouble: return getUInt16() < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() < other.getInt8();
                case VariantType::kUInt8: return getInt32() < other.getUInt8();
                case VariantType::kInt16: return getInt32() < other.getInt16();
                case VariantType::kUInt16: return getInt32() < other.getUInt16();
                case VariantType::kInt32: return getInt32() < other.getInt32();
                case VariantType::kUInt32:
                    return getInt32() < static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt32() < other.getInt64();
                case VariantType::kUInt64:
                    return getInt32() < static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt32() < static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt32() < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int32_t>(getUInt32()) < other.getInt8();
                case VariantType::kUInt8: return getUInt32() < other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int32_t>(getUInt32()) < other.getInt16();
                case VariantType::kUInt16: return getUInt32() < other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int32_t>(getUInt32()) < other.getInt32();
                case VariantType::kUInt32: return getUInt32() < other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int32_t>(getUInt32()) < other.getInt64();
                case VariantType::kUInt64: return getUInt32() < other.getUInt64();
                case VariantType::kFloat:
                    return getUInt32() < static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt32() < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() < other.getInt8();
                case VariantType::kUInt8: return getInt64() < other.getUInt8();
                case VariantType::kInt16: return getInt64() < other.getInt16();
                case VariantType::kUInt16: return getInt64() < other.getUInt16();
                case VariantType::kInt32: return getInt64() < other.getInt32();
                case VariantType::kUInt32: return getInt64() < other.getUInt32();
                case VariantType::kInt64: return getInt64() < other.getInt64();
                case VariantType::kUInt64:
                    return getInt64() < static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt64() < static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt64() < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int64_t>(getUInt64()) < other.getInt8();
                case VariantType::kUInt8: return getUInt64() < other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int64_t>(getUInt64()) < other.getInt16();
                case VariantType::kUInt16: return getUInt64() < other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int64_t>(getUInt64()) < other.getInt32();
                case VariantType::kUInt32: return getUInt64() < other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int64_t>(getUInt64()) < other.getInt64();
                case VariantType::kUInt64: return getUInt64() < other.getUInt64();
                case VariantType::kFloat:
                    return getUInt64() < static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt64() < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kFloat: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getFloat() < other.getInt8();
                case VariantType::kUInt8: return getFloat() < other.getUInt8();
                case VariantType::kInt16: return getFloat() < other.getInt16();
                case VariantType::kUInt16: return getFloat() < other.getUInt16();
                case VariantType::kInt32: return static_cast<double>(getFloat()) < other.getInt32();
                case VariantType::kUInt32:
                    return static_cast<double>(getFloat()) < other.getUInt32();
                case VariantType::kInt64: return static_cast<double>(getFloat()) < other.getInt64();
                case VariantType::kUInt64:
                    return static_cast<double>(getFloat()) < other.getUInt64();
                case VariantType::kFloat: return getFloat() < other.getFloat();
                case VariantType::kDouble:
                    return static_cast<double>(getFloat()) < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kDouble: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getDouble() < other.getInt8();
                case VariantType::kUInt8: return getDouble() < other.getUInt8();
                case VariantType::kInt16: return getDouble() < other.getInt16();
                case VariantType::kUInt16: return getDouble() < other.getUInt16();
                case VariantType::kInt32: return getDouble() < other.getInt32();
                case VariantType::kUInt32: return getDouble() < other.getUInt32();
                case VariantType::kInt64: return getDouble() < other.getInt64();
                case VariantType::kUInt64: return getDouble() < other.getUInt64();
                case VariantType::kFloat:
                    return getDouble() < static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getDouble() < other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kString: {
            if (other.isString()) {
                return utils::utf8_strcmp(*m_value.m_string, *other.m_value.m_string) < 0;
            } else if (other.isDateTime()) {
                // Dates could be sent as simple strings like '1991-10-20'
                return asDateTime() < *other.m_value.m_dt;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kBinary: {
            if (other.isBinary()) {
                return *m_value.m_binary < *other.m_value.m_binary;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kDateTime: {
            if (other.isDateTime()) {
                return *m_value.m_dt < *other.m_value.m_dt;
            } else if (other.isString()) {
                return *m_value.m_dt < other.asDateTime();
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        default: {
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
    }
}

bool Variant::compatibleLessOrEqual(const Variant& other) const
{
    if (SIODB_UNLIKELY(&other == this)) return true;
    switch (m_valueType) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() <= other.getInt8();
                case VariantType::kUInt8: return getInt8() <= other.getUInt8();
                case VariantType::kInt16: return getInt8() <= other.getInt16();
                case VariantType::kUInt16: return getInt8() <= other.getUInt16();
                case VariantType::kInt32: return getInt8() <= other.getInt32();
                case VariantType::kUInt32:
                    return getInt8() <= static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt8() <= other.getInt64();
                case VariantType::kUInt64:
                    return getInt8() <= static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt8() <= other.getFloat();
                case VariantType::kDouble: return getInt8() <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() <= other.getInt8();
                case VariantType::kUInt8: return getUInt8() <= other.getUInt8();
                case VariantType::kInt16: return getUInt8() <= other.getInt16();
                case VariantType::kUInt16: return getUInt8() <= other.getUInt16();
                case VariantType::kInt32: return getUInt8() <= other.getInt32();
                case VariantType::kUInt32: return getUInt8() <= other.getUInt32();
                case VariantType::kInt64: return getUInt8() <= other.getInt64();
                case VariantType::kUInt64: return getUInt8() <= other.getUInt64();
                case VariantType::kFloat: return getUInt8() <= other.getFloat();
                case VariantType::kDouble: return getUInt8() <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() <= other.getInt8();
                case VariantType::kUInt8: return getInt16() <= other.getUInt8();
                case VariantType::kInt16: return getInt16() <= other.getInt16();
                case VariantType::kUInt16: return getInt16() <= other.getUInt16();
                case VariantType::kInt32: return getInt16() <= other.getInt32();
                case VariantType::kUInt32:
                    return getInt16() <= static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt16() <= other.getInt64();
                case VariantType::kUInt64:
                    return getInt16() <= static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt16() <= other.getFloat();
                case VariantType::kDouble: return getInt16() <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() <= other.getInt8();
                case VariantType::kUInt8: return getUInt16() <= other.getUInt8();
                case VariantType::kInt16: return getUInt16() <= other.getInt16();
                case VariantType::kUInt16: return getUInt16() <= other.getUInt16();
                case VariantType::kInt32: return getUInt16() <= other.getInt32();
                case VariantType::kUInt32: return getUInt16() <= other.getUInt32();
                case VariantType::kInt64: return getUInt16() <= other.getInt64();
                case VariantType::kUInt64: return getUInt16() <= other.getUInt64();
                case VariantType::kFloat: return getUInt16() <= other.getFloat();
                case VariantType::kDouble: return getUInt16() <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() <= other.getInt8();
                case VariantType::kUInt8: return getInt32() <= other.getUInt8();
                case VariantType::kInt16: return getInt32() <= other.getInt16();
                case VariantType::kUInt16: return getInt32() <= other.getUInt16();
                case VariantType::kInt32: return getInt32() <= other.getInt32();
                case VariantType::kUInt32:
                    return getInt32() <= static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt32() <= other.getInt64();
                case VariantType::kUInt64:
                    return getInt32() <= static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat:
                    return getInt32() <= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt32() <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int32_t>(getUInt32()) <= other.getInt8();
                case VariantType::kUInt8: return getUInt32() <= other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int32_t>(getUInt32()) <= other.getInt16();
                case VariantType::kUInt16: return getUInt32() <= other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int32_t>(getUInt32()) <= other.getInt32();
                case VariantType::kUInt32: return getUInt32() <= other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int32_t>(getUInt32()) <= other.getInt64();
                case VariantType::kUInt64: return getUInt32() <= other.getUInt64();
                case VariantType::kFloat:
                    return getUInt32() <= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt32() <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() <= other.getInt8();
                case VariantType::kUInt8: return getInt64() <= other.getUInt8();
                case VariantType::kInt16: return getInt64() <= other.getInt16();
                case VariantType::kUInt16: return getInt64() <= other.getUInt16();
                case VariantType::kInt32: return getInt64() <= other.getInt32();
                case VariantType::kUInt32: return getInt64() <= other.getUInt32();
                case VariantType::kInt64: return getInt64() <= other.getInt64();
                case VariantType::kUInt64:
                    return getInt64() <= static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat:
                    return getInt64() <= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt64() <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int64_t>(getUInt64()) <= other.getInt8();
                case VariantType::kUInt8: return getUInt64() <= other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int64_t>(getUInt64()) <= other.getInt16();
                case VariantType::kUInt16: return getUInt64() <= other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int64_t>(getUInt64()) <= other.getInt32();
                case VariantType::kUInt32: return getUInt64() <= other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int64_t>(getUInt64()) <= other.getInt64();
                case VariantType::kUInt64: return getUInt64() <= other.getUInt64();
                case VariantType::kFloat:
                    return getUInt64() <= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt64() <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kFloat: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getFloat() <= other.getInt8();
                case VariantType::kUInt8: return getFloat() <= other.getUInt8();
                case VariantType::kInt16: return getFloat() <= other.getInt16();
                case VariantType::kUInt16: return getFloat() <= other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<double>(getFloat()) <= other.getInt32();
                case VariantType::kUInt32:
                    return static_cast<double>(getFloat()) <= other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<double>(getFloat()) <= other.getInt64();
                case VariantType::kUInt64:
                    return static_cast<double>(getFloat()) <= other.getUInt64();
                case VariantType::kFloat: return getFloat() <= other.getFloat();
                case VariantType::kDouble:
                    return static_cast<double>(getFloat()) <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kDouble: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getDouble() <= other.getInt8();
                case VariantType::kUInt8: return getDouble() <= other.getUInt8();
                case VariantType::kInt16: return getDouble() <= other.getInt16();
                case VariantType::kUInt16: return getDouble() <= other.getUInt16();
                case VariantType::kInt32: return getDouble() <= other.getInt32();
                case VariantType::kUInt32: return getDouble() <= other.getUInt32();
                case VariantType::kInt64: return getDouble() <= other.getInt64();
                case VariantType::kUInt64: return getDouble() <= other.getUInt64();
                case VariantType::kFloat:
                    return getDouble() <= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getDouble() <= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kString: {
            if (other.isString()) {
                return utils::utf8_strcmp(*m_value.m_string, *other.m_value.m_string) <= 0;
            } else if (other.isDateTime()) {
                // Dates could be sent as simple strings like '1991-10-20'
                return asDateTime() <= *other.m_value.m_dt;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kBinary: {
            if (other.isBinary()) {
                return *m_value.m_binary <= *other.m_value.m_binary;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kDateTime: {
            if (other.isDateTime()) {
                return *m_value.m_dt <= *other.m_value.m_dt;
            } else if (other.isString()) {
                return *m_value.m_dt <= other.asDateTime();
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        default: {
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
    }
}

bool Variant::compatibleGreater(const Variant& other) const
{
    if (SIODB_UNLIKELY(&other == this)) return false;
    switch (m_valueType) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() > other.getInt8();
                case VariantType::kUInt8: return getInt8() > other.getUInt8();
                case VariantType::kInt16: return getInt8() > other.getInt16();
                case VariantType::kUInt16: return getInt8() > other.getUInt16();
                case VariantType::kInt32: return getInt8() > other.getInt32();
                case VariantType::kUInt32:
                    return getInt8() > static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt8() > other.getInt64();
                case VariantType::kUInt64:
                    return getInt8() > static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt8() > other.getFloat();
                case VariantType::kDouble: return getInt8() > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() > other.getInt8();
                case VariantType::kUInt8: return getUInt8() > other.getUInt8();
                case VariantType::kInt16: return getUInt8() > other.getInt16();
                case VariantType::kUInt16: return getUInt8() > other.getUInt16();
                case VariantType::kInt32: return getUInt8() > other.getInt32();
                case VariantType::kUInt32: return getUInt8() > other.getUInt32();
                case VariantType::kInt64: return getUInt8() > other.getInt64();
                case VariantType::kUInt64: return getUInt8() > other.getUInt64();
                case VariantType::kFloat: return getUInt8() > other.getFloat();
                case VariantType::kDouble: return getUInt8() > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() > other.getInt8();
                case VariantType::kUInt8: return getInt16() > other.getUInt8();
                case VariantType::kInt16: return getInt16() > other.getInt16();
                case VariantType::kUInt16: return getInt16() > other.getUInt16();
                case VariantType::kInt32: return getInt16() > other.getInt32();
                case VariantType::kUInt32:
                    return getInt16() > static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt16() > other.getInt64();
                case VariantType::kUInt64:
                    return getInt16() > static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt16() > other.getFloat();
                case VariantType::kDouble: return getInt16() > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() > other.getInt8();
                case VariantType::kUInt8: return getUInt16() > other.getUInt8();
                case VariantType::kInt16: return getUInt16() > other.getInt16();
                case VariantType::kUInt16: return getUInt16() > other.getUInt16();
                case VariantType::kInt32: return getUInt16() > other.getInt32();
                case VariantType::kUInt32: return getUInt16() > other.getUInt32();
                case VariantType::kInt64: return getUInt16() > other.getInt64();
                case VariantType::kUInt64: return getUInt16() > other.getUInt64();
                case VariantType::kFloat: return getUInt16() > other.getFloat();
                case VariantType::kDouble: return getUInt16() > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() > other.getInt8();
                case VariantType::kUInt8: return getInt32() > other.getUInt8();
                case VariantType::kInt16: return getInt32() > other.getInt16();
                case VariantType::kUInt16: return getInt32() > other.getUInt16();
                case VariantType::kInt32: return getInt32() > other.getInt32();
                case VariantType::kUInt32:
                    return getInt32() > static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt32() > other.getInt64();
                case VariantType::kUInt64:
                    return getInt32() > static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt32() > static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt32() > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int32_t>(getUInt32()) > other.getInt8();
                case VariantType::kUInt8: return getUInt32() > other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int32_t>(getUInt32()) > other.getInt16();
                case VariantType::kUInt16: return getUInt32() > other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int32_t>(getUInt32()) > other.getInt32();
                case VariantType::kUInt32: return getUInt32() > other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int32_t>(getUInt32()) > other.getInt64();
                case VariantType::kUInt64: return getUInt32() > other.getUInt64();
                case VariantType::kFloat:
                    return getUInt32() > static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt32() > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() > other.getInt8();
                case VariantType::kUInt8: return getInt64() > other.getUInt8();
                case VariantType::kInt16: return getInt64() > other.getInt16();
                case VariantType::kUInt16: return getInt64() > other.getUInt16();
                case VariantType::kInt32: return getInt64() > other.getInt32();
                case VariantType::kUInt32: return getInt64() > other.getUInt32();
                case VariantType::kInt64: return getInt64() > other.getInt64();
                case VariantType::kUInt64:
                    return getInt64() > static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt64() > static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt64() > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int64_t>(getUInt64()) > other.getInt8();
                case VariantType::kUInt8: return getUInt64() > other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int64_t>(getUInt64()) > other.getInt16();
                case VariantType::kUInt16: return getUInt64() > other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int64_t>(getUInt64()) > other.getInt32();
                case VariantType::kUInt32: return getUInt64() > other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int64_t>(getUInt64()) > other.getInt64();
                case VariantType::kUInt64: return getUInt64() > other.getUInt64();
                case VariantType::kFloat:
                    return getUInt64() > static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt64() > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kFloat: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getFloat() > other.getInt8();
                case VariantType::kUInt8: return getFloat() > other.getUInt8();
                case VariantType::kInt16: return getFloat() > other.getInt16();
                case VariantType::kUInt16: return getFloat() > other.getUInt16();
                case VariantType::kInt32: return static_cast<double>(getFloat()) > other.getInt32();
                case VariantType::kUInt32:
                    return static_cast<double>(getFloat()) > other.getUInt32();
                case VariantType::kInt64: return static_cast<double>(getFloat()) > other.getInt64();
                case VariantType::kUInt64:
                    return static_cast<double>(getFloat()) > other.getUInt64();
                case VariantType::kFloat: return getFloat() > other.getFloat();
                case VariantType::kDouble:
                    return static_cast<double>(getFloat()) > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kDouble: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getDouble() > other.getInt8();
                case VariantType::kUInt8: return getDouble() > other.getUInt8();
                case VariantType::kInt16: return getDouble() > other.getInt16();
                case VariantType::kUInt16: return getDouble() > other.getUInt16();
                case VariantType::kInt32: return getDouble() > other.getInt32();
                case VariantType::kUInt32: return getDouble() > other.getUInt32();
                case VariantType::kInt64: return getDouble() > other.getInt64();
                case VariantType::kUInt64: return getDouble() > other.getUInt64();
                case VariantType::kFloat:
                    return getDouble() > static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getDouble() > other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kString: {
            if (other.isString()) {
                return utils::utf8_strcmp(*m_value.m_string, *other.m_value.m_string) > 0;
            } else if (other.isDateTime()) {
                // Dates could be sent as simple strings like '1991-10-20'
                return asDateTime() > *other.m_value.m_dt;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kBinary: {
            if (other.isBinary()) {
                return *m_value.m_binary > *other.m_value.m_binary;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kDateTime: {
            if (other.isDateTime()) {
                return *m_value.m_dt > *other.m_value.m_dt;
            } else if (other.isString()) {
                return *m_value.m_dt > other.asDateTime();
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        default: {
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
    }
}

bool Variant::compatibleGreaterOrEqual(const Variant& other) const
{
    if (SIODB_UNLIKELY(&other == this)) return true;
    switch (m_valueType) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() >= other.getInt8();
                case VariantType::kUInt8: return getInt8() >= other.getUInt8();
                case VariantType::kInt16: return getInt8() >= other.getInt16();
                case VariantType::kUInt16: return getInt8() >= other.getUInt16();
                case VariantType::kInt32: return getInt8() >= other.getInt32();
                case VariantType::kUInt32:
                    return getInt8() >= static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt8() >= other.getInt64();
                case VariantType::kUInt64:
                    return getInt8() >= static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt8() >= other.getFloat();
                case VariantType::kDouble: return getInt8() >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() >= other.getInt8();
                case VariantType::kUInt8: return getUInt8() >= other.getUInt8();
                case VariantType::kInt16: return getUInt8() >= other.getInt16();
                case VariantType::kUInt16: return getUInt8() >= other.getUInt16();
                case VariantType::kInt32: return getUInt8() >= other.getInt32();
                case VariantType::kUInt32: return getUInt8() >= other.getUInt32();
                case VariantType::kInt64: return getUInt8() >= other.getInt64();
                case VariantType::kUInt64: return getUInt8() >= other.getUInt64();
                case VariantType::kFloat: return getUInt8() >= other.getFloat();
                case VariantType::kDouble: return getUInt8() >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() >= other.getInt8();
                case VariantType::kUInt8: return getInt16() >= other.getUInt8();
                case VariantType::kInt16: return getInt16() >= other.getInt16();
                case VariantType::kUInt16: return getInt16() >= other.getUInt16();
                case VariantType::kInt32: return getInt16() >= other.getInt32();
                case VariantType::kUInt32:
                    return getInt16() >= static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt16() >= other.getInt64();
                case VariantType::kUInt64:
                    return getInt16() >= static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat: return getInt16() >= other.getFloat();
                case VariantType::kDouble: return getInt16() >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() >= other.getInt8();
                case VariantType::kUInt8: return getUInt16() >= other.getUInt8();
                case VariantType::kInt16: return getUInt16() >= other.getInt16();
                case VariantType::kUInt16: return getUInt16() >= other.getUInt16();
                case VariantType::kInt32: return getUInt16() >= other.getInt32();
                case VariantType::kUInt32: return getUInt16() >= other.getUInt32();
                case VariantType::kInt64: return getUInt16() >= other.getInt64();
                case VariantType::kUInt64: return getUInt16() >= other.getUInt64();
                case VariantType::kFloat: return getUInt16() >= other.getFloat();
                case VariantType::kDouble: return getUInt16() >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() >= other.getInt8();
                case VariantType::kUInt8: return getInt32() >= other.getUInt8();
                case VariantType::kInt16: return getInt32() >= other.getInt16();
                case VariantType::kUInt16: return getInt32() >= other.getUInt16();
                case VariantType::kInt32: return getInt32() >= other.getInt32();
                case VariantType::kUInt32:
                    return getInt32() >= static_cast<std::int32_t>(other.getUInt32());
                case VariantType::kInt64: return getInt32() >= other.getInt64();
                case VariantType::kUInt64:
                    return getInt32() >= static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat:
                    return getInt32() >= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt32() >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int32_t>(getUInt32()) >= other.getInt8();
                case VariantType::kUInt8: return getUInt32() >= other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int32_t>(getUInt32()) >= other.getInt16();
                case VariantType::kUInt16: return getUInt32() >= other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int32_t>(getUInt32()) >= other.getInt32();
                case VariantType::kUInt32: return getUInt32() >= other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int32_t>(getUInt32()) >= other.getInt64();
                case VariantType::kUInt64: return getUInt32() >= other.getUInt64();
                case VariantType::kFloat:
                    return getUInt32() >= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt32() >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() >= other.getInt8();
                case VariantType::kUInt8: return getInt64() >= other.getUInt8();
                case VariantType::kInt16: return getInt64() >= other.getInt16();
                case VariantType::kUInt16: return getInt64() >= other.getUInt16();
                case VariantType::kInt32: return getInt64() >= other.getInt32();
                case VariantType::kUInt32: return getInt64() >= other.getUInt32();
                case VariantType::kInt64: return getInt64() >= other.getInt64();
                case VariantType::kUInt64:
                    return getInt64() >= static_cast<std::int64_t>(other.getUInt64());
                case VariantType::kFloat:
                    return getInt64() >= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getInt64() >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8:
                    return static_cast<std::int64_t>(getUInt64()) >= other.getInt8();
                case VariantType::kUInt8: return getUInt64() >= other.getUInt8();
                case VariantType::kInt16:
                    return static_cast<std::int64_t>(getUInt64()) >= other.getInt16();
                case VariantType::kUInt16: return getUInt64() >= other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<std::int64_t>(getUInt64()) >= other.getInt32();
                case VariantType::kUInt32: return getUInt64() >= other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<std::int64_t>(getUInt64()) >= other.getInt64();
                case VariantType::kUInt64: return getUInt64() >= other.getUInt64();
                case VariantType::kFloat:
                    return getUInt64() >= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getUInt64() >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kFloat: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getFloat() >= other.getInt8();
                case VariantType::kUInt8: return getFloat() >= other.getUInt8();
                case VariantType::kInt16: return getFloat() >= other.getInt16();
                case VariantType::kUInt16: return getFloat() >= other.getUInt16();
                case VariantType::kInt32:
                    return static_cast<double>(getFloat()) >= other.getInt32();
                case VariantType::kUInt32:
                    return static_cast<double>(getFloat()) >= other.getUInt32();
                case VariantType::kInt64:
                    return static_cast<double>(getFloat()) >= other.getInt64();
                case VariantType::kUInt64:
                    return static_cast<double>(getFloat()) >= other.getUInt64();
                case VariantType::kFloat: return getFloat() >= other.getFloat();
                case VariantType::kDouble:
                    return static_cast<double>(getFloat()) >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kDouble: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getDouble() >= other.getInt8();
                case VariantType::kUInt8: return getDouble() >= other.getUInt8();
                case VariantType::kInt16: return getDouble() >= other.getInt16();
                case VariantType::kUInt16: return getDouble() >= other.getUInt16();
                case VariantType::kInt32: return getDouble() >= other.getInt32();
                case VariantType::kUInt32: return getDouble() >= other.getUInt32();
                case VariantType::kInt64: return getDouble() >= other.getInt64();
                case VariantType::kUInt64: return getDouble() >= other.getUInt64();
                case VariantType::kFloat:
                    return getDouble() >= static_cast<double>(other.getFloat());
                case VariantType::kDouble: return getDouble() >= other.getDouble();
                default: {
                    throw VariantTypeCastError(
                            m_valueType, other.getValueType(), "Values comparison is impossible");
                }
            }
            break;
        }
        case VariantType::kString: {
            if (other.isString()) {
                return utils::utf8_strcmp(*m_value.m_string, *other.m_value.m_string) >= 0;
            } else if (other.isDateTime()) {
                // Dates could be sent as simple strings like '1991-10-20'
                return asDateTime() >= *other.m_value.m_dt;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kBinary: {
            if (other.isBinary()) {
                return *m_value.m_binary >= *other.m_value.m_binary;
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        case VariantType::kDateTime: {
            if (other.isDateTime()) {
                return *m_value.m_dt >= *other.m_value.m_dt;
            } else if (other.isString()) {
                return *m_value.m_dt >= other.asDateTime();
            }
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
        default: {
            throw VariantTypeCastError(
                    other.m_valueType, m_valueType, "Values comparison is impossible");
        }
    }
}

bool Variant::isNegative() const
{
    switch (m_valueType) {
        case VariantType::kInt8: return getInt8() < 0;
        case VariantType::kUInt8: return false;
        case VariantType::kInt16: return getInt16() < 0;
        case VariantType::kUInt16: return false;
        case VariantType::kInt32: return getInt32() < 0;
        case VariantType::kUInt32: return false;
        case VariantType::kInt64: return getInt64() < 0;
        case VariantType::kUInt64: return false;
        case VariantType::kDouble: return getDouble() < 0.0;
        case VariantType::kFloat: return getFloat() < 0.0f;
        default: {
            throw WrongVariantTypeError(m_valueType, "Value is not numeric");
        }
    }
}

bool Variant::isPositive() const
{
    switch (m_valueType) {
        case VariantType::kInt8: return getInt8() > 0;
        case VariantType::kUInt8: return getUInt8() > 0U;
        case VariantType::kInt16: return getInt16() > 0;
        case VariantType::kUInt16: return getUInt16() > 0U;
        case VariantType::kInt32: return getInt32() > 0;
        case VariantType::kUInt32: return getUInt32() > 0U;
        case VariantType::kInt64: return getInt64() > 0;
        case VariantType::kUInt64: return getUInt64() > 0U;
        case VariantType::kDouble: return getDouble() > 0.0;
        case VariantType::kFloat: return getFloat() > 0.0f;
        default: {
            throw WrongVariantTypeError(m_valueType, "Value is not numeric");
        }
    }
}

bool Variant::isZero() const
{
    switch (m_valueType) {
        case VariantType::kInt8: return getInt8() == 0;
        case VariantType::kUInt8: return getUInt8() == 0U;
        case VariantType::kInt16: return getInt16() == 0;
        case VariantType::kUInt16: return getUInt16() == 0U;
        case VariantType::kInt32: return getInt32() == 0;
        case VariantType::kUInt32: return getUInt32() == 0U;
        case VariantType::kInt64: return getInt64() == 0;
        case VariantType::kUInt64: return getUInt64() == 0U;
        case VariantType::kDouble: return getDouble() == 0.0;
        case VariantType::kFloat: return getFloat() == 0.0f;
        default: {
            throw WrongVariantTypeError(m_valueType, "Value is not numeric");
        }
    }
}
}  // namespace siodb::iomgr::dbengine
