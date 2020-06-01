// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ExpressionSerializationTest.h"

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace requests = dbengine::requests;

constexpr std::size_t kExtraBufferSize = 16;

void testExpressionSerialization(
        const requests::Expression& expr, const std::size_t expectedSerializedSize)
{
    const auto serializedSize = expr.getSerializedSize();
    ASSERT_EQ(serializedSize, expectedSerializedSize);

    stdext::buffer<std::uint8_t> buffer(expectedSerializedSize + kExtraBufferSize, 0xCD);
    const auto end = expr.serializeUnchecked(buffer.data());
    const auto actualSize = static_cast<std::size_t>(end - buffer.data());
    ASSERT_EQ(actualSize, expectedSerializedSize);

    requests::ExpressionPtr dest;
    const auto consumed = requests::Expression::deserialize(buffer.data(), buffer.size(), dest);
    ASSERT_EQ(consumed, expectedSerializedSize);
    ASSERT_EQ(*dest, expr);
}
