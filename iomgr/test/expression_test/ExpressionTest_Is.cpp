// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Common project headers
#include <siodb/common/utils/DebugMacros.h>

// Google Test
#include <gtest/gtest.h>

// IS operator test
TEST(IsOperator, IsOperator)
{
    TestContext context;
    auto expr = makeIs(1, nullptr, false);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    expr = makeIs(nullptr, 1, false);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    expr = makeIs(1, 1, false);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    expr = makeIs(nullptr, nullptr, false);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);
}

// IS NOT operator test
TEST(IsOprator, IsNot)
{
    TestContext context;
    auto expr = makeIs(1, nullptr, true);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    expr = makeIs(nullptr, 1, true);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    expr = makeIs(1, 1, true);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    expr = makeIs(nullptr, nullptr, true);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);
}
