// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Common project headers
#include <siodb/common/utils/Debug.h>

// Google Test
#include <gtest/gtest.h>

// In operator test with int32_t value type
TEST(InOperator, Int32)
{
    TestContext context;
    for (bool notIn : {true, false}) {
        std::int32_t value = 300;
        auto expr = makeIn<int>(value, {100, 200, 300}, notIn);
        auto result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), !notIn);

        value = 400;
        expr = makeIn<int>(value, {100, 200, 300}, notIn);
        result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), notIn);

        value = 100;
        expr = makeIn<int>(value, {100, 200, 300}, notIn);
        result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), !notIn);

        value = 50;
        expr = makeIn<int>(value, {100, 200, 300}, notIn);
        result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), notIn);

        value = 250;
        expr = makeIn<int>(value, {100, 200, 300}, notIn);
        result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), notIn);
    }
}

// In operator test with string value type
TEST(InOperator, String)
{
    TestContext context;
    for (bool notIn : {true, false}) {
        std::string value = "London";
        auto expr = makeIn<std::string>(value, {"London", "Berlin", "Vienna"}, notIn);
        auto result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), !notIn);

        value = "Beijing";
        expr = makeIn<std::string>(value, {"London", "Berlin", "Vienna"}, notIn);
        result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), notIn);

        value = "Berlin";
        expr = makeIn<std::string>(value, {"London", "Berlin", "Vienna"}, notIn);
        result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), !notIn);

        value = "Seoul";
        expr = makeIn<std::string>(value, {"London", "Berlin", "Vienna"}, notIn);
        result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), notIn);

        value = "Rome";
        expr = makeIn<std::string>(value, {"London", "Berlin", "Vienna"}, notIn);
        result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), notIn);
    }
}

// In operator test with binary value type
TEST(InOperator, Binary)
{
    TestContext context;
    for (bool notIn : {true, false}) {
        siodb::BinaryValue value = {0xFA, 0xA4, 0x13};
        const auto expr = makeIn<siodb::BinaryValue, siodb::BinaryValue>(
                value, {{0x13, 0xA4, 0xFA}, {0xA4, 0xFA, 0x13}, value}, notIn);
        const auto result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        EXPECT_EQ(result.getBool(), !notIn);
    }
}

TEST(InOperator, DateColumnExpressions)
{
    TestContext context;

    // columnDate contains 2019-12-19
    auto columnDate = std::make_unique<requests::SingleColumnExpression>("TestTbl", "Date");
    columnDate->setDatasetTableIndex(0);
    columnDate->setDatasetColumnIndex(4);
    const auto v1 = columnDate->evaluate(context);

    auto expr = makeInWithColumn<std::string>(
            std::move(columnDate), {"2019-11-19", "2019-12-19", "2019-12-18"}, false);

    expr->validate(context);
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result.isBool());
    EXPECT_EQ(result.getBool(), true);

    columnDate = std::make_unique<requests::SingleColumnExpression>("TestTbl", "Date");
    columnDate->setDatasetTableIndex(0);
    columnDate->setDatasetColumnIndex(4);
    expr = makeInWithColumn<std::string>(
            std::move(columnDate), {"2019-11-19", "2019-12-19", "2019zdazda"}, false);

    try {
        expr->validate(context);
        FAIL() << "Invalid date string wrongly accepted";
    } catch (std::exception& e) {
    }
}
