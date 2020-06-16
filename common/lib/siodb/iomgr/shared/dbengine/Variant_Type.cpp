// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant_Type.h"

// STL headers
#include <array>

namespace siodb::iomgr::dbengine {

namespace {
/** Textual names of the Siodb column data types */
const std::array<const char*, kVariantTypeCount> kVariantTypeNames {
        "Null",
        "Bool",
        "Int8",
        "UInt8",
        "Int16",
        "UInt16",
        "Int32",
        "UInt32",
        "Int64",
        "UInt64",
        "Float",
        "Double",
        "Date",
        "Time",
        "DateTime",
        "TimeWithTZ",
        "DateTimeWithTZ",
        "DateInterval",
        "TimeInterval",
        "String",
        "Binary",
        "Clob",
        "Blob",
};

}  // namespace

const char* getVariantTypeName(VariantType type)
{
    return kVariantTypeNames.at(static_cast<std::size_t>(type));
}

const char* getVariantTypeName2(VariantType type) noexcept
{
    try {
        return getVariantTypeName(type);
    } catch (...) {
        return "<Unknown>";
    }
}

}  // namespace siodb::iomgr::dbengine
