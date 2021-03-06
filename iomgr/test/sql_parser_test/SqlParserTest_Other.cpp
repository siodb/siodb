// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "TestContext.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/iomgr/shared/dbengine/parser/expr/AllExpressions.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;

TEST(Query, UseDatabase)
{
    // Parse statement and prepare request
    const std::string statement("USE DATABASE my_database");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kUseDatabase);

    // Check request
    const auto& request = dynamic_cast<const requests::UseDatabaseRequest&>(*dbeRequest);
    ASSERT_EQ(request.m_database, "MY_DATABASE");
}

TEST(Query, ShowDatabases)
{
    // Parse statement and prepare request
    const std::string statement("SHOW DATABASES");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kShowDatabases);
}

TEST(Query, ShowDbs)
{
    // Parse statement and prepare request
    const std::string statement("SHOW DBS");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kShowDatabases);
}

TEST(Query, ShowTables)
{
    // Parse statement and prepare request
    const std::string statement("SHOW TABLES");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kShowTables);
}

TEST(Query, DescTable)
{
    // Parse statement and prepare request
    const std::string statement("DESC TABLE SYS.SYS_TABLES");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDescribeTable);

    // Check request
    const auto& request = dynamic_cast<const requests::DescribeTableRequest&>(*dbeRequest);
    ASSERT_EQ(request.m_database, "SYS");
    ASSERT_EQ(request.m_table, "SYS_TABLES");
}

TEST(Query, DescribeTable)
{
    // Parse statement and prepare request
    const std::string statement("DESCRIBE TABLE SYS.SYS_TABLES");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDescribeTable);

    // Check request
    const auto& request = dynamic_cast<const requests::DescribeTableRequest&>(*dbeRequest);
    ASSERT_EQ(request.m_database, "SYS");
    ASSERT_EQ(request.m_table, "SYS_TABLES");
}
