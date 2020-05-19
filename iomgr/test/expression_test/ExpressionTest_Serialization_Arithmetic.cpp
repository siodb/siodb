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

TEST(Serialization_Arithmetic, AddOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::AddOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Arithmetic, SubtractOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::SubtractOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Arithmetic, MultiplyOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::MultiplyOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Arithmetic, DivideOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::DivideOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Arithmetic, ModuloOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::ModuloOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Arithmetic, UnaryMinusOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 4;
    const auto expr = makeUnaryOperator<requests::UnaryMinusOperator>(5);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Arithmetic, UnaryPlusOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 4;
    const auto expr = makeUnaryOperator<requests::UnaryPlusOperator>(5);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}
