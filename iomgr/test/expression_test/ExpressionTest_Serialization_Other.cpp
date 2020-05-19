// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "ExpressionSerializationTest.h"

// Common project headers
#include <siodb/common/utils/Debug.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace requests = dbengine::requests;

TEST(Serialization_Other, BetweenOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 11;
    const auto expr = makeBetween(35, 1, 10, true);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Other, CastOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 11;
    const auto expr = makeBinaryOperator<requests::CastOperator>("1", "INT");
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Other, ConcatenationOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 14;
    const auto expr = makeConcatenation("abc", "defg");
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Other, InOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 28;
    const auto expr = makeIn("xyz", {"abc", "defg", "xyz"}, true);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Other, IsOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 8;
    const auto expr = makeIs(1, 2, true);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Other, LikeOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 15;
    const auto expr = makeLike("abc", "defg", true);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Other, AllColumnsExpression)
{
    constexpr std::size_t kExpectedSerializedSize = 11;
    const auto expr = std::make_unique<requests::AllColumnsExpression>("CUSTOMERS");
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Other, SingleColumnExpression)
{
    constexpr std::size_t kExpectedSerializedSize = 22;
    const auto expr = std::make_unique<requests::SingleColumnExpression>("CUSTOMERS", "FIRST_NAME");
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Other, ListExpression)
{
    constexpr std::size_t kExpectedSerializedSize = 23;
    std::vector<requests::ExpressionPtr> items;
    items.push_back(makeConstant(1));
    items.push_back(makeConstant("hello"));
    items.push_back(makeConstant(5.0));
    const auto expr = std::make_unique<requests::ListExpression>(std::move(items));
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}
