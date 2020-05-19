// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Int16IndexKeyTraits.h"

// Common project headers
#include <siodb/common/config/Config.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

std::size_t Int16IndexKeyTraits::getKeySize() const noexcept
{
    return 2;
}

void* Int16IndexKeyTraits::getMinKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0;
    k[1] = 0x80;
    return key;
}

void* Int16IndexKeyTraits::getMaxKey(void* key) const noexcept
{
    std::uint8_t* k = static_cast<std::uint8_t*>(key);
    k[0] = 0xFF;
    k[1] = 0x7F;
    return key;
}

NumericKeyType Int16IndexKeyTraits::getNumericKeyType() const noexcept
{
    return NumericKeyType::kSignedInt;
}

int Int16IndexKeyTraits::compareKeys(const void* left, const void* right) noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    const auto a = *reinterpret_cast<const std::int16_t*>(left);
    const auto b = *reinterpret_cast<const std::int16_t*>(right);
#else
    std::int16_t a = 0, b = 0;
    ::pbeDecodeInt16(reinterpret_cast<const std::uint8_t*>(left), &a);
    ::pbeDecodeInt16(reinterpret_cast<const std::uint8_t*>(right), &b);
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

}  // namespace siodb::iomgr::dbengine
