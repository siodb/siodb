// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant.h"

// STL headers
#include <sstream>

namespace siodb::iomgr::dbengine {

const Variant Variant::m_nullValue;
const Variant Variant::m_emptyStringValue = std::string();

Variant::Variant(const Variant& src)
    : m_valueType(src.m_valueType)
{
    switch (m_valueType) {
        case VariantType::kString: {
            m_value.m_string = new std::string(*src.m_value.m_string);
            return;
        }
        case VariantType::kBinary: {
            m_value.m_binary = new BinaryValue(*src.m_value.m_binary);
            return;
        }
        case VariantType::kClob: {
            m_value.m_clob = src.m_value.m_clob->clone();
            if (m_value.m_clob) return;
            break;
        }
        case VariantType::kBlob: {
            m_value.m_blob = src.m_value.m_blob->clone();
            if (m_value.m_blob) return;
            break;
        }
        case VariantType::kDateTime: {
            m_value.m_dt = new RawDateTime(*src.m_value.m_dt);
            return;
        }
        default: {
            // m_ptr size is equal to largest date type
            m_value.m_ptr = src.m_value.m_ptr;
            return;
        }
    }
    std::ostringstream err;
    err << "Can't copy variant value of type " << static_cast<int>(m_valueType);
    throw std::invalid_argument(err.str());
}

Variant::Variant(const char* value)
    : m_valueType(VariantType::kString)
{
    if (SIODB_UNLIKELY(value == nullptr)) throw std::invalid_argument("null c-string value");
    m_value.m_string = new std::string(value);
}

Variant::Variant(std::string* value)
    : m_valueType(VariantType::kString)
{
    if (SIODB_UNLIKELY(value == nullptr)) throw std::invalid_argument("null string value");
    m_value.m_string = value;
}

Variant::Variant(const void* value, std::size_t size)
    : m_valueType(VariantType::kBinary)
{
    if (SIODB_UNLIKELY(value == nullptr)) throw std::invalid_argument("null binary value");
    auto v = std::make_unique<BinaryValue>(size);
    if (size > 0) std::memcpy(v->data(), value, size);
    m_value.m_binary = v.release();
}

Variant::Variant(BinaryValue* value)
    : m_valueType(VariantType::kBinary)
{
    if (SIODB_UNLIKELY(value == nullptr)) throw std::invalid_argument("null binary value");
    m_value.m_binary = value;
}

Variant::Variant(ClobStream* value)
    : m_valueType(VariantType::kClob)
{
    if (SIODB_UNLIKELY(value == nullptr)) throw std::invalid_argument("null clob stream");
    m_value.m_clob = value;
}

Variant::Variant(BlobStream* value)
    : m_valueType(VariantType::kBlob)
{
    if (SIODB_UNLIKELY(value == nullptr)) throw std::invalid_argument("null blob stream");
    m_value.m_blob = value;
}

}  // namespace siodb::iomgr::dbengine
