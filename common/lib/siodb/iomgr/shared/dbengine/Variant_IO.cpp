// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant.h"

// STL headers
#include <sstream>

namespace siodb::iomgr::dbengine {

const char Variant::m_hexConversionTable[16] = {
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        'a',
        'b',
        'c',
        'd',
        'e',
        'f',
};

void Variant::dump(std::ostream& os) const
{
    os << '(' << getValueTypeName() << " <" << *this << ">)";
}

std::ostream& operator<<(std::ostream& os, const Variant& v)
{
    switch (v.m_valueType) {
        case VariantType::kNull: return os << "null";
        case VariantType::kBool: return os << (v.m_value.m_bool ? "true" : "false");
        case VariantType::kInt8: return os << static_cast<std::int16_t>(v.m_value.m_i8);
        case VariantType::kUInt8: return os << static_cast<std::uint16_t>(v.m_value.m_ui8);
        case VariantType::kInt16: return os << v.m_value.m_i16;
        case VariantType::kUInt16: return os << v.m_value.m_ui16;
        case VariantType::kInt32: return os << v.m_value.m_i32;
        case VariantType::kUInt32: return os << v.m_value.m_ui32;
        case VariantType::kInt64: return os << v.m_value.m_i64;
        case VariantType::kUInt64: return os << v.m_value.m_ui64;
        case VariantType::kFloat: return os << v.m_value.m_float;
        case VariantType::kDouble: return os << v.m_value.m_double;
        case VariantType::kDateTime: {
            return os << v.m_value.m_dt->format(Variant::kDefaultDateTimeFormat);
        }
        case VariantType::kString: return os << *v.m_value.m_string;
        case VariantType::kBinary: {
            os << "\\x'";
            for (auto p = v.m_value.m_binary->data(), pe = p + v.m_value.m_binary->size(); p != pe;
                    ++p) {
                const unsigned char v = *p;
                os << Variant::m_hexConversionTable[v >> 4]
                   << Variant::m_hexConversionTable[v & 15];
            }
            os << '\'';
            return os;
        }
        case VariantType::kClob: {
            return os << "(clob: len=" << v.m_value.m_clob->getSize()
                      << ", pos=" << v.m_value.m_clob->getPos() << ')';
        }
        case VariantType::kBlob: {
            return os << "(blob: len=" << v.m_value.m_blob->getSize()
                      << ", pos=" << v.m_value.m_blob->getPos() << ')';
        }
        default: return os << "(type " << static_cast<int>(v.m_valueType) << ')';
    }
}

}  // namespace siodb::iomgr::dbengine
