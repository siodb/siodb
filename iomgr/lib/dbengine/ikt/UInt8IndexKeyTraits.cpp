// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UInt8IndexKeyTraits.h"

namespace siodb::iomgr::dbengine {

std::size_t UInt8IndexKeyTraits::getKeySize() const noexcept
{
    return 1;
}

void* UInt8IndexKeyTraits::getMinKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0x80;
    return key;
}

void* UInt8IndexKeyTraits::getMaxKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0x7F;
    return key;
}

NumericKeyType UInt8IndexKeyTraits::getNumericKeyType() const noexcept
{
    return NumericKeyType::kUnsignedInt;
}

int UInt8IndexKeyTraits::compareKeys(const void* left, const void* right) noexcept
{
    const auto a = *reinterpret_cast<const std::uint8_t*>(left);
    const auto b = *reinterpret_cast<const std::uint8_t*>(right);
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

}  // namespace siodb::iomgr::dbengine
