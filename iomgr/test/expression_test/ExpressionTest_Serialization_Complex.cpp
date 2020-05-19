// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

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

TEST(Serialization_Complex, Test1)
{
    constexpr std::size_t kExpectedSerializedSize = 28 + 15 + 1;
    auto left = makeIn(dbengine::Variant("xyz"), {"abc", "defg", "xyz"}, true);
    auto right = makeLike("abc", "defg", true);
    const auto expr =
            std::make_unique<requests::LogicalOrOperator>(std::move(left), std::move(right));
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Complex, Test2)
{
    constexpr std::size_t kExpectedSerializedSize = 23;
    auto middle = makeBinaryOperator<requests::AddOperator>(3, 2);
    auto left = makeBinaryOperator<requests::AddOperator>(3, 2);
    auto right = makeBinaryOperator<requests::DivideOperator>(10, 5);
    const auto expr = std::make_unique<requests::BetweenOperator>(
            std::move(left), std::move(middle), std::move(right), false);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Complex, Test3)
{
    constexpr std::size_t kExpectedSerializedSize = 38;
    auto value = makeBinaryOperator<requests::ModuloOperator>(3, 2);
    std::vector<requests::ExpressionPtr> variants;
    variants.reserve(4);
    variants.push_back(makeBinaryOperator<requests::AddOperator>(3, 2));
    variants.push_back(makeBinaryOperator<requests::SubtractOperator>(3, 2));
    variants.push_back(makeBinaryOperator<requests::MultiplyOperator>(3, 2));
    variants.push_back(makeBinaryOperator<requests::DivideOperator>(3, 2));
    const auto expr =
            std::make_unique<requests::InOperator>(std::move(value), std::move(variants), true);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Complex, Test4)
{
    constexpr std::size_t kExpectedSerializedSize = 35;
    auto expr1 = std::make_unique<requests::EqualOperator>(
            std::make_unique<requests::SingleColumnExpression>("T1", "C1"), makeConstant(3));
    auto expr2 = std::make_unique<requests::NotEqualOperator>(
            std::make_unique<requests::SingleColumnExpression>("T2", "C2"), makeConstant(4));
    auto expr3 = std::make_unique<requests::GreaterOperator>(
            std::make_unique<requests::SingleColumnExpression>("T3", "C3"), makeConstant(5));
    auto expr4 = std::make_unique<requests::LogicalOrOperator>(std::move(expr2), std::move(expr3));
    const auto expr =
            std::make_unique<requests::LogicalAndOperator>(std::move(expr1), std::move(expr4));
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}
