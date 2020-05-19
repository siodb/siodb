// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Google Test
#include <gtest/gtest.h>

// ASSERT vs EXPECT
// https://stackoverflow.com/a/2565309/1540501

// Add operator test, 255(u8) + 1(u16) -> Expects i32(256)
TEST(AddOperator, U8_U16)
{
    TestContext context;
    const std::uint8_t v1 = 255;
    const std::uint16_t v2 = 1;
    const auto expr = makeAddition(v1, v2);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), 256);
}

// Add operator test, 255000000000000(u64) + -10234334532453(d)
TEST(AddOperator, U64_Double)
{
    TestContext context;
    const std::uint64_t v1 = 255000000000000ULL;
    const double v2 = -10234334532453.0;
    const auto expr = makeAddition(v1, v2);
    ASSERT_FALSE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kDouble);
    ASSERT_EQ(result.getDouble(), double(v1 + v2));
}

// Add operator test, "ABC"(S) + "XYZ"
TEST(AddOperator, String_String)
{
    TestContext context;
    const std::string v1 = "ABC";
    const std::string v2 = "XYZ";
    const auto expr = makeAddition(v1, v2);
    ASSERT_TRUE(isStringType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kString);
    ASSERT_EQ(result.getString(), std::string("ABCXYZ"));
}

// Substract operator test, 255(u8) - 1(u16)
TEST(SubstractOperator, U8_U16)
{
    TestContext context;
    const std::uint8_t v1 = 255;
    const std::uint16_t v2 = 1;
    const auto expr = makeSubstraction(v1, v2);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), 254);
}

// Substract operator test, 255000000000000(u64) - -10234334532453(d): expect double
TEST(SubstractOperator, U64_Double)
{
    TestContext context;
    const std::uint64_t v1 = 255000000000000ULL;
    const double v2 = -10234334532453.0;
    const auto expr = makeSubstraction(v1, v2);
    ASSERT_FALSE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kDouble);
    ASSERT_EQ(result.getDouble(), double(v1 - v2));
}

// Division operator test, 255(u8) / 1(u16): expect int32
TEST(DivideOperator, U8_U16)
{
    TestContext context;
    const std::uint8_t v1 = 255;
    const std::uint16_t v2 = 1;
    const auto expr = makeDivision(v1, v2);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), 255);
}

// Multiplication operator test, 255(u8) * 1(d): expect int32
TEST(MultiplicationOperator, U8_U16)
{
    TestContext context;
    const std::uint8_t v1 = 255;
    const std::uint16_t v2 = 1;
    const auto expr = makeMultiplication(v1, v2);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), 255);
}

// Multiplication operator test, 255000000000000(u64) * -10234334532453(d): expect double
TEST(MultiplicationOperator, U64_Double)
{
    TestContext context;
    const std::uint64_t v1 = 255000000000000ULL;
    const double v2 = -10234334532453.0;
    const auto expr = makeMultiplication(v1, v2);
    ASSERT_FALSE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kDouble);
    ASSERT_EQ(result.getDouble(), double(v1 * v2));
}

// Multiplication operator test, 23.0000000001(double) * 3(uint8): expect double
TEST(MultiplicationOperator, Double_U8)
{
    TestContext context;
    const double v1 = 23.0000000001;
    const std::uint8_t v2 = 3;

    const auto expr = makeMultiplication(v1, v2);
    ASSERT_FALSE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kDouble);
    ASSERT_EQ(result.getDouble(), double(v1 * v2));
}

// Division operator test, 255000000000000(u64) / -10234334532453(d): expect double
TEST(DivideOperator, U64_Double)
{
    TestContext context;
    const std::uint64_t v1 = 255000000000000ULL;
    const double v2 = -10234334532453.0;
    const auto expr = makeDivision(v1, v2);
    ASSERT_FALSE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kDouble);
    ASSERT_EQ(result.getDouble(), double(v1 / v2));
}

// Modulo operator test, 255(u8) % 2(i32)
TEST(ModuloOperator, U8_I32)
{
    TestContext context;
    const std::uint8_t v1 = 255;
    const std::int32_t v2 = 2;
    const auto expr = makeModulo(v1, v2);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), 1);
}

// Unary plus operator test, +(int8(-4) -> Expects i32(-4)
TEST(UnaryPlusOperator, Int8)
{
    TestContext context;
    const std::int8_t v1 = -4;
    const auto expr = makeUnaryPlus(v1);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), v1);
}

// Unary plus operator test, +(int8(-4) -> Expects i32(-4)
TEST(UnaryPlusOperator, UInt8)
{
    TestContext context;
    const std::int8_t v1 = 4;
    const auto expr = makeUnaryPlus(v1);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), v1);
}

// Unary plus operator test, +(float(-4.0f) -> Expects float(-4.0f)
TEST(UnaryPlusOperator, Float)
{
    TestContext context;
    const float v1 = -4.0f;
    const auto expr = makeUnaryPlus(v1);
    ASSERT_FALSE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kFloat);
    ASSERT_EQ(result.getFloat(), v1);
}

// Unary plus operator test, +(float(-4.0f) -> Expects float(-4.0f)
TEST(UnaryPlusOperator, Double)
{
    TestContext context;
    const double v1 = -4.0;
    const auto expr = makeUnaryPlus(v1);
    ASSERT_FALSE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kDouble);
    ASSERT_EQ(result.getDouble(), v1);
}

// Unary minus operator test, -(int8(-4) -> Expects i32(4)
TEST(UnaryMinusOperator, Int8)
{
    TestContext context;
    const std::int8_t v1 = -4;
    const auto expr = makeUnaryMinus(v1);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), -v1);
}

// Unary minus operator test, -(uint8(4) -> Expects i32(-4)
TEST(UnaryMinusOperator, UInt8)
{
    TestContext context;
    const std::uint8_t v1 = 4;
    const auto expr = makeUnaryMinus(v1);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), -v1);
}

// Unary minus operator test, -(float(-4) -> Expects float(4)
TEST(UnaryMinusOperator, Float)
{
    TestContext context;
    const float v1 = -4.0f;
    const auto expr = makeUnaryMinus(v1);
    ASSERT_FALSE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kFloat);
    ASSERT_EQ(result.getFloat(), -v1);
}

// Unary minus operator test, -(double(-4) -> Expects double(4)
TEST(UnaryMinusOperator, Double)
{
    TestContext context;
    const double v1 = -4.0;
    const auto expr = makeUnaryMinus(v1);
    ASSERT_FALSE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kDouble);
    ASSERT_EQ(result.getDouble(), -v1);
}
