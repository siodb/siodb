// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "TestContext.h"
#include "dbengine/parser/DBEngineRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;

TEST(General, SingleStatement)
{
    // Parse statement
    const std::string statement("SELECT my_column FROM my_table");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    // Check parser
    EXPECT_EQ(parser.getStatementCount(), 1U);
}

TEST(General, MultipleStatements)
{
    // Parse statement
    const std::string statement("SELECT my_column FROM my_table; SELECT column2 FROM table2;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    // Check parser
    EXPECT_EQ(parser.getStatementCount(), 2U);
    EXPECT_TRUE(parser.findStatement(0) != nullptr);
    EXPECT_TRUE(parser.findStatement(1) != nullptr);
    EXPECT_TRUE(parser.findStatement(2) == nullptr);
}

TEST(General, ParseError)
{
    // Parse statement and prepare request
    const std::string statement("NOT SELECT my_column;");
    parser_ns::SqlParser parser(statement);
    try {
        parser.parse();
        throw "Should not reach here";
    } catch (...) {
        // Check parser
        //std::cout << "Got exception: " << ex.what() << std::endl;
        const std::string s = "at (1, 0): extraneous input 'NOT'";
        EXPECT_EQ(parser.getErrorMessage().substr(0, s.length()), s);
    }
}
