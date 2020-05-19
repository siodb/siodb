// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Google Test
#include <gtest/gtest.h>

TEST(BetweenOperator, ValueInBounds)
{
    TestContext context;
    const std::int32_t exprValue = 12;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is in bounds
    auto expr = makeBetween(exprValue, v1, v2, false);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);
}

TEST(BetweenOperator, ValueLessThanLowerBound)
{
    TestContext context;
    const std::int32_t exprValue = -33;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is less than lower bound
    const auto expr = makeBetween(exprValue, v1, v2, false);
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);
}

TEST(BetweenOperator, ValueEqualToLowerBound)
{
    TestContext context;
    const std::int32_t exprValue = -32;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is exactly equal to lower bound
    const auto expr = makeBetween(exprValue, v1, v2, false);
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);
}

TEST(BetweenOperator, ValueIsGreaterThanHigherBound)
{
    TestContext context;
    const std::int32_t exprValue = 33;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is greater than higher bound
    const auto expr = makeBetween(exprValue, v1, v2, false);
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);
}

TEST(BetweenOperator, ValueEqualToHigherBound)
{
    TestContext context;
    const std::int32_t exprValue = 32;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is exactly equal to higher bound
    const auto expr = makeBetween(exprValue, v1, v2, false);
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);
}

// Test checks SQL NOT BETWEEN operator
TEST(NotBetweenOperator, ValueInBounds)
{
    TestContext context;
    const std::int32_t exprValue = 12;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is in bounds
    auto expr = makeBetween(exprValue, v1, v2, true);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);
}

TEST(NotBetweenOperator, ValueLessThanLowerBound)
{
    TestContext context;
    const std::int32_t exprValue = -33;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is less than lower bound
    const auto expr = makeBetween(exprValue, v1, v2, true);
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);
}

TEST(NotBetweenOperator, ValueEqualToLowerBound)
{
    TestContext context;
    const std::int32_t exprValue = -32;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is exactly equal to lower bound
    const auto expr = makeBetween(exprValue, v1, v2, true);
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);
}

TEST(NotBetweenOperator, ValueGreaterThanHigherBound)
{
    TestContext context;
    const std::int32_t exprValue = 33;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is greater than higher bound
    const auto expr = makeBetween(exprValue, v1, v2, true);
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);
}

TEST(NotBetweenOperator, ValueEqualToHigherBound)
{
    TestContext context;
    const std::int32_t exprValue = 32;
    const std::int32_t v1 = -32;
    const std::int32_t v2 = 32;

    // Value is exactly equal to higher bound
    const auto expr = makeBetween(exprValue, v1, v2, true);
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);
}
