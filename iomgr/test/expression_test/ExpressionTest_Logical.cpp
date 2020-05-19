// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Google Test
#include <gtest/gtest.h>

// Logical AND operator test with known values
TEST(AndOperator, KnownValues)
{
    TestContext context;
    using A = std::array<bool, 3>;
    const std::array<A, 4> values {
            A {false, false, false},
            A {false, true, false},
            A {true, false, false},
            A {true, true, true},
    };
    for (const auto& v : values) {
        const auto expr = makeAnd(v[0], v[1]);
        EXPECT_TRUE(isBoolType(expr->getResultValueType(context)));
        expr->validate(context);
        const auto result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        ASSERT_EQ(result.getBool(), v[2]);
    }
}

// Logical OR operator test with known values
TEST(OrOperator, KnownValues)
{
    TestContext context;
    using A = std::array<bool, 3>;
    const std::array<A, 4> values {
            A {false, false, false},
            A {false, true, true},
            A {true, false, true},
            A {true, true, true},
    };
    for (const auto& v : values) {
        const auto expr = makeOr(v[0], v[1]);
        EXPECT_TRUE(isBoolType(expr->getResultValueType(context)));
        expr->validate(context);
        const auto result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        ASSERT_EQ(result.getBool(), v[2]);
    }
}

// Logical NOT operator test with known values
TEST(LogicalNotOperator, KnownValues)
{
    TestContext context;
    const std::array<bool, 2> values {true, false};
    for (const auto v : values) {
        const auto expr = makeNot(v);
        EXPECT_TRUE(isBoolType(expr->getResultValueType(context)));
        expr->validate(context);
        const auto result = expr->evaluate(context);
        ASSERT_TRUE(result.isBool());
        ASSERT_EQ(result.getBool(), !v);
    }
}
