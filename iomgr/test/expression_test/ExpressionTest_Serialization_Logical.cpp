// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "ExpressionSerializationTest.h"

// Common project headers
#include <siodb/common/utils/DebugMacros.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace requests = dbengine::requests;

TEST(Serialization_Bitwise, LogicalAndOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::LogicalAndOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Bitwise, LogicalOrOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::LogicalOrOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Bitwise, LogicalNotOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 4;
    const auto expr = makeUnaryOperator<requests::LogicalNotOperator>(5);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}
