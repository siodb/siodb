// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant.h"

// Common project headers
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

Variant Variant::operator-() const
{
    switch (m_valueType) {
        case VariantType::kInt8: return -m_value.m_i8;
        case VariantType::kUInt8: return -m_value.m_ui8;
        case VariantType::kInt16: return -m_value.m_i16;
        case VariantType::kUInt16: return -m_value.m_ui16;
        case VariantType::kInt32: return -m_value.m_i32;
        case VariantType::kUInt32: return -m_value.m_ui32;
        case VariantType::kInt64: return -m_value.m_i64;
        case VariantType::kUInt64: return -m_value.m_ui64;
        case VariantType::kFloat: return -m_value.m_float;
        case VariantType::kDouble: return -m_value.m_double;
        default: throw WrongVariantTypeError(m_valueType, "Value is not numeric");
    }
}

Variant Variant::operator+() const
{
    switch (m_valueType) {
        case VariantType::kInt8: return +m_value.m_i8;
        case VariantType::kUInt8: return +m_value.m_ui8;
        case VariantType::kInt16: return +m_value.m_i16;
        case VariantType::kUInt16: return +m_value.m_ui16;
        case VariantType::kInt32: return +m_value.m_i32;
        case VariantType::kUInt32: return +m_value.m_ui32;
        case VariantType::kInt64: return +m_value.m_i64;
        case VariantType::kUInt64: return +m_value.m_ui64;
        case VariantType::kFloat: return +m_value.m_float;
        case VariantType::kDouble: return +m_value.m_double;
        default: throw WrongVariantTypeError(m_valueType, "Value is not numeric");
    }
}

Variant Variant::operator~() const
{
    switch (m_valueType) {
        case VariantType::kInt8: return ~m_value.m_i8;
        case VariantType::kUInt8: return ~m_value.m_ui8;
        case VariantType::kInt16: return ~m_value.m_i16;
        case VariantType::kUInt16: return ~m_value.m_ui16;
        case VariantType::kInt32: return ~m_value.m_i32;
        case VariantType::kUInt32: return ~m_value.m_ui32;
        case VariantType::kInt64: return ~m_value.m_i64;
        case VariantType::kUInt64: return ~m_value.m_ui64;
        default: throw WrongVariantTypeError(m_valueType, "Value is not numeric");
    }
}

Variant Variant::operator+(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() + other.getInt8();
                case VariantType::kUInt8: return getInt8() + other.getUInt8();
                case VariantType::kInt16: return getInt8() + other.getInt16();
                case VariantType::kUInt16: return getInt8() + other.getUInt16();
                case VariantType::kInt32: return getInt8() + other.getInt32();
                case VariantType::kUInt32: return getInt8() + other.getUInt32();
                case VariantType::kInt64: return getInt8() + other.getInt64();
                case VariantType::kUInt64: return getInt8() + other.getUInt64();
                case VariantType::kFloat: return getInt8() + other.getFloat();
                case VariantType::kDouble: return getInt8() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() + other.getInt8();
                case VariantType::kUInt8: return getUInt8() + other.getUInt8();
                case VariantType::kInt16: return getUInt8() + other.getInt16();
                case VariantType::kUInt16: return getUInt8() + other.getUInt16();
                case VariantType::kInt32: return getUInt8() + other.getInt32();
                case VariantType::kUInt32: return getUInt8() + other.getUInt32();
                case VariantType::kInt64: return getUInt8() + other.getInt64();
                case VariantType::kUInt64: return getUInt8() + other.getUInt64();
                case VariantType::kFloat: return getUInt8() + other.getFloat();
                case VariantType::kDouble: return getUInt8() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() + other.getInt8();
                case VariantType::kUInt8: return getInt16() + other.getUInt8();
                case VariantType::kInt16: return getInt16() + other.getInt16();
                case VariantType::kUInt16: return getInt16() + other.getUInt16();
                case VariantType::kInt32: return getInt16() + other.getInt32();
                case VariantType::kUInt32: return getInt16() + other.getUInt32();
                case VariantType::kInt64: return getInt16() + other.getInt64();
                case VariantType::kUInt64: return getInt16() + other.getUInt64();
                case VariantType::kFloat: return getInt16() + other.getFloat();
                case VariantType::kDouble: return getInt16() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() + other.getInt8();
                case VariantType::kUInt8: return getUInt16() + other.getUInt8();
                case VariantType::kInt16: return getUInt16() + other.getInt16();
                case VariantType::kUInt16: return getUInt16() + other.getUInt16();
                case VariantType::kInt32: return getUInt16() + other.getInt32();
                case VariantType::kUInt32: return getUInt16() + other.getUInt32();
                case VariantType::kInt64: return getUInt16() + other.getInt64();
                case VariantType::kUInt64: return getUInt16() + other.getUInt64();
                case VariantType::kFloat: return getUInt16() + other.getFloat();
                case VariantType::kDouble: return getUInt16() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() + other.getInt8();
                case VariantType::kUInt8: return getInt32() + other.getUInt8();
                case VariantType::kInt16: return getInt32() + other.getInt16();
                case VariantType::kUInt16: return getInt32() + other.getUInt16();
                case VariantType::kInt32: return getInt32() + other.getInt32();
                case VariantType::kUInt32: return getInt32() + other.getUInt32();
                case VariantType::kInt64: return getInt32() + other.getInt64();
                case VariantType::kUInt64: return getInt32() + other.getUInt64();
                case VariantType::kFloat: return getInt32() + other.getFloat();
                case VariantType::kDouble: return getInt32() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() + other.getInt8();
                case VariantType::kUInt8: return getUInt32() + other.getUInt8();
                case VariantType::kInt16: return getUInt32() + other.getInt16();
                case VariantType::kUInt16: return getUInt32() + other.getUInt16();
                case VariantType::kInt32: return getUInt32() + other.getInt32();
                case VariantType::kUInt32: return getUInt32() + other.getUInt32();
                case VariantType::kInt64: return getUInt32() + other.getInt64();
                case VariantType::kUInt64: return getUInt32() + other.getUInt64();
                case VariantType::kFloat: return getUInt32() + other.getFloat();
                case VariantType::kDouble: return getUInt32() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() + other.getInt8();
                case VariantType::kUInt8: return getInt64() + other.getUInt8();
                case VariantType::kInt16: return getInt64() + other.getInt16();
                case VariantType::kUInt16: return getInt64() + other.getUInt16();
                case VariantType::kInt32: return getInt64() + other.getInt32();
                case VariantType::kUInt32: return getInt64() + other.getUInt32();
                case VariantType::kInt64: return getInt64() + other.getInt64();
                case VariantType::kUInt64: return getInt64() + other.getUInt64();
                case VariantType::kFloat: return getInt64() + other.getFloat();
                case VariantType::kDouble: return getInt64() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() + other.getInt8();
                case VariantType::kUInt8: return getUInt64() + other.getUInt8();
                case VariantType::kInt16: return getUInt64() + other.getInt16();
                case VariantType::kUInt16: return getUInt64() + other.getUInt16();
                case VariantType::kInt32: return getUInt64() + other.getInt32();
                case VariantType::kUInt32: return getUInt64() + other.getUInt32();
                case VariantType::kInt64: return getUInt64() + other.getInt64();
                case VariantType::kUInt64: return getUInt64() + other.getUInt64();
                case VariantType::kFloat: return getUInt64() + other.getFloat();
                case VariantType::kDouble: return getUInt64() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kFloat: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getFloat() + other.getInt8();
                case VariantType::kUInt8: return getFloat() + other.getUInt8();
                case VariantType::kInt16: return getFloat() + other.getInt16();
                case VariantType::kUInt16: return getFloat() + other.getUInt16();
                case VariantType::kInt32: return getFloat() + other.getInt32();
                case VariantType::kUInt32: return getFloat() + other.getUInt32();
                case VariantType::kInt64: return getFloat() + other.getInt64();
                case VariantType::kUInt64: return getFloat() + other.getUInt64();
                case VariantType::kFloat: return getFloat() + other.getFloat();
                case VariantType::kDouble: return getFloat() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kDouble: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getDouble() + other.getInt8();
                case VariantType::kUInt8: return getDouble() + other.getUInt8();
                case VariantType::kInt16: return getDouble() + other.getInt16();
                case VariantType::kUInt16: return getDouble() + other.getUInt16();
                case VariantType::kInt32: return getDouble() + other.getInt32();
                case VariantType::kUInt32: return getDouble() + other.getUInt32();
                case VariantType::kInt64: return getDouble() + other.getInt64();
                case VariantType::kUInt64: return getDouble() + other.getUInt64();
                case VariantType::kFloat: return getDouble() + other.getFloat();
                case VariantType::kDouble: return getDouble() + other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        case VariantType::kString: {
            switch (other.getValueType()) {
                case VariantType::kString: return getString() + other.getString();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for + operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for + operation");
        }
    }
}

Variant Variant::operator-(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() - other.getInt8();
                case VariantType::kUInt8: return getInt8() - other.getUInt8();
                case VariantType::kInt16: return getInt8() - other.getInt16();
                case VariantType::kUInt16: return getInt8() - other.getUInt16();
                case VariantType::kInt32: return getInt8() - other.getInt32();
                case VariantType::kUInt32: return getInt8() - other.getUInt32();
                case VariantType::kInt64: return getInt8() - other.getInt64();
                case VariantType::kUInt64: return getInt8() - other.getUInt64();
                case VariantType::kFloat: return getInt8() - other.getFloat();
                case VariantType::kDouble: return getInt8() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() - other.getInt8();
                case VariantType::kUInt8: return getUInt8() - other.getUInt8();
                case VariantType::kInt16: return getUInt8() - other.getInt16();
                case VariantType::kUInt16: return getUInt8() - other.getUInt16();
                case VariantType::kInt32: return getUInt8() - other.getInt32();
                case VariantType::kUInt32: return getUInt8() - other.getUInt32();
                case VariantType::kInt64: return getUInt8() - other.getInt64();
                case VariantType::kUInt64: return getUInt8() - other.getUInt64();
                case VariantType::kFloat: return getUInt8() - other.getFloat();
                case VariantType::kDouble: return getUInt8() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() - other.getInt8();
                case VariantType::kUInt8: return getInt16() - other.getUInt8();
                case VariantType::kInt16: return getInt16() - other.getInt16();
                case VariantType::kUInt16: return getInt16() - other.getUInt16();
                case VariantType::kInt32: return getInt16() - other.getInt32();
                case VariantType::kUInt32: return getInt16() - other.getUInt32();
                case VariantType::kInt64: return getInt16() - other.getInt64();
                case VariantType::kUInt64: return getInt16() - other.getUInt64();
                case VariantType::kFloat: return getInt16() - other.getFloat();
                case VariantType::kDouble: return getInt16() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() - other.getInt8();
                case VariantType::kUInt8: return getUInt16() - other.getUInt8();
                case VariantType::kInt16: return getUInt16() - other.getInt16();
                case VariantType::kUInt16: return getUInt16() - other.getUInt16();
                case VariantType::kInt32: return getUInt16() - other.getInt32();
                case VariantType::kUInt32: return getUInt16() - other.getUInt32();
                case VariantType::kInt64: return getUInt16() - other.getInt64();
                case VariantType::kUInt64: return getUInt16() - other.getUInt64();
                case VariantType::kFloat: return getUInt16() - other.getFloat();
                case VariantType::kDouble: return getUInt16() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() - other.getInt8();
                case VariantType::kUInt8: return getInt32() - other.getUInt8();
                case VariantType::kInt16: return getInt32() - other.getInt16();
                case VariantType::kUInt16: return getInt32() - other.getUInt16();
                case VariantType::kInt32: return getInt32() - other.getInt32();
                case VariantType::kUInt32: return getInt32() - other.getUInt32();
                case VariantType::kInt64: return getInt32() - other.getInt64();
                case VariantType::kUInt64: return getInt32() - other.getUInt64();
                case VariantType::kFloat: return getInt32() - other.getFloat();
                case VariantType::kDouble: return getInt32() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() - other.getInt8();
                case VariantType::kUInt8: return getUInt32() - other.getUInt8();
                case VariantType::kInt16: return getUInt32() - other.getInt16();
                case VariantType::kUInt16: return getUInt32() - other.getUInt16();
                case VariantType::kInt32: return getUInt32() - other.getInt32();
                case VariantType::kUInt32: return getUInt32() - other.getUInt32();
                case VariantType::kInt64: return getUInt32() - other.getInt64();
                case VariantType::kUInt64: return getUInt32() - other.getUInt64();
                case VariantType::kFloat: return getUInt32() - other.getFloat();
                case VariantType::kDouble: return getUInt32() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() - other.getInt8();
                case VariantType::kUInt8: return getInt64() - other.getUInt8();
                case VariantType::kInt16: return getInt64() - other.getInt16();
                case VariantType::kUInt16: return getInt64() - other.getUInt16();
                case VariantType::kInt32: return getInt64() - other.getInt32();
                case VariantType::kUInt32: return getInt64() - other.getUInt32();
                case VariantType::kInt64: return getInt64() - other.getInt64();
                case VariantType::kUInt64: return getInt64() - other.getUInt64();
                case VariantType::kFloat: return getInt64() - other.getFloat();
                case VariantType::kDouble: return getInt64() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() - other.getInt8();
                case VariantType::kUInt8: return getUInt64() - other.getUInt8();
                case VariantType::kInt16: return getUInt64() - other.getInt16();
                case VariantType::kUInt16: return getUInt64() - other.getUInt16();
                case VariantType::kInt32: return getUInt64() - other.getInt32();
                case VariantType::kUInt32: return getUInt64() - other.getUInt32();
                case VariantType::kInt64: return getUInt64() - other.getInt64();
                case VariantType::kUInt64: return getUInt64() - other.getUInt64();
                case VariantType::kFloat: return getUInt64() - other.getFloat();
                case VariantType::kDouble: return getUInt64() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        case VariantType::kFloat: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getFloat() - other.getInt8();
                case VariantType::kUInt8: return getFloat() - other.getUInt8();
                case VariantType::kInt16: return getFloat() - other.getInt16();
                case VariantType::kUInt16: return getFloat() - other.getUInt16();
                case VariantType::kInt32: return getFloat() - other.getInt32();
                case VariantType::kUInt32: return getFloat() - other.getUInt32();
                case VariantType::kInt64: return getFloat() - other.getInt64();
                case VariantType::kUInt64: return getFloat() - other.getUInt64();
                case VariantType::kFloat: return getFloat() - other.getFloat();
                case VariantType::kDouble: return getFloat() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        case VariantType::kDouble: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getDouble() - other.getInt8();
                case VariantType::kUInt8: return getDouble() - other.getUInt8();
                case VariantType::kInt16: return getDouble() - other.getInt16();
                case VariantType::kUInt16: return getDouble() - other.getUInt16();
                case VariantType::kInt32: return getDouble() - other.getInt32();
                case VariantType::kUInt32: return getDouble() - other.getUInt32();
                case VariantType::kInt64: return getDouble() - other.getInt64();
                case VariantType::kUInt64: return getDouble() - other.getUInt64();
                case VariantType::kFloat: return getDouble() - other.getFloat();
                case VariantType::kDouble: return getDouble() - other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for - operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for - operation");
        }
    }
}

Variant Variant::operator*(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() * other.getInt8();
                case VariantType::kUInt8: return getInt8() * other.getUInt8();
                case VariantType::kInt16: return getInt8() * other.getInt16();
                case VariantType::kUInt16: return getInt8() * other.getUInt16();
                case VariantType::kInt32: return getInt8() * other.getInt32();
                case VariantType::kUInt32: return getInt8() * other.getUInt32();
                case VariantType::kInt64: return getInt8() * other.getInt64();
                case VariantType::kUInt64: return getInt8() * other.getUInt64();
                case VariantType::kFloat: return getInt8() * other.getFloat();
                case VariantType::kDouble: return getInt8() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() * other.getInt8();
                case VariantType::kUInt8: return getUInt8() * other.getUInt8();
                case VariantType::kInt16: return getUInt8() * other.getInt16();
                case VariantType::kUInt16: return getUInt8() * other.getUInt16();
                case VariantType::kInt32: return getUInt8() * other.getInt32();
                case VariantType::kUInt32: return getUInt8() * other.getUInt32();
                case VariantType::kInt64: return getUInt8() * other.getInt64();
                case VariantType::kUInt64: return getUInt8() * other.getUInt64();
                case VariantType::kFloat: return getUInt8() * other.getFloat();
                case VariantType::kDouble: return getUInt8() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() * other.getInt8();
                case VariantType::kUInt8: return getInt16() * other.getUInt8();
                case VariantType::kInt16: return getInt16() * other.getInt16();
                case VariantType::kUInt16: return getInt16() * other.getUInt16();
                case VariantType::kInt32: return getInt16() * other.getInt32();
                case VariantType::kUInt32: return getInt16() * other.getUInt32();
                case VariantType::kInt64: return getInt16() * other.getInt64();
                case VariantType::kUInt64: return getInt16() * other.getUInt64();
                case VariantType::kFloat: return getInt16() * other.getFloat();
                case VariantType::kDouble: return getInt16() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() * other.getInt8();
                case VariantType::kUInt8: return getUInt16() * other.getUInt8();
                case VariantType::kInt16: return getUInt16() * other.getInt16();
                case VariantType::kUInt16: return getUInt16() * other.getUInt16();
                case VariantType::kInt32: return getUInt16() * other.getInt32();
                case VariantType::kUInt32: return getUInt16() * other.getUInt32();
                case VariantType::kInt64: return getUInt16() * other.getInt64();
                case VariantType::kUInt64: return getUInt16() * other.getUInt64();
                case VariantType::kFloat: return getUInt16() * other.getFloat();
                case VariantType::kDouble: return getUInt16() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() * other.getInt8();
                case VariantType::kUInt8: return getInt32() * other.getUInt8();
                case VariantType::kInt16: return getInt32() * other.getInt16();
                case VariantType::kUInt16: return getInt32() * other.getUInt16();
                case VariantType::kInt32: return getInt32() * other.getInt32();
                case VariantType::kUInt32: return getInt32() * other.getUInt32();
                case VariantType::kInt64: return getInt32() * other.getInt64();
                case VariantType::kUInt64: return getInt32() * other.getUInt64();
                case VariantType::kFloat: return getInt32() * other.getFloat();
                case VariantType::kDouble: return getInt32() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() * other.getInt8();
                case VariantType::kUInt8: return getUInt32() * other.getUInt8();
                case VariantType::kInt16: return getUInt32() * other.getInt16();
                case VariantType::kUInt16: return getUInt32() * other.getUInt16();
                case VariantType::kInt32: return getUInt32() * other.getInt32();
                case VariantType::kUInt32: return getUInt32() * other.getUInt32();
                case VariantType::kInt64: return getUInt32() * other.getInt64();
                case VariantType::kUInt64: return getUInt32() * other.getUInt64();
                case VariantType::kFloat: return getUInt32() * other.getFloat();
                case VariantType::kDouble: return getUInt32() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() * other.getInt8();
                case VariantType::kUInt8: return getInt64() * other.getUInt8();
                case VariantType::kInt16: return getInt64() * other.getInt16();
                case VariantType::kUInt16: return getInt64() * other.getUInt16();
                case VariantType::kInt32: return getInt64() * other.getInt32();
                case VariantType::kUInt32: return getInt64() * other.getUInt32();
                case VariantType::kInt64: return getInt64() * other.getInt64();
                case VariantType::kUInt64: return getInt64() * other.getUInt64();
                case VariantType::kFloat: return getInt64() * other.getFloat();
                case VariantType::kDouble: return getInt64() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() * other.getInt8();
                case VariantType::kUInt8: return getUInt64() * other.getUInt8();
                case VariantType::kInt16: return getUInt64() * other.getInt16();
                case VariantType::kUInt16: return getUInt64() * other.getUInt16();
                case VariantType::kInt32: return getUInt64() * other.getInt32();
                case VariantType::kUInt32: return getUInt64() * other.getUInt32();
                case VariantType::kInt64: return getUInt64() * other.getInt64();
                case VariantType::kUInt64: return getUInt64() * other.getUInt64();
                case VariantType::kFloat: return getUInt64() * other.getFloat();
                case VariantType::kDouble: return getUInt64() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        case VariantType::kFloat: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getFloat() * other.getInt8();
                case VariantType::kUInt8: return getFloat() * other.getUInt8();
                case VariantType::kInt16: return getFloat() * other.getInt16();
                case VariantType::kUInt16: return getFloat() * other.getUInt16();
                case VariantType::kInt32: return getFloat() * other.getInt32();
                case VariantType::kUInt32: return getFloat() * other.getUInt32();
                case VariantType::kInt64: return getFloat() * other.getInt64();
                case VariantType::kUInt64: return getFloat() * other.getUInt64();
                case VariantType::kFloat: return getFloat() * other.getFloat();
                case VariantType::kDouble: return getFloat() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        case VariantType::kDouble: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getDouble() * other.getInt8();
                case VariantType::kUInt8: return getDouble() * other.getUInt8();
                case VariantType::kInt16: return getDouble() * other.getInt16();
                case VariantType::kUInt16: return getDouble() * other.getUInt16();
                case VariantType::kInt32: return getDouble() * other.getInt32();
                case VariantType::kUInt32: return getDouble() * other.getUInt32();
                case VariantType::kInt64: return getDouble() * other.getInt64();
                case VariantType::kUInt64: return getDouble() * other.getUInt64();
                case VariantType::kFloat: return getDouble() * other.getFloat();
                case VariantType::kDouble: return getDouble() * other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for * operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for * operation");
        }
    }
}

Variant Variant::operator/(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() / other.getInt8();
                case VariantType::kUInt8: return getInt8() / other.getUInt8();
                case VariantType::kInt16: return getInt8() / other.getInt16();
                case VariantType::kUInt16: return getInt8() / other.getUInt16();
                case VariantType::kInt32: return getInt8() / other.getInt32();
                case VariantType::kUInt32: return getInt8() / other.getUInt32();
                case VariantType::kInt64: return getInt8() / other.getInt64();
                case VariantType::kUInt64: return getInt8() / other.getUInt64();
                case VariantType::kFloat: return getInt8() / other.getFloat();
                case VariantType::kDouble: return getInt8() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() / other.getInt8();
                case VariantType::kUInt8: return getUInt8() / other.getUInt8();
                case VariantType::kInt16: return getUInt8() / other.getInt16();
                case VariantType::kUInt16: return getUInt8() / other.getUInt16();
                case VariantType::kInt32: return getUInt8() / other.getInt32();
                case VariantType::kUInt32: return getUInt8() / other.getUInt32();
                case VariantType::kInt64: return getUInt8() / other.getInt64();
                case VariantType::kUInt64: return getUInt8() / other.getUInt64();
                case VariantType::kFloat: return getUInt8() / other.getFloat();
                case VariantType::kDouble: return getUInt8() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() / other.getInt8();
                case VariantType::kUInt8: return getInt16() / other.getUInt8();
                case VariantType::kInt16: return getInt16() / other.getInt16();
                case VariantType::kUInt16: return getInt16() / other.getUInt16();
                case VariantType::kInt32: return getInt16() / other.getInt32();
                case VariantType::kUInt32: return getInt16() / other.getUInt32();
                case VariantType::kInt64: return getInt16() / other.getInt64();
                case VariantType::kUInt64: return getInt16() / other.getUInt64();
                case VariantType::kFloat: return getInt16() / other.getFloat();
                case VariantType::kDouble: return getInt16() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() / other.getInt8();
                case VariantType::kUInt8: return getUInt16() / other.getUInt8();
                case VariantType::kInt16: return getUInt16() / other.getInt16();
                case VariantType::kUInt16: return getUInt16() / other.getUInt16();
                case VariantType::kInt32: return getUInt16() / other.getInt32();
                case VariantType::kUInt32: return getUInt16() / other.getUInt32();
                case VariantType::kInt64: return getUInt16() / other.getInt64();
                case VariantType::kUInt64: return getUInt16() / other.getUInt64();
                case VariantType::kFloat: return getUInt16() / other.getFloat();
                case VariantType::kDouble: return getUInt16() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() / other.getInt8();
                case VariantType::kUInt8: return getInt32() / other.getUInt8();
                case VariantType::kInt16: return getInt32() / other.getInt16();
                case VariantType::kUInt16: return getInt32() / other.getUInt16();
                case VariantType::kInt32: return getInt32() / other.getInt32();
                case VariantType::kUInt32: return getInt32() / other.getUInt32();
                case VariantType::kInt64: return getInt32() / other.getInt64();
                case VariantType::kUInt64: return getInt32() / other.getUInt64();
                case VariantType::kFloat: return getInt32() / other.getFloat();
                case VariantType::kDouble: return getInt32() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() / other.getInt8();
                case VariantType::kUInt8: return getUInt32() / other.getUInt8();
                case VariantType::kInt16: return getUInt32() / other.getInt16();
                case VariantType::kUInt16: return getUInt32() / other.getUInt16();
                case VariantType::kInt32: return getUInt32() / other.getInt32();
                case VariantType::kUInt32: return getUInt32() / other.getUInt32();
                case VariantType::kInt64: return getUInt32() / other.getInt64();
                case VariantType::kUInt64: return getUInt32() / other.getUInt64();
                case VariantType::kFloat: return getUInt32() / other.getFloat();
                case VariantType::kDouble: return getUInt32() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() / other.getInt8();
                case VariantType::kUInt8: return getInt64() / other.getUInt8();
                case VariantType::kInt16: return getInt64() / other.getInt16();
                case VariantType::kUInt16: return getInt64() / other.getUInt16();
                case VariantType::kInt32: return getInt64() / other.getInt32();
                case VariantType::kUInt32: return getInt64() / other.getUInt32();
                case VariantType::kInt64: return getInt64() / other.getInt64();
                case VariantType::kUInt64: return getInt64() / other.getUInt64();
                case VariantType::kFloat: return getInt64() / other.getFloat();
                case VariantType::kDouble: return getInt64() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() / other.getInt8();
                case VariantType::kUInt8: return getUInt64() / other.getUInt8();
                case VariantType::kInt16: return getUInt64() / other.getInt16();
                case VariantType::kUInt16: return getUInt64() / other.getUInt16();
                case VariantType::kInt32: return getUInt64() / other.getInt32();
                case VariantType::kUInt32: return getUInt64() / other.getUInt32();
                case VariantType::kInt64: return getUInt64() / other.getInt64();
                case VariantType::kUInt64: return getUInt64() / other.getUInt64();
                case VariantType::kFloat: return getUInt64() / other.getFloat();
                case VariantType::kDouble: return getUInt64() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        case VariantType::kFloat: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getFloat() / other.getInt8();
                case VariantType::kUInt8: return getFloat() / other.getUInt8();
                case VariantType::kInt16: return getFloat() / other.getInt16();
                case VariantType::kUInt16: return getFloat() / other.getUInt16();
                case VariantType::kInt32: return getFloat() / other.getInt32();
                case VariantType::kUInt32: return getFloat() / other.getUInt32();
                case VariantType::kInt64: return getFloat() / other.getInt64();
                case VariantType::kUInt64: return getFloat() / other.getUInt64();
                case VariantType::kFloat: return getFloat() / other.getFloat();
                case VariantType::kDouble: return getFloat() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        case VariantType::kDouble: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getDouble() / other.getInt8();
                case VariantType::kUInt8: return getDouble() / other.getUInt8();
                case VariantType::kInt16: return getDouble() / other.getInt16();
                case VariantType::kUInt16: return getDouble() / other.getUInt16();
                case VariantType::kInt32: return getDouble() / other.getInt32();
                case VariantType::kUInt32: return getDouble() / other.getUInt32();
                case VariantType::kInt64: return getDouble() / other.getInt64();
                case VariantType::kUInt64: return getDouble() / other.getUInt64();
                case VariantType::kFloat: return getDouble() / other.getFloat();
                case VariantType::kDouble: return getDouble() / other.getDouble();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for / operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for / operation");
        }
    }
}

Variant Variant::operator%(const Variant& other) const
{
    switch (getValueType()) {
        case VariantType::kInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt8() % other.getInt8();
                case VariantType::kUInt8: return getInt8() % other.getUInt8();
                case VariantType::kInt16: return getInt8() % other.getInt16();
                case VariantType::kUInt16: return getInt8() % other.getUInt16();
                case VariantType::kInt32: return getInt8() % other.getInt32();
                case VariantType::kUInt32: return getInt8() % other.getUInt32();
                case VariantType::kInt64: return getInt8() % other.getInt64();
                case VariantType::kUInt64: return getInt8() % other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for % operation");
                }
            }
            break;
        }
        case VariantType::kUInt8: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt8() % other.getInt8();
                case VariantType::kUInt8: return getUInt8() % other.getUInt8();
                case VariantType::kInt16: return getUInt8() % other.getInt16();
                case VariantType::kUInt16: return getUInt8() % other.getUInt16();
                case VariantType::kInt32: return getUInt8() % other.getInt32();
                case VariantType::kUInt32: return getUInt8() % other.getUInt32();
                case VariantType::kInt64: return getUInt8() % other.getInt64();
                case VariantType::kUInt64: return getUInt8() % other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for % operation");
                }
            }
            break;
        }
        case VariantType::kInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt16() % other.getInt8();
                case VariantType::kUInt8: return getInt16() % other.getUInt8();
                case VariantType::kInt16: return getInt16() % other.getInt16();
                case VariantType::kUInt16: return getInt16() % other.getUInt16();
                case VariantType::kInt32: return getInt16() % other.getInt32();
                case VariantType::kUInt32: return getInt16() % other.getUInt32();
                case VariantType::kInt64: return getInt16() % other.getInt64();
                case VariantType::kUInt64: return getInt16() % other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for % operation");
                }
            }
            break;
        }
        case VariantType::kUInt16: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt16() % other.getInt8();
                case VariantType::kUInt8: return getUInt16() % other.getUInt8();
                case VariantType::kInt16: return getUInt16() % other.getInt16();
                case VariantType::kUInt16: return getUInt16() % other.getUInt16();
                case VariantType::kInt32: return getUInt16() % other.getInt32();
                case VariantType::kUInt32: return getUInt16() % other.getUInt32();
                case VariantType::kInt64: return getUInt16() % other.getInt64();
                case VariantType::kUInt64: return getUInt16() % other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for % operation");
                }
            }
            break;
        }
        case VariantType::kInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt32() % other.getInt8();
                case VariantType::kUInt8: return getInt32() % other.getUInt8();
                case VariantType::kInt16: return getInt32() % other.getInt16();
                case VariantType::kUInt16: return getInt32() % other.getUInt16();
                case VariantType::kInt32: return getInt32() % other.getInt32();
                case VariantType::kUInt32: return getInt32() % other.getUInt32();
                case VariantType::kInt64: return getInt32() % other.getInt64();
                case VariantType::kUInt64: return getInt32() % other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for % operation");
                }
            }
            break;
        }
        case VariantType::kUInt32: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt32() % other.getInt8();
                case VariantType::kUInt8: return getUInt32() % other.getUInt8();
                case VariantType::kInt16: return getUInt32() % other.getInt16();
                case VariantType::kUInt16: return getUInt32() % other.getUInt16();
                case VariantType::kInt32: return getUInt32() % other.getInt32();
                case VariantType::kUInt32: return getUInt32() % other.getUInt32();
                case VariantType::kInt64: return getUInt32() % other.getInt64();
                case VariantType::kUInt64: return getUInt32() % other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for % operation");
                }
            }
            break;
        }
        case VariantType::kInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getInt64() % other.getInt8();
                case VariantType::kUInt8: return getInt64() % other.getUInt8();
                case VariantType::kInt16: return getInt64() % other.getInt16();
                case VariantType::kUInt16: return getInt64() % other.getUInt16();
                case VariantType::kInt32: return getInt64() % other.getInt32();
                case VariantType::kUInt32: return getInt64() % other.getUInt32();
                case VariantType::kInt64: return getInt64() % other.getInt64();
                case VariantType::kUInt64: return getInt64() % other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for % operation");
                }
            }
            break;
        }
        case VariantType::kUInt64: {
            switch (other.getValueType()) {
                case VariantType::kInt8: return getUInt64() % other.getInt8();
                case VariantType::kUInt8: return getUInt64() % other.getUInt8();
                case VariantType::kInt16: return getUInt64() % other.getInt16();
                case VariantType::kUInt16: return getUInt64() % other.getUInt16();
                case VariantType::kInt32: return getUInt64() % other.getInt32();
                case VariantType::kUInt32: return getUInt64() % other.getUInt32();
                case VariantType::kInt64: return getUInt64() % other.getInt64();
                case VariantType::kUInt64: return getUInt64() % other.getUInt64();
                default: {
                    throw VariantTypeCastError(m_valueType, other.getValueType(),
                            "Incompatible operator types for % operation");
                }
            }
            break;
        }
        default: {
            throw VariantTypeCastError(m_valueType, other.getValueType(),
                    "Incompatible operator types for % operation");
        }
    }
}

}  // namespace siodb::iomgr::dbengine
