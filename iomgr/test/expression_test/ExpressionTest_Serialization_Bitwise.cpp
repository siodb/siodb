// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "ExpressionSerializationTest.h"
#include "TestContext.h"

// Common project headers
#include <siodb/common/utils/Debug.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace requests = dbengine::requests;

TEST(Serialization_Bitwise, BitwiseAndOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::BitwiseAndOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Bitwise, BitwiseOrOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::BitwiseOrOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Bitwise, BitwiseXorOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::BitwiseXorOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Bitwise, ComplementOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 4;
    const auto expr = makeUnaryOperator<requests::ComplementOperator>(5);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Bitwise, LeftShiftOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::LeftShiftOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Bitwise, RightShiftOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::RightShiftOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}
