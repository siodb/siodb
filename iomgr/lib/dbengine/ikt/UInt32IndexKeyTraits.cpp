// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UInt32IndexKeyTraits.h"

// Common project headers
#include <siodb/common/config/Config.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

std::size_t UInt32IndexKeyTraits::getKeySize() const noexcept
{
    return 4;
}

void* UInt32IndexKeyTraits::getMinKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0;
    k[1] = 0;
    k[2] = 0;
    k[3] = 0;
    return key;
}

void* UInt32IndexKeyTraits::getMaxKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0xFF;
    k[1] = 0xFF;
    k[2] = 0xFF;
    k[3] = 0xFF;
    return key;
}

NumericKeyType UInt32IndexKeyTraits::getNumericKeyType() const noexcept
{
    return NumericKeyType::kUnsignedInt;
}

int UInt32IndexKeyTraits::compareKeys(const void* left, const void* right) noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    const auto a = *reinterpret_cast<const std::uint32_t*>(left);
    const auto b = *reinterpret_cast<const std::uint32_t*>(right);
#else
    std::uint32_t a = 0, b = 0;
    ::pbeDecodeUInt32(reinterpret_cast<const std::uint8_t*>(left), &a);
    ::pbeDecodeUInt32(reinterpret_cast<const std::uint8_t*>(right), &b);
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

}  // namespace siodb::iomgr::dbengine
