// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "TestContext.h"

// Google Test
#include <gtest/gtest.h>

// requests::SingleColumnExpression test
TEST(ColumnExpression, Test1)
{
    // TestContext contains following column values for the table TestTbl:
    // uint64_t TRID: 1
    // std::string ADDRESS: 121 Anselmo str.
    // std::int32_t COUNT: -25
    // double LEVEL: 1230.0165432
    // DateTime: DATE 2019-12-19;
    TestContext context;

    // Create TRID column expression, read the value check it's type and value
    requests::SingleColumnExpression columnTrid("TestTbl", "TRID");
    columnTrid.setSingleDatasetTableIndex(0);
    columnTrid.setDatasetColumnIndex(0);
    auto result = columnTrid.evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kUInt64);
    EXPECT_EQ(result.getUInt64(), std::uint64_t(1));
    columnTrid.validate(context);

    // Create ADDRESS column expression, read the value check it's type and value
    requests::SingleColumnExpression columnAddress("TestTbl", "ADDRESS");
    columnAddress.setSingleDatasetTableIndex(0);
    columnAddress.setDatasetColumnIndex(1);
    result = columnAddress.evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kString);
    EXPECT_EQ(result.getString(), std::string("121 Anselmo str."));
    columnAddress.validate(context);

    // Create COUNT column expression, read the value check it's type and value
    requests::SingleColumnExpression columnCount("TestTbl", "COUNT");
    columnCount.setSingleDatasetTableIndex(0);
    columnCount.setDatasetColumnIndex(2);
    result = columnCount.evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kInt32);
    EXPECT_EQ(result.getInt32(), std::int32_t(-25));
    columnCount.validate(context);

    // Create LEVEL column expression, read the value check it's type and value
    requests::SingleColumnExpression columnLevel("TestTbl", "LEVEL");
    columnLevel.setSingleDatasetTableIndex(0);
    columnLevel.setDatasetColumnIndex(3);
    columnCount.validate(context);
    result = columnLevel.evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kDouble);
    EXPECT_EQ(result.getDouble(), double(1230.0165432));

    requests::SingleColumnExpression columnDate("TestTbl", "Date");
    columnDate.setSingleDatasetTableIndex(0);
    columnDate.setDatasetColumnIndex(4);
    result = columnDate.evaluate(context);
    ASSERT_EQ(result.getValueType(), dbengine::VariantType::kDateTime);
    columnDate.validate(context);
}
