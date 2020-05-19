// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Int32IndexKeyTraits.h"

// Common project headers
#include <siodb/common/config/Config.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

std::size_t Int32IndexKeyTraits::getKeySize() const noexcept
{
    return 4;
}

void* Int32IndexKeyTraits::getMinKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0;
    k[1] = 0;
    k[2] = 0;
    k[3] = 0x80;
    return key;
}

void* Int32IndexKeyTraits::getMaxKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0xFF;
    k[1] = 0xFF;
    k[2] = 0xFF;
    k[3] = 0x7F;
    return key;
}

NumericKeyType Int32IndexKeyTraits::getNumericKeyType() const noexcept
{
    return NumericKeyType::kSignedInt;
}

int Int32IndexKeyTraits::compareKeys(const void* left, const void* right) noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    const auto a = *reinterpret_cast<const std::int32_t*>(left);
    const auto b = *reinterpret_cast<const std::int32_t*>(right);
#else
    std::int32_t a = 0, b = 0;
    ::pbeDecodeInt32(reinterpret_cast<const std::uint8_t*>(left), &a);
    ::pbeDecodeInt32(reinterpret_cast<const std::uint8_t*>(right), &b);
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

}  // namespace siodb::iomgr::dbengine
