// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "TestContext.h"
#include "dbengine/parser/DBEngineRequestFactoryError.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"
#include "dbengine/parser/SqlParser.h"
#include "dbengine/parser/expr/AllExpressions.h"

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;

TEST(DML, Insert1)
{
    // Parse statement and prepare request
    const std::string statement(
            "INSERT INTO my_database.my_table (col0, col1, col2, col3)"
            " VALUES (1, 'Bill', true, NULL);");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kInsert);

    // Check request
    const auto& request = dynamic_cast<const requests::InsertRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_columns.size(), 4U);
    EXPECT_EQ(request.m_columns[0], "COL0");
    EXPECT_EQ(request.m_columns[1], "COL1");
    EXPECT_EQ(request.m_columns[2], "COL2");
    EXPECT_EQ(request.m_columns[3], "COL3");

    // Check values
    ASSERT_EQ(request.m_values.size(), 1U);
    ASSERT_EQ(request.m_values[0].size(), 4U);

    dbengine::Variant v;
    TestContext context;

    v = request.m_values[0][0]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kUInt8);
    EXPECT_EQ(v.getUInt8(), 1U);

    v = request.m_values[0][1]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kString);
    EXPECT_EQ(v.getString(), "Bill");

    v = request.m_values[0][2]->evaluate(context);
    EXPECT_TRUE(v.isBool());
    EXPECT_TRUE(v.getBool());

    v = request.m_values[0][3]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kNull);
}

TEST(DML, Insert2)
{
    // Parse statement and prepare request
    const std::string statement(
            "INSERT INTO my_database.my_table"
            " VALUES (1, 'Bill', true, NULL), (2, 'Steve', false);");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kInsert);

    // Check request
    const auto& request = dynamic_cast<const requests::InsertRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_columns.size(), 0U);

    // Check values
    ASSERT_EQ(request.m_values.size(), 2U);
    ASSERT_EQ(request.m_values[0].size(), 4U);
    ASSERT_EQ(request.m_values[1].size(), 3U);

    dbengine::Variant v;
    TestContext context;

    v = request.m_values[0][0]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kUInt8);
    EXPECT_EQ(v.getUInt8(), 1U);

    v = request.m_values[0][1]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kString);
    EXPECT_EQ(v.getString(), "Bill");

    v = request.m_values[0][2]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kBool);
    EXPECT_TRUE(v.getBool());

    v = request.m_values[0][3]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kNull);

    v = request.m_values[1][0]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kUInt8);
    EXPECT_EQ(v.getUInt8(), 2U);

    v = request.m_values[1][1]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kString);
    EXPECT_EQ(v.getString(), "Steve");

    v = request.m_values[1][2]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kBool);
    EXPECT_FALSE(v.getBool());
}

TEST(DML, Insert3)
{
    // Parse statement and prepare request
    const std::string statement("INSERT INTO my_database.my_table (col1) VALUES (x'abcdef');");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kInsert);

    // Check request
    const auto& request = dynamic_cast<const requests::InsertRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_columns.size(), 1U);
    EXPECT_EQ(request.m_columns[0], "COL1");

    // Check values
    ASSERT_EQ(request.m_values.size(), 1U);
    ASSERT_EQ(request.m_values[0].size(), 1U);

    dbengine::Variant v;
    TestContext context;

    v = request.m_values[0][0]->evaluate(context);
    EXPECT_EQ(v.getValueType(), siodb::iomgr::dbengine::VariantType::kBinary);
    const siodb::BinaryValue expectedBinary {0xAB, 0xCD, 0xEF};
    EXPECT_EQ(v.getBinary(), expectedBinary);
}

// Invalid character in the hex string
TEST(DML, Insert_InvalidCharInHexString)
{
    // Parse statement and prepare request
    const std::string statement("INSERT INTO my_database.my_table (col1) VALUES (x'abcdefg');");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    ASSERT_THROW(factory.createSqlRequest(), std::runtime_error);
}

// Hex string size is odd
TEST(DML, Insert_HexStringLengthIsOdd)
{
    // Parse statement and prepare request
    const std::string statement("INSERT INTO my_database.my_table (col1) VALUES (x'abcdef1');");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    ASSERT_THROW(factory.createSqlRequest(), std::runtime_error);
}

TEST(DML, Update)
{
    // Parse statement and prepare request
    const std::string statement(
            "UPDATE my_database.my_table"
            " SET address = 'San Francisco', zip='94010' WHERE name = 'mycompany'");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kUpdate);

    // Check request
    const auto& request = dynamic_cast<const requests::UpdateRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table.m_name, "MY_TABLE");
    EXPECT_TRUE(request.m_table.m_alias.empty());

    // Check columns
    ASSERT_EQ(request.m_columns.size(), 2U);
    EXPECT_EQ(request.m_columns[0].m_column, "ADDRESS");
    EXPECT_TRUE(request.m_columns[0].m_table.empty());
    EXPECT_EQ(request.m_columns[1].m_column, "ZIP");
    EXPECT_TRUE(request.m_columns[1].m_table.empty());

    TestContext context;
    ASSERT_EQ(request.m_values.size(), 2U);
    request.m_values[0]->validate(context);
    request.m_values[1]->validate(context);
    ASSERT_EQ(request.m_values[0]->evaluate(context), "San Francisco");
    ASSERT_EQ(request.m_values[1]->evaluate(context), "94010");

    ASSERT_TRUE(request.m_where);
    ASSERT_TRUE(isBoolType(request.m_where->getResultValueType(context)));
    ASSERT_EQ(request.m_where->getType(), requests::ExpressionType::kEqualPredicate);

    const auto& equalExpr = dynamic_cast<const requests::EqualOperator&>(*request.m_where);

    ASSERT_EQ(
            equalExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(equalExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(equalExpr.getLeftOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "NAME");
}

TEST(DML, Delete)
{
    // Parse statement and prepare request
    const std::string statement("DELETE FROM my_database.my_table WHERE id = 7;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDelete);

    // Check request
    const auto& request = dynamic_cast<const requests::DeleteRequest&>(*dbeRequest);

    TestContext context;
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table.m_name, "MY_TABLE");
    ASSERT_TRUE(request.m_where);
    ASSERT_TRUE(isBoolType(request.m_where->getResultValueType(context)));

    ASSERT_EQ(request.m_where->getType(), requests::ExpressionType::kEqualPredicate);

    const auto& equalExpr = dynamic_cast<const requests::EqualOperator&>(*request.m_where);

    ASSERT_EQ(
            equalExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(equalExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(equalExpr.getLeftOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "ID");
}

TEST(DML, DeleteWithTableAliasTest)
{
    // 2 strings with and without 'AS' keyword for table alias
    for (const std::string statement :
            {"DELETE from MY_DB.MY_TABLE as MY_TABLE_ALIAS where MY_TABLE_ALIAS.ID = 132",
                    "DELETE from MY_DB.MY_TABLE MY_TABLE_ALIAS where MY_TABLE_ALIAS.ID = 132"}) {
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto dbeRequest = factory.createSqlRequest();

        // Check request type
        ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDelete);

        // Check request
        const auto& request = dynamic_cast<const requests::DeleteRequest&>(*dbeRequest);

        TestContext context;
        EXPECT_EQ(request.m_database, "MY_DB");
        EXPECT_EQ(request.m_table.m_name, "MY_TABLE");
        EXPECT_EQ(request.m_table.m_alias, "MY_TABLE_ALIAS");
        ASSERT_TRUE(request.m_where);
        ASSERT_TRUE(isBoolType(request.m_where->getResultValueType(context)));

        ASSERT_EQ(request.m_where->getType(), requests::ExpressionType::kEqualPredicate);

        const auto& equalExpr = dynamic_cast<const requests::EqualOperator&>(*request.m_where);

        ASSERT_EQ(equalExpr.getLeftOperand().getType(),
                requests::ExpressionType::kSingleColumnReference);

        ASSERT_EQ(equalExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

        const auto& columnExpr =
                dynamic_cast<const requests::SingleColumnExpression&>(equalExpr.getLeftOperand());

        EXPECT_EQ(columnExpr.getColumnName(), "ID");
        EXPECT_EQ(columnExpr.getTableName(), "MY_TABLE_ALIAS");
    }
}

TEST(DML, InsertColumnName_1)
{
    const std::string str = "insert into test.t2 values (\"汉字\")";
    parser_ns::SqlParser parser(str);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    ASSERT_THROW(factory.createSqlRequest(), parser_ns::DBEngineRequestFactoryError);
}

TEST(DML, InsertColumnName_2)
{
    const std::string str = "insert into test.t2 values (TableName.ColumnName)";
    parser_ns::SqlParser parser(str);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    ASSERT_THROW(factory.createSqlRequest(), parser_ns::DBEngineRequestFactoryError);
}
