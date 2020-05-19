// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UInt16IndexKeyTraits.h"

// Common project headers
#include <siodb/common/config/Config.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

std::size_t UInt16IndexKeyTraits::getKeySize() const noexcept
{
    return 2;
}

void* UInt16IndexKeyTraits::getMinKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0;
    k[1] = 0;
    return key;
}

void* UInt16IndexKeyTraits::getMaxKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0xFF;
    k[1] = 0xFF;
    return key;
}

NumericKeyType UInt16IndexKeyTraits::getNumericKeyType() const noexcept
{
    return NumericKeyType::kUnsignedInt;
}

int UInt16IndexKeyTraits::compareKeys(const void* left, const void* right) noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    const auto a = *reinterpret_cast<const std::uint16_t*>(left);
    const auto b = *reinterpret_cast<const std::uint16_t*>(right);
#else
    std::uint16_t a = 0, b = 0;
    ::pbeDecodeUInt16(reinterpret_cast<const std::uint8_t*>(left), &a);
    ::pbeDecodeUInt16(reinterpret_cast<const std::uint8_t*>(right), &b);
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

}  // namespace siodb::iomgr::dbengine
