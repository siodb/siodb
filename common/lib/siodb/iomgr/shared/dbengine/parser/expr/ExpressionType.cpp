// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ExpressionType.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

namespace siodb::iomgr::dbengine::requests {

unsigned getExpressionTypeSerializedSize(ExpressionType expressionType) noexcept
{
    return ::getVarIntSize(static_cast<std::uint32_t>(expressionType));
}

std::uint8_t* serializeExpressionTypeUnchecked(
        ExpressionType expressionType, std::uint8_t* buffer) noexcept
{
    return ::encodeVarInt(static_cast<std::uint32_t>(expressionType), buffer);
}

}  // namespace siodb::iomgr::dbengine::requests
