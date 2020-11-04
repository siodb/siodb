// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "TestContext.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"
#include "dbengine/parser/SqlParser.h"
#include "dbengine/parser/expr/AllExpressions.h"

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;

TEST(Query, SelectWithNegativeInt8)
{
    // Parse statement and prepare request
    const std::string statement("SELECT -111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kUnaryMinusOperator);

    const auto& unaryMinus = dynamic_cast<const requests::UnaryMinusOperator&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& operand = unaryMinus.getOperand();
    ASSERT_EQ(operand.getType(), requests::ExpressionType::kConstant);
    const auto constant = dynamic_cast<const requests::ConstantExpression&>(operand);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kUInt8);
    ASSERT_EQ(value.getUInt8(), 111U);
}

TEST(Query, SelectWithUInt8)
{
    // Parse statement and prepare request
    const std::string statement("SELECT 111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kConstant);

    const auto& constant = dynamic_cast<const requests::ConstantExpression&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kUInt8);
    ASSERT_EQ(value.getUInt8(), 111U);
}

TEST(Query, SelectWithNegativeInt16)
{
    // Parse statement and prepare request
    const std::string statement("SELECT -11111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kUnaryMinusOperator);

    const auto& unaryMinus = dynamic_cast<const requests::UnaryMinusOperator&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& operand = unaryMinus.getOperand();
    ASSERT_EQ(operand.getType(), requests::ExpressionType::kConstant);
    const auto constant = dynamic_cast<const requests::ConstantExpression&>(operand);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kUInt16);
    ASSERT_EQ(value.getUInt16(), 11111U);
}

TEST(Query, SelectWithUInt16)
{
    // Parse statement and prepare request
    const std::string statement("SELECT 11111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kConstant);

    const auto& constant = dynamic_cast<const requests::ConstantExpression&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kUInt16);
    ASSERT_EQ(value.getUInt16(), 11111U);
}

TEST(Query, SelectWithNegativeInt32)
{
    // Parse statement and prepare request
    const std::string statement("SELECT -111111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kUnaryMinusOperator);

    const auto& unaryMinus = dynamic_cast<const requests::UnaryMinusOperator&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& operand = unaryMinus.getOperand();
    ASSERT_EQ(operand.getType(), requests::ExpressionType::kConstant);
    const auto constant = dynamic_cast<const requests::ConstantExpression&>(operand);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kUInt32);
    ASSERT_EQ(value.getUInt32(), 111111U);
}

TEST(Query, SelectWithUInt32)
{
    // Parse statement and prepare request
    const std::string statement("SELECT 111111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kConstant);

    const auto& constant = dynamic_cast<const requests::ConstantExpression&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kUInt32);
    ASSERT_EQ(value.getUInt32(), 111111U);
}

TEST(Query, SelectWithNegativeInt64)
{
    // Parse statement and prepare request
    const std::string statement("SELECT -111111111111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kUnaryMinusOperator);

    const auto& unaryMinus = dynamic_cast<const requests::UnaryMinusOperator&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& operand = unaryMinus.getOperand();
    ASSERT_EQ(operand.getType(), requests::ExpressionType::kConstant);
    const auto constant = dynamic_cast<const requests::ConstantExpression&>(operand);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kUInt64);
    ASSERT_EQ(value.getUInt64(), 111111111111ULL);
}

TEST(Query, SelectWithUInt64)
{
    // Parse statement and prepare request
    const std::string statement("SELECT 111111111111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kConstant);

    const auto& constant = dynamic_cast<const requests::ConstantExpression&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kUInt64);
    ASSERT_EQ(value.getUInt64(), 111111111111ULL);
}

TEST(Query, SelectWithNegativeDouble)
{
    // Parse statement and prepare request
    const std::string statement("SELECT -111.111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kUnaryMinusOperator);

    const auto& unaryMinus = dynamic_cast<const requests::UnaryMinusOperator&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& operand = unaryMinus.getOperand();
    ASSERT_EQ(operand.getType(), requests::ExpressionType::kConstant);
    const auto constant = dynamic_cast<const requests::ConstantExpression&>(operand);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kDouble);
    ASSERT_EQ(value.getDouble(), 111.111);
}

TEST(Query, SelectWithDouble)
{
    // Parse statement and prepare request
    const std::string statement("SELECT 111.111 AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kConstant);

    const auto& constant = dynamic_cast<const requests::ConstantExpression&>(
            *request.m_resultExpressions[0].m_expression);
    const auto& value = constant.getValue();

    ASSERT_EQ(value.getValueType(), dbengine::VariantType::kDouble);
    ASSERT_EQ(value.getDouble(), 111.111);
}
