// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Int8IndexKeyTraits.h"

namespace siodb::iomgr::dbengine {

std::size_t Int8IndexKeyTraits::getKeySize() const noexcept
{
    return 1;
}

void* Int8IndexKeyTraits::getMinKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0x80;
    return key;
}

void* Int8IndexKeyTraits::getMaxKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0x7F;
    return key;
}

NumericKeyType Int8IndexKeyTraits::getNumericKeyType() const noexcept
{
    return NumericKeyType::kSignedInt;
}

int Int8IndexKeyTraits::compareKeys(const void* left, const void* right) noexcept
{
    const auto a = *reinterpret_cast<const std::int8_t*>(left);
    const auto b = *reinterpret_cast<const std::int8_t*>(right);
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

}  // namespace siodb::iomgr::dbengine
