// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Google Test
#include <gtest/gtest.h>

// Concatenation test, string || string: expect string
TEST(ConcatenationOperator, String_String)
{
    TestContext context;
    const std::string v1 = "abc";
    const std::string v2 = "bca";
    const auto expr = makeConcatenation(v1, v2);
    EXPECT_TRUE(isStringType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kString);
    ASSERT_EQ(result.getString(), "abcbca");
}

// Concatenation test, string || u16: expect string
TEST(ConcatenationOperator, String_U16)
{
    TestContext context;
    const std::string v1 = "abc";
    const std::uint16_t v2 = 1U;
    const auto expr = makeConcatenation(v1, v2);
    EXPECT_TRUE(isStringType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kString);
    ASSERT_EQ(result.getString(), "abc1");
}

// Concatenation test, string || u16: expect string
TEST(ConcatenationOperator, U16_String)
{
    TestContext context;
    const std::uint16_t v1 = 1U;
    const std::string v2 = "abc";
    const auto expr = makeConcatenation(v1, v2);
    EXPECT_TRUE(isStringType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kString);
    ASSERT_EQ(result.getString(), "1abc");
}

// Concatenation test, float || u16: expect string
TEST(ConcatenationOperator, Float_U16)
{
    TestContext context;
    const float v1 = 123.0f;
    const std::uint16_t v2 = 512u;
    const auto expr = makeConcatenation(v1, v2);
    EXPECT_TRUE(isStringType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kString);
    ASSERT_EQ(result.getString(), "123.00000000512");
}
