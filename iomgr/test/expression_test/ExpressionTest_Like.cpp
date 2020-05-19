// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Google Test
#include <gtest/gtest.h>

// Like operator pattern matching test
TEST(LikeOperator, Matching)
{
    TestContext context;
    const std::string str = "ATestString";

    // Test: 'ATestString' LIKE 'A__%_g%'
    // EXPECT: Value matches the pattern
    std::string pattern = "A__%_g%";
    auto expr = makeLike(str, pattern, false);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Test: 'ATestString' LIKE '%stStr__%'
    // EXPECT: Value matches the pattern
    pattern = "%stStr__%";
    expr = makeLike(str, pattern, false);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Test: 'ATestString' LIKE '%in_'
    // EXPECT: Value matches the pattern
    pattern = "%in_";
    expr = makeLike(str, pattern, false);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Test: 'ATestString' LIKE 'ATestString'
    // EXPECT: Value matches the pattern
    pattern = "ATestString";
    expr = makeLike(str, pattern, false);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Test: 'ATestString' LIKE '%ATestString%'
    // EXPECT: Value matches the pattern
    pattern = "%ATestString%";
    expr = makeLike(str, pattern, false);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Test: 'ATestString' LIKE '___________'
    // EXPECT: Value matches the pattern
    pattern = "___________";
    expr = makeLike(str, pattern, false);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Test: 'ATestString' LIKE '__________T%'
    // EXPECT: Value does not match the pattern (no T in string after __________ symbols)
    pattern = "__________T%";
    expr = makeLike(str, pattern, false);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    // Test: 'ATestString' LIKE '%Z%'
    // EXPECT: Value does not match the pattern (Input string does not contain 'Z')
    pattern = "%Z%";
    expr = makeLike(str, pattern, false);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    // Test: 'ATestString' NOT LIKE '%Z%'
    // EXPECT: Value does not match the pattern (Pattern has less symbols than expected)
    pattern = "________";
    expr = makeLike(str, pattern, true);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Test: 'ATestString' NOT LIKE '%__ST______'
    // EXPECT: Value does not match the pattern (no "ST" upper case in string)
    pattern = "%__ST______";
    expr = makeLike(str, pattern, true);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Test: 'ATestString' NOT LIKE '%A'
    // EXPECT: Value does not match the pattern (ATestString does not end with 'A')
    pattern = "%A";
    expr = makeLike(str, pattern, true);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);
}

// Like operator pattern matching test with national symbols
TEST(LikeOperator, MultiLanguage)
{
    TestContext context;
    const std::string str = "EnglishРусский한국어";
    const std::string pattern = "%lish%__ки%_한국%";
    const auto expr = makeLike(str, pattern, false);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);
}
