// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Google Test
#include <gtest/gtest.h>

// Test check correctess of string comparison
TEST(Comparisons, CompareString1)
{
    TestContext context;
    const std::string s1 = "abc";

    // Checks "abc" == "abc"
    auto expr = makeEqual(s1, s1);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Checks "abc" > "bca" (false)
    std::string s2 = "bca";
    expr = makeGreater(s1, s2);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    // Checks "bca" > "abc" (true)
    expr = makeGreater(s2, s1);

    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Checks "abc" >= "bca" (false)
    expr = makeGreaterOrEqual(s1, s2);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    // Checks "abc" < "abc0"
    s2 = s1 + "0";
    expr = makeLess(s1, s2);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Checks "abc0" < "abc" (false)
    expr = makeLess(s2, s1);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    // Checks "abc" <= "abc0"
    expr = makeLessOrEqual(s1, s2);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Checks "abc0" <= "abc"
    expr = makeLessOrEqual(s2, s1);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);
}

// Test check correctess of date and valid date string comparison
TEST(Comparisons, CompareDateAndString)
{
    TestContext context;
    const auto date = siodb::RawDateTime("2019-12-23", siodb::RawDateTime::kDefaultDateFormat);
    const std::string str = "2019-12-24";

    auto expr = makeEqual(date, str);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    expr = makeLess(date, str);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    expr = makeLessOrEqual(date, str);
    ASSERT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    expr = makeGreater(date, str);
    EXPECT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);

    expr = makeGreaterOrEqual(date, str);
    EXPECT_TRUE(isBoolType(expr->getResultValueType(context)));
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), false);
}

// Test check correctess of date and invalid date string comparison
TEST(Comparisons, CompareDateAndInvalidDateString)
{
    TestContext context;
    const auto date = siodb::RawDateTime("2019-12-23", siodb::RawDateTime::kDefaultDateFormat);
    const std::string str = "2019-Invalid";

    auto expr = makeEqual(date, str);
    try {
        expr->evaluate(context);
        FAIL();
    } catch (std::exception& e) {
        // OK exception is expected. 2019Invalid is invalid date
    }

    expr = makeLess(date, str);
    try {
        expr->evaluate(context);
        FAIL();
    } catch (std::exception& e) {
    }

    expr = makeLessOrEqual(date, str);
    try {
        expr->evaluate(context);
        FAIL();
    } catch (std::exception& e) {
    }

    expr = makeGreater(date, str);
    try {
        expr->evaluate(context);
        FAIL();
    } catch (std::exception& e) {
    }

    expr = makeGreaterOrEqual(date, str);
    try {
        expr->evaluate(context);
        FAIL();
    } catch (std::exception& e) {
    }
}

// Test check correctess of string comparison with national symbols
TEST(Comparisons, CompareString_MultiLanguage)
{
    TestContext context;
    const std::string s1 = "abcабв";
    const std::string s2 = "абвabc";

    // Checks abcабв == abcабв
    auto expr = makeEqual(s1, s1);
    expr->validate(context);
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Checks абвabc > abcабв
    expr = makeGreater(s2, s1);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Checks абвabc >= abcабв
    expr = makeGreaterOrEqual(s2, s1);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Checks абвabc < abcабв
    expr = makeLess(s1, s2);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    // Checks абвabc <= abcабв
    expr = makeLessOrEqual(s1, s2);
    expr->validate(context);
    result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);
}
