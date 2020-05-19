// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant.h"

// Common project headers
#include <siodb/common/utils/PlainBinaryEncoding.h>

// CRT headers
#include <cstdio>

// STL headers
#include <sstream>

namespace siodb::iomgr::dbengine {

void Variant::clear() noexcept
{
    switch (m_valueType) {
        case VariantType::kString: {
            delete m_value.m_string;
            break;
        }
        case VariantType::kBinary: {
            delete m_value.m_binary;
            break;
        }
        case VariantType::kClob: {
            delete m_value.m_clob;
            break;
        }
        case VariantType::kBlob: {
            delete m_value.m_blob;
            break;
        }
        case VariantType::kDateTime: {
            delete m_value.m_dt;
            break;
        }
        default: break;
    }
    m_valueType = VariantType::kNull;
}

Variant Variant::concatenate(const Variant& other) const
{
    if (isString()) {
        if (other.isString())
            return getString() + other.getString();
        else
            return getString() + *other.asString();
    } else {
        if (other.isString())
            return *asString() + other.getString();
        else
            return *asString() + *other.asString();
    }
}

}  // namespace siodb::iomgr::dbengine
