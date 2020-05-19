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

TEST(Serialization_Comarisons, EqualOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::EqualOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Comarisons, NotEqualOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::NotEqualOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Comarisons, LessOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::LessOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Comarisons, LessOrEqualOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::LessOrEqualOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Comarisons, GreaterOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::GreaterOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}

TEST(Serialization_Comarisons, GreaterOrEqualOperator)
{
    constexpr std::size_t kExpectedSerializedSize = 7;
    const auto expr = makeBinaryOperator<requests::GreaterOrEqualOperator>(1, 2);
    testExpressionSerialization(*expr, kExpectedSerializedSize);
}
