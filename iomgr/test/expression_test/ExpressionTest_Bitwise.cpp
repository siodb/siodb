// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Google Test
#include <gtest/gtest.h>

// Bitwise or operator test, 4123(u32) | 8124(u16): expect u32
TEST(BitwiseOrOperator, BitwiseOrOperator)
{
    TestContext context;
    const std::uint32_t v1 = 4123;
    const std::uint16_t v2 = 8124;
    const auto expr = makeBitwiseOr(v1, v2);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kUInt32);
    ASSERT_EQ(result.getUInt32(), v1 | v2);
}

// Bitwise and operator test, 4123(u32) & 8124(u16): expect u32
TEST(BitwiseAndOperator, U32_U16)
{
    TestContext context;
    const std::uint32_t v1 = 4123;
    const std::uint16_t v2 = 8245;
    const auto expr = makeBitwiseAnd(v1, v2);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kUInt32);
    ASSERT_EQ(result.getUInt32(), v1 & v2);
}

// Complement operator test, ui16(~12465) -> Expects i32(~12465)
TEST(ComplementOperator, UInt16)
{
    TestContext context;
    const std::uint16_t v1 = 12465;
    const auto expr = makeComplement(v1);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    ASSERT_EQ(result.getInt32(), ~12465);
}

// Left shift operator test, 4(u32) << 2(u16): expect u32
TEST(LeftShiftOperator, U32_U16)
{
    TestContext context;
    const std::uint32_t v1 = 4;
    const std::uint16_t v2 = 2;
    const auto expr = makeLeftShift(v1, v2);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kUInt32);
    ASSERT_EQ(result.getUInt32(), v1 << v2);
}

// Right shift operator test, 4(u32) >> 2(u16): expect u32
TEST(RightShiftOperator, U32_U16)
{
    TestContext context;
    const std::uint32_t v1 = 4;
    const std::uint16_t v2 = 2;
    const auto expr = makeRightShift(v1, v2);
    ASSERT_TRUE(isIntegerType(expr->getResultValueType(context)));
    ASSERT_TRUE(isNumericType(expr->getResultValueType(context)));
    expr->validate(context);
    const auto result = expr->evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kUInt32);
    ASSERT_EQ(result.getUInt32(), v1 >> v2);
}
