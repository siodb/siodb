// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Google Test
#include <gtest/gtest.h>

TEST(Constants, Test1)
{
    TestContext context;

    requests::ConstantExpression uint8Value(std::uint8_t(1));
    ASSERT_TRUE(isIntegerType(uint8Value.getResultValueType(context)));
    ASSERT_TRUE(isNumericType(uint8Value.getResultValueType(context)));

    requests::ConstantExpression int32Value(std::int32_t(1));
    ASSERT_TRUE(isIntegerType(int32Value.getResultValueType(context)));
    ASSERT_TRUE(isNumericType(int32Value.getResultValueType(context)));

    requests::ConstantExpression floatValue(1.0f);
    ASSERT_FALSE(isIntegerType(floatValue.getResultValueType(context)));
    ASSERT_TRUE(isNumericType(floatValue.getResultValueType(context)));

    requests::ConstantExpression doubleValue(-3462.283712345678234523445);
    ASSERT_FALSE(isIntegerType(doubleValue.getResultValueType(context)));
    ASSERT_TRUE(isNumericType(doubleValue.getResultValueType(context)));
    ASSERT_EQ(doubleValue.evaluate(context), -3462.283712345678234523445);

    requests::ConstantExpression stringValue(std::string("ABC"));
    ASSERT_TRUE(isStringType(stringValue.getResultValueType(context)));

    requests::ConstantExpression boolValue(true);
    ASSERT_TRUE(isBoolType(boolValue.getResultValueType(context)));

    siodb::RawDateTime dt;
    requests::ConstantExpression dateTimeValue(dt);
    ASSERT_TRUE(isDateTimeType(dateTimeValue.getResultValueType(context)));
}

// Test for NULL result
TEST(Constants, NullResult)
{
    TestContext context;

    for (const auto& expr : {
                 makeComplement(nullptr),
                 makeUnaryMinus(nullptr),
                 makeNot(nullptr),
                 makeAnd(true, nullptr),
                 makeAnd(nullptr, true),
                 makeAnd(nullptr, nullptr),
                 makeConcatenation(std::string(), nullptr),
                 makeConcatenation(nullptr, std::string()),
                 makeConcatenation(nullptr, nullptr),
                 makeSubstraction(1, nullptr),
                 makeSubstraction(nullptr, 1),
                 makeSubstraction(nullptr, nullptr),
                 makeAddition(1, nullptr),
                 makeAddition(nullptr, 1),
                 makeAddition(nullptr, nullptr),
                 makeLeftShift(1, nullptr),
                 makeLeftShift(nullptr, 1),
                 makeLeftShift(nullptr, nullptr),
         }) {
        const auto resultType = expr->getResultValueType(context);
        ASSERT_TRUE(dbengine::isNullType(resultType));
        expr->validate(context);
    }

    for (auto& expr : {
                 makeEqual(1, nullptr),
                 makeEqual(nullptr, 1),
                 makeEqual(nullptr, nullptr),
                 makeLessOrEqual(1, nullptr),
                 makeLessOrEqual(nullptr, 1),
                 makeLessOrEqual(nullptr, nullptr),
                 makeBetween(nullptr, 1, 1, false),
                 makeBetween(1, nullptr, 1, false),
                 makeBetween(1, 1, nullptr, false),
                 makeBetween(nullptr, nullptr, 1, false),
                 makeBetween(nullptr, 1, nullptr, false),
                 makeBetween(1, nullptr, nullptr, false),
                 makeBetween(nullptr, nullptr, nullptr, false),
                 makeIn<std::nullptr_t, int>(nullptr, {1, 2, 3}, false),
                 makeLike(std::string(), nullptr, false),
                 makeLike(nullptr, std::string(), false),
                 makeLike(nullptr, nullptr, false),
         }) {
        const auto resultType = expr->getResultValueType(context);
        ASSERT_FALSE(isNullType(resultType));
        expr->validate(context);
    }

    auto nullColumn = std::make_unique<requests::SingleColumnExpression>("TestTbl", "NullCollumn");
    nullColumn->setDatasetTableIndex(0);
    nullColumn->setDatasetColumnIndex(5);

    ASSERT_TRUE(isNullType(nullColumn->getResultValueType(context)));
    nullColumn->validate(context);

    requests::ConstantExpression constantExpr;
    ASSERT_TRUE(isNullType(constantExpr.getResultValueType(context)));
    constantExpr.validate(context);

    for (auto& expr : {
                 makeIs(1, 1, false),
                 makeIs(1, nullptr, false),
                 makeIs(nullptr, 1, false),
                 makeIs(nullptr, nullptr, false),
         }) {
        const auto resultType = expr->getResultValueType(context);
        ASSERT_FALSE(dbengine::isNullType(resultType));
        expr->validate(context);
    }
}
