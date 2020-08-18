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

namespace {

void checkColumnNameAndAlias(
        const requests::ResultExpression& resultColumn, const char* name, const char* alias)
{
    ASSERT_EQ(
            resultColumn.m_expression->getType(), requests::ExpressionType::kSingleColumnReference);
    const auto columnExpression =
            dynamic_cast<const requests::SingleColumnExpression*>(resultColumn.m_expression.get());
    ASSERT_NE(columnExpression, nullptr);
    EXPECT_EQ(columnExpression->getColumnName(), name);
    ASSERT_EQ(resultColumn.m_alias, alias);
}

}  // namespace

TEST(SqlParser_Query, SelectSimple)
{
    // Parse statement and prepare request
    const std::string statement(
            "SELECT column1, column2 AS column_2222 FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 2U);

    checkColumnNameAndAlias(request.m_resultExpressions[0], "COLUMN1", "");
    checkColumnNameAndAlias(request.m_resultExpressions[1], "COLUMN2", "COLUMN_2222");

    // TODO: implement: ORDER BY, GROUP BY, HAVING, LIMIT
}

TEST(SqlParser_Query, SelectWithExpression)
{
    // Parse statement and prepare request
    const std::string statement(
            "SELECT (c1 + c2) || 'test' AS column_alias FROM my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 1U);

    EXPECT_EQ(request.m_resultExpressions[0].m_expression->getType(),
            requests::ExpressionType::kConcatenateOperator);

    // (c1 + c2) || 'test'
    const auto& concatenateExpr = dynamic_cast<const requests::ConcatenationOperator&>(
            *request.m_resultExpressions[0].m_expression);

    ASSERT_EQ(concatenateExpr.getLeftOperand().getType(), requests::ExpressionType::kAddOperator);

    ASSERT_EQ(concatenateExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    // c1 + c2
    const auto& addExpr =
            dynamic_cast<const requests::AddOperator&>(concatenateExpr.getLeftOperand());

    ASSERT_EQ(addExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(
            addExpr.getRightOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    EXPECT_EQ(request.m_resultExpressions[0].m_alias, "COLUMN_ALIAS");
}

/**
 * Test checks simple where expression: column > constant
 */
TEST(SqlParser_Query, SelectWithWhereSimpleGreater)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE A > 123;");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);

    TestContext context;
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kGreaterPredicate);

    const auto& greaterExpr =
            dynamic_cast<const requests::GreaterOperator&>(*selectRequest.m_where);

    ASSERT_EQ(greaterExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(greaterExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(greaterExpr.getLeftOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "A");
}

/**
 * Test checks BETWEEN operator in where expression
 */
TEST(SqlParser_Query, SelectWithWhereBetween)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE a BETWEEN 10 AND 100");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kBetweenPredicate);

    const auto& betweenExpr =
            dynamic_cast<const requests::BetweenOperator&>(*selectRequest.m_where);

    ASSERT_EQ(betweenExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(betweenExpr.getMiddleOperand().getType(), requests::ExpressionType::kConstant);

    ASSERT_EQ(betweenExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    ASSERT_FALSE(betweenExpr.isNotBetween());

    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(betweenExpr.getLeftOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "A");  // always UPCASE
}

/**
 * Test checks BETWEEN operator AND in where expression
 */
TEST(SqlParser_Query, SelectWithWhereBetweenWithAnd)
{
    // Parse statement
    const std::string statement(
            "SELECT date, name FROM table_name WHERE date BETWEEN '2015-01-01' and '2019-01-01' "
            "and name = 'SQL'");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);

    // date BETWEEN '2015-01-01' and '2019-01-01' and name = 'SQL'"
    const auto& andExpr = dynamic_cast<const requests::LogicalAndOperator&>(*selectRequest.m_where);

    ASSERT_EQ(andExpr.getLeftOperand().getType(), requests::ExpressionType::kBetweenPredicate);

    ASSERT_EQ(andExpr.getRightOperand().getType(), requests::ExpressionType::kEqualPredicate);

    // date BETWEEN '2015-01-01' and '2019-01-01'
    const auto& betweenExpr =
            dynamic_cast<const requests::BetweenOperator&>(andExpr.getLeftOperand());

    ASSERT_EQ(betweenExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(betweenExpr.getMiddleOperand().getType(), requests::ExpressionType::kConstant);

    ASSERT_EQ(betweenExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    ASSERT_FALSE(betweenExpr.isNotBetween());

    const auto& betweenColumnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(betweenExpr.getLeftOperand());

    EXPECT_TRUE(betweenColumnExpr.getTableName().empty());
    EXPECT_EQ(betweenColumnExpr.getColumnName(), "DATE");  // always UPCASE

    // name = 'SQL'
    const auto& equalExpr = dynamic_cast<const requests::EqualOperator&>(andExpr.getRightOperand());

    ASSERT_EQ(
            equalExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    // 'SQL'
    ASSERT_EQ(equalExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    // name
    const auto& rightColumnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(equalExpr.getLeftOperand());

    ASSERT_TRUE(rightColumnExpr.getTableName().empty());
    ASSERT_EQ(rightColumnExpr.getColumnName(), "NAME");
}

/**
 * Test checks where statement: condition AND condition
 * <=, >=, AND operators are tested
 */
TEST(SqlParser_Query, SelectWithWhereAndStatement)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE a <= 13 AND a >= 4");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kLogicalAndOperator);

    const auto& andExpr = dynamic_cast<const requests::LogicalAndOperator&>(*selectRequest.m_where);

    ASSERT_EQ(andExpr.getLeftOperand().getType(), requests::ExpressionType::kLessOrEqualPredicate);

    ASSERT_EQ(andExpr.getRightOperand().getType(),
            requests::ExpressionType::kGreaterOrEqualPredicate);

    const auto& lessOrEqual =
            dynamic_cast<const requests::LessOrEqualOperator&>(andExpr.getLeftOperand());

    ASSERT_EQ(lessOrEqual.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(lessOrEqual.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& leftColumnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(lessOrEqual.getLeftOperand());

    EXPECT_TRUE(leftColumnExpr.getTableName().empty());
    EXPECT_EQ(leftColumnExpr.getColumnName(), "A");

    const auto& greaterOrEqual =
            dynamic_cast<const requests::GreaterOrEqualOperator&>(andExpr.getRightOperand());

    ASSERT_EQ(greaterOrEqual.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(greaterOrEqual.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& rightColumnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(greaterOrEqual.getLeftOperand());

    EXPECT_TRUE(rightColumnExpr.getTableName().empty());
    EXPECT_EQ(rightColumnExpr.getColumnName(), "A");
}

/**
 * Test checks where statement: condition OR condition
 * <=, >=, OR operators are tested
 */
TEST(SqlParser_Query, SelectWithWhereOrCodition)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE a <= 13 OR a >= 4");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kLogicalOrOperator);

    const auto& orExpr = dynamic_cast<const requests::LogicalOrOperator&>(*selectRequest.m_where);

    ASSERT_EQ(orExpr.getLeftOperand().getType(), requests::ExpressionType::kLessOrEqualPredicate);

    ASSERT_EQ(
            orExpr.getRightOperand().getType(), requests::ExpressionType::kGreaterOrEqualPredicate);

    const auto& lessOrEqual =
            dynamic_cast<const requests::LessOrEqualOperator&>(orExpr.getLeftOperand());

    ASSERT_EQ(lessOrEqual.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(lessOrEqual.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& leftColumnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(lessOrEqual.getLeftOperand());

    EXPECT_TRUE(leftColumnExpr.getTableName().empty());
    EXPECT_EQ(leftColumnExpr.getColumnName(), "A");

    const auto& greaterOrEqual =
            dynamic_cast<const requests::GreaterOrEqualOperator&>(orExpr.getRightOperand());

    ASSERT_EQ(greaterOrEqual.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(greaterOrEqual.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& rightColumnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(greaterOrEqual.getLeftOperand());

    EXPECT_TRUE(rightColumnExpr.getTableName().empty());
    EXPECT_EQ(rightColumnExpr.getColumnName(), "A");
}

/**
 * Test checks where statement: Complex expression > complex expression
 * +, /, - operators are tested
 */
TEST(SqlParser_Query, SelectWithWhereArithmeticSubExpression)
{
    // Parse statement
    const std::string statement("SELECT a, b FROM table_name WHERE (a+13/a) > (b+12-a)");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kGreaterPredicate);

    const auto& greaterExpr =
            dynamic_cast<const requests::GreaterOperator&>(*selectRequest.m_where);

    ASSERT_EQ(greaterExpr.getLeftOperand().getType(), requests::ExpressionType::kAddOperator);

    ASSERT_EQ(greaterExpr.getRightOperand().getType(), requests::ExpressionType::kSubtractOperator);

    // (a+13/a)
    const auto& leftExpr = dynamic_cast<const requests::AddOperator&>(greaterExpr.getLeftOperand());

    ASSERT_EQ(
            leftExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(leftExpr.getRightOperand().getType(), requests::ExpressionType::kDivideOperator);
    // 13/a
    const auto& leftDivisionExpr =
            dynamic_cast<const requests::DivideOperator&>(leftExpr.getRightOperand());

    ASSERT_EQ(leftDivisionExpr.getLeftOperand().getType(), requests::ExpressionType::kConstant);

    ASSERT_EQ(leftDivisionExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // (b+12-a)
    const auto& rightExpr =
            dynamic_cast<const requests::SubtractOperator&>(greaterExpr.getRightOperand());

    ASSERT_EQ(rightExpr.getLeftOperand().getType(), requests::ExpressionType::kAddOperator);

    ASSERT_EQ(rightExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // b+12
    const auto& rightAddExpr =
            dynamic_cast<const requests::AddOperator&>(rightExpr.getLeftOperand());

    ASSERT_EQ(rightAddExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(rightAddExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);
}

/**
 * Test checks complex where statement:
 * %, +, /, -, * & operators are tested
 */
TEST(SqlParser_Query, SelectWithWhereComplexExpression)
{
    // Parse statement
    const std::string statement("SELECT  * FROM table_name WHERE a <= (a%b+13/(c - ((d*5)&6)))");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kLessOrEqualPredicate);

    const auto& lessOrEqualExpr =
            dynamic_cast<const requests::LessOrEqualOperator&>(*selectRequest.m_where);

    // a
    ASSERT_EQ(lessOrEqualExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // (a%b+13/(c - ((d*5)&6)))
    ASSERT_EQ(lessOrEqualExpr.getRightOperand().getType(), requests::ExpressionType::kAddOperator);

    const auto& addExpr =
            dynamic_cast<const requests::AddOperator&>(lessOrEqualExpr.getRightOperand());
    // a%b
    ASSERT_EQ(addExpr.getLeftOperand().getType(), requests::ExpressionType::kModuloOperator);

    // 13/(c - ((d*5)&6))
    ASSERT_EQ(addExpr.getRightOperand().getType(), requests::ExpressionType::kDivideOperator);

    const auto& moduloExpr =
            dynamic_cast<const requests::ModuloOperator&>(addExpr.getLeftOperand());

    // a
    ASSERT_EQ(moduloExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // b
    ASSERT_EQ(moduloExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    const auto& divisionExpr =
            dynamic_cast<const requests::DivideOperator&>(addExpr.getRightOperand());

    // 13
    ASSERT_EQ(divisionExpr.getLeftOperand().getType(), requests::ExpressionType::kConstant);

    // c - ((d*5)&6)
    ASSERT_EQ(
            divisionExpr.getRightOperand().getType(), requests::ExpressionType::kSubtractOperator);

    const auto& subtractExpr =
            dynamic_cast<const requests::SubtractOperator&>(divisionExpr.getRightOperand());

    // c
    ASSERT_EQ(subtractExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // ((d*5)&6
    ASSERT_EQ(subtractExpr.getRightOperand().getType(),
            requests::ExpressionType::kBitwiseAndOperator);

    const auto& bitwiseAnd =
            dynamic_cast<const requests::BitwiseAndOperator&>(subtractExpr.getRightOperand());

    // (d*5)
    ASSERT_EQ(bitwiseAnd.getLeftOperand().getType(), requests::ExpressionType::kMultiplyOperator);

    // 6
    ASSERT_EQ(bitwiseAnd.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& multiplyExpr =
            dynamic_cast<const requests::MultiplyOperator&>(bitwiseAnd.getLeftOperand());

    // d
    ASSERT_EQ(multiplyExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // 5
    ASSERT_EQ(multiplyExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);
}

/**
 * Test checks LIKE operator in where statement:
 */
TEST(SqlParser_Query, SelectWithWhereLike)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE a LIKE 'a__%'");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kLikePredicate);

    const auto& likeExpr = dynamic_cast<const requests::LikeOperator&>(*selectRequest.m_where);

    ASSERT_FALSE(likeExpr.isNotLike());

    ASSERT_EQ(
            likeExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(likeExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(likeExpr.getLeftOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "A");
}

/**
 * Test checks NOT LIKE operator in where statement:
 */
TEST(SqlParser_Query, SelectWithWhereNotLike)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE a NOT LIKE 'a__%'");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kLikePredicate);

    const auto& likeExpr = dynamic_cast<const requests::LikeOperator&>(*selectRequest.m_where);

    ASSERT_TRUE(likeExpr.isNotLike());

    ASSERT_EQ(
            likeExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(likeExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(likeExpr.getLeftOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "A");
}

/**
 * Test checks NOT BETWEEN operator in where statement:
 */
TEST(SqlParser_Query, SelectWithWhereNotBetween)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE a NOT BETWEEN 10 AND 100");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kBetweenPredicate);

    const auto& betweenExpr =
            dynamic_cast<const requests::BetweenOperator&>(*selectRequest.m_where);

    ASSERT_EQ(betweenExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(betweenExpr.getMiddleOperand().getType(), requests::ExpressionType::kConstant);

    ASSERT_EQ(betweenExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    ASSERT_TRUE(betweenExpr.isNotBetween());

    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(betweenExpr.getLeftOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "A");  // always UPCASE
}

/**
 * Test checks where statement with unary minus operator.
 * unary -, +, > operators are checked
 */
TEST(SqlParser_Query, SelectWithWhereUnaryMinus)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE a > -(a+10)");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kGreaterPredicate);

    // a > -(a+10)
    const auto& greaterExpr =
            dynamic_cast<const requests::GreaterOperator&>(*selectRequest.m_where);

    ASSERT_EQ(greaterExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(
            greaterExpr.getRightOperand().getType(), requests::ExpressionType::kUnaryMinusOperator);

    // a
    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(greaterExpr.getLeftOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "A");

    // -(a+10)
    const auto& unaryMinusExpr =
            dynamic_cast<const requests::UnaryMinusOperator&>(greaterExpr.getRightOperand());

    ASSERT_EQ(unaryMinusExpr.getOperand().getType(), requests::ExpressionType::kAddOperator);

    // a + 10
    const auto& addOperator =
            dynamic_cast<const requests::AddOperator&>(unaryMinusExpr.getOperand());

    ASSERT_EQ(addOperator.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(addOperator.getRightOperand().getType(), requests::ExpressionType::kConstant);

    // a
    const auto& addColumnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(addOperator.getLeftOperand());

    EXPECT_TRUE(addColumnExpr.getTableName().empty());
    EXPECT_EQ(addColumnExpr.getColumnName(), "A");
}

/**
 * Test checks where statement with unary plus operator.
 * unary +, < operators are checked
 */
TEST(SqlParser_Query, SelectWithWhereUnaryPlus)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE +a < 1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kLessPredicate);

    // +a < 1
    const auto& lessExpr = dynamic_cast<const requests::LessOperator&>(*selectRequest.m_where);

    ASSERT_EQ(lessExpr.getLeftOperand().getType(), requests::ExpressionType::kUnaryPlusOperator);

    ASSERT_EQ(lessExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    // +a
    const auto& unaryPlusExpr =
            dynamic_cast<const requests::UnaryPlusOperator&>(lessExpr.getLeftOperand());

    ASSERT_EQ(
            unaryPlusExpr.getOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    // a
    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(unaryPlusExpr.getOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "A");
}

/**
 * Test checks where statement with complement(~) operator.
 * ~, = operators are checked
 */
TEST(SqlParser_Query, SelectWithWhereComplement)
{
    // Parse statement
    const std::string statement("SELECT * FROM table_name WHERE a = ~b");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kEqualPredicate);

    // a = ~b
    const auto& equalExp = dynamic_cast<const requests::EqualOperator&>(*selectRequest.m_where);

    ASSERT_EQ(
            equalExp.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(equalExp.getRightOperand().getType(),
            requests::ExpressionType::kBitwiseComplementOperator);

    // a
    const auto& columnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(equalExp.getLeftOperand());

    EXPECT_TRUE(columnExpr.getTableName().empty());
    EXPECT_EQ(columnExpr.getColumnName(), "A");

    // ~b
    const auto& complementExpr =
            dynamic_cast<const requests::ComplementOperator&>(equalExp.getRightOperand());

    ASSERT_EQ(complementExpr.getOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);
}

/**
 * Test checks where statement with column expression with a table
 * >, NOT operators are checked
 */
TEST(SqlParser_Query, SelectWithWhereTableColumn)
{
    // Parse statement
    for (const std::string statement : {"SELECT a FROM table_name WHERE NOT (123 > t.a)",
                 "SELECT a FROM table_name WHERE NOT 123 > t.a"}) {
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto dbeRequest =
                parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

        ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
        const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
        ASSERT_TRUE(selectRequest.m_where != nullptr);

        ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kLogicalNotOperator);

        const auto& notExpr =
                dynamic_cast<const requests::LogicalNotOperator&>(*selectRequest.m_where);

        ASSERT_EQ(notExpr.getOperand().getType(), requests::ExpressionType::kGreaterPredicate);

        const auto& greaterExpr =
                dynamic_cast<const requests::GreaterOperator&>(notExpr.getOperand());

        ASSERT_EQ(greaterExpr.getLeftOperand().getType(), requests::ExpressionType::kConstant);

        ASSERT_EQ(greaterExpr.getRightOperand().getType(),
                requests::ExpressionType::kSingleColumnReference);

        const auto& columnExpr = dynamic_cast<const requests::SingleColumnExpression&>(
                greaterExpr.getRightOperand());

        EXPECT_EQ(columnExpr.getTableName(), "T");
        EXPECT_EQ(columnExpr.getColumnName(), "A");
    }
}

/**
 * Test checks where statement with right shift operator.
 * >>, = operators are checked
 */
TEST(SqlParser_Query, SelectWithWhereRightShift)
{
    // Parse statement
    const std::string statement("SELECT a FROM table_name WHERE a = 4 >> b");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kEqualPredicate);

    // a = 4 >> b
    const auto& equalExpr = dynamic_cast<const requests::EqualOperator&>(*selectRequest.m_where);

    ASSERT_EQ(
            equalExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(equalExpr.getRightOperand().getType(), requests::ExpressionType::kRightShiftOperator);
}

/**
 * Test checks where statement with left shift operator.
 * <<, = operators are checked
 */
TEST(SqlParser_Query, SelectWithWhereLeftShift)
{
    // Parse statement
    const std::string statement("SELECT * FROM table_name WHERE a = 4 << b");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kEqualPredicate);

    // a = 4 << b
    const auto& equalExpr = dynamic_cast<const requests::EqualOperator&>(*selectRequest.m_where);

    ASSERT_EQ(
            equalExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(equalExpr.getRightOperand().getType(), requests::ExpressionType::kLeftShiftOperator);
}

/**
 * Test checks where statement with bitwise or operator.
 * |, = operators are checked
 */
TEST(SqlParser_Query, SelectWithWhereBitwiseOrOperator)
{
    // Parse statement
    const std::string statement("SELECT * FROM table_name WHERE a = 4 | b");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kEqualPredicate);

    // a = 4 | b
    const auto& equalExpr = dynamic_cast<const requests::EqualOperator&>(*selectRequest.m_where);

    ASSERT_EQ(
            equalExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(equalExpr.getRightOperand().getType(), requests::ExpressionType::kBitwiseOrOperator);
}

/**
 * Test checks where statement with IN operator.
 * IN operator is checked
 */
TEST(SqlParser_Query, SelectWithWhereIn)
{
    // Parse statement
    for (const std::string statement :
            {"SELECT a FROM t1 WHERE a in ('A', 'C', b, ('A' + c), 'A' + b + 'C')",
                    "SELECT a FROM t1 WHERE a not in ('A', 'C', b, ('A' + c), 'A' + b + 'C')"}) {
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto dbeRequest =
                parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

        ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
        const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
        ASSERT_TRUE(selectRequest.m_where != nullptr);
        ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kInPredicate);

        // a in ('ABC', 'XYZ', b)
        const auto& inOperator = dynamic_cast<const requests::InOperator&>(*selectRequest.m_where);

        ASSERT_EQ(
                inOperator.getValue().getType(), requests::ExpressionType::kSingleColumnReference);

        ASSERT_EQ(inOperator.getVariants().size(), 5U);

        ASSERT_EQ(inOperator.getVariants()[0]->getType(), requests::ExpressionType::kConstant);
        ASSERT_EQ(inOperator.getVariants()[1]->getType(), requests::ExpressionType::kConstant);
        ASSERT_EQ(inOperator.getVariants()[2]->getType(),
                requests::ExpressionType::kSingleColumnReference);
        ASSERT_EQ(inOperator.getVariants()[3]->getType(), requests::ExpressionType::kAddOperator);
        ASSERT_EQ(inOperator.getVariants()[4]->getType(), requests::ExpressionType::kAddOperator);
    }
}

/*
 * Test checks operator precedence in where statement without parentheses
 */
TEST(SqlParser_Query, SelectWithWhereOperatorPrecedence)
{
    // Parse statement
    const std::string statement =
            "SELECT * FROM T WHERE NOT a * +b + c / d - -e > a || b % c << d >> e  & f | ~g ^ "
            "h "
            "AND a LIKE b OR a NOT BETWEEN b AND c";
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);

    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kLogicalOrOperator);

    // NOT a * +b + c / d - -e > aa || b % c << d >> e  & f | ~g ^ h && a LIKE b OR a NOT BETWEEN b AND c;
    const auto& logicalOrExpr =
            dynamic_cast<const requests::LogicalOrOperator&>(*selectRequest.m_where);

    ASSERT_EQ(logicalOrExpr.getLeftOperand().getType(),
            requests::ExpressionType::kLogicalAndOperator);

    ASSERT_EQ(
            logicalOrExpr.getRightOperand().getType(), requests::ExpressionType::kBetweenPredicate);

    // NOT a * +b + c / d - -e > a || b % c << d >> e  & f | ~g ^ h && a LIKE b
    const auto& logicalAndExpr =
            dynamic_cast<const requests::LogicalAndOperator&>(logicalOrExpr.getLeftOperand());

    ASSERT_EQ(logicalAndExpr.getLeftOperand().getType(),
            requests::ExpressionType::kLogicalNotOperator);

    ASSERT_EQ(logicalAndExpr.getRightOperand().getType(), requests::ExpressionType::kLikePredicate);

    // a LIKE b
    const auto& likeExpr =
            dynamic_cast<const requests::LikeOperator&>(logicalAndExpr.getRightOperand());

    ASSERT_EQ(
            likeExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(
            likeExpr.getRightOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_FALSE(likeExpr.isNotLike());

    // a NOT BETWEEN b AND c
    const auto& betweenExpr =
            dynamic_cast<const requests::BetweenOperator&>(logicalOrExpr.getRightOperand());

    ASSERT_TRUE(betweenExpr.isNotBetween());

    ASSERT_EQ(betweenExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(betweenExpr.getMiddleOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(betweenExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // NOT a * +b + c / d - -e > a || b % c << d >> e  & f | ~g ^ h
    const auto& notExpr =
            dynamic_cast<const requests::LogicalNotOperator&>(logicalAndExpr.getLeftOperand());

    ASSERT_EQ(notExpr.getOperand().getType(), requests::ExpressionType::kGreaterPredicate);

    // a * +b + c / d - -e > a || b % c << d >> e  & f | ~g ^ h
    const auto& greaterExpr = dynamic_cast<const requests::GreaterOperator&>(notExpr.getOperand());

    ASSERT_EQ(greaterExpr.getLeftOperand().getType(), requests::ExpressionType::kSubtractOperator);

    ASSERT_EQ(
            greaterExpr.getRightOperand().getType(), requests::ExpressionType::kBitwiseXorOperator);

    // a || b % c << d >> e  & f | ~g ^ h
    const auto& subtractExpr =
            dynamic_cast<const requests::SubtractOperator&>(greaterExpr.getLeftOperand());

    ASSERT_EQ(subtractExpr.getLeftOperand().getType(), requests::ExpressionType::kAddOperator);

    ASSERT_EQ(subtractExpr.getRightOperand().getType(),
            requests::ExpressionType::kUnaryMinusOperator);

    // a * +b + c / d
    const auto& addExpr = dynamic_cast<const requests::AddOperator&>(subtractExpr.getLeftOperand());

    ASSERT_EQ(addExpr.getLeftOperand().getType(), requests::ExpressionType::kMultiplyOperator);

    ASSERT_EQ(addExpr.getRightOperand().getType(), requests::ExpressionType::kDivideOperator);

    // a * +b
    const auto& multiplyExpr =
            dynamic_cast<const requests::MultiplyOperator&>(addExpr.getLeftOperand());

    ASSERT_EQ(multiplyExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(
            multiplyExpr.getRightOperand().getType(), requests::ExpressionType::kUnaryPlusOperator);

    // +b
    const auto& unaryPlusExpr =
            dynamic_cast<const requests::UnaryPlusOperator&>(multiplyExpr.getRightOperand());

    ASSERT_EQ(
            unaryPlusExpr.getOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    // c / d
    const auto& divideExpr =
            dynamic_cast<const requests::DivideOperator&>(addExpr.getRightOperand());

    ASSERT_EQ(divideExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(divideExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // -e
    const auto& unaryMinusExpr =
            dynamic_cast<const requests::UnaryMinusOperator&>(subtractExpr.getRightOperand());

    ASSERT_EQ(unaryMinusExpr.getOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // a || b % c << d >> e  & f | ~g ^ h
    const auto& xorExpr =
            dynamic_cast<const requests::BitwiseXorOperator&>(greaterExpr.getRightOperand());

    ASSERT_EQ(xorExpr.getLeftOperand().getType(), requests::ExpressionType::kBitwiseOrOperator);

    ASSERT_EQ(
            xorExpr.getRightOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    // a || b % c << d >> e  & f | ~g
    const auto& bitwiseOrExpr =
            dynamic_cast<const requests::BitwiseOrOperator&>(xorExpr.getLeftOperand());

    ASSERT_EQ(bitwiseOrExpr.getLeftOperand().getType(),
            requests::ExpressionType::kBitwiseAndOperator);

    ASSERT_EQ(bitwiseOrExpr.getRightOperand().getType(),
            requests::ExpressionType::kBitwiseComplementOperator);

    // ~f
    const auto& complementExpr =
            dynamic_cast<const requests::ComplementOperator&>(bitwiseOrExpr.getRightOperand());

    ASSERT_EQ(complementExpr.getOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // a || b % c << d >> e  & f
    const auto& bitwiseAndExpr =
            dynamic_cast<const requests::BitwiseAndOperator&>(bitwiseOrExpr.getLeftOperand());

    ASSERT_EQ(bitwiseAndExpr.getLeftOperand().getType(),
            requests::ExpressionType::kRightShiftOperator);

    ASSERT_EQ(bitwiseAndExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // a || b % c << d >> e
    const auto& rightShiftExpr =
            dynamic_cast<const requests::RightShiftOperator&>(bitwiseAndExpr.getLeftOperand());

    ASSERT_EQ(rightShiftExpr.getLeftOperand().getType(),
            requests::ExpressionType::kLeftShiftOperator);

    ASSERT_EQ(rightShiftExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // a || b % c << d
    const auto& leftShiftExpr =
            dynamic_cast<const requests::LeftShiftOperator&>(rightShiftExpr.getLeftOperand());

    ASSERT_EQ(leftShiftExpr.getLeftOperand().getType(), requests::ExpressionType::kModuloOperator);

    ASSERT_EQ(leftShiftExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // a || b % c
    const auto& moduloExpr =
            dynamic_cast<const requests::ModuloOperator&>(leftShiftExpr.getLeftOperand());

    ASSERT_EQ(
            moduloExpr.getLeftOperand().getType(), requests::ExpressionType::kConcatenateOperator);

    ASSERT_EQ(moduloExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    // a || b
    const auto& concatenateExpr =
            dynamic_cast<const requests::ConcatenationOperator&>(moduloExpr.getLeftOperand());

    ASSERT_EQ(concatenateExpr.getLeftOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(concatenateExpr.getRightOperand().getType(),
            requests::ExpressionType::kSingleColumnReference);
}

/** This test checks usage of SQL keyword in the WHERE clause. */
TEST(SqlParser_Query, SelectWithKeyword)
{
    // Parse statement. ASC and WITH are SQL keywords, keywords are allowed to be used in statements
    // as column names
    const std::string statement("SELECT * FROM table_name WHERE WITH.ASC = B");

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kEqualPredicate);

    // WITH.ASC = B
    const auto& equalExp = dynamic_cast<const requests::EqualOperator&>(*selectRequest.m_where);

    ASSERT_EQ(
            equalExp.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(
            equalExp.getRightOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    // WITH.ASC
    const auto& leftColumnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(equalExp.getLeftOperand());

    EXPECT_EQ(leftColumnExpr.getTableName(), "WITH");
    EXPECT_EQ(leftColumnExpr.getColumnName(), "ASC");

    // B
    const auto& rightColumnExpr =
            dynamic_cast<const requests::SingleColumnExpression&>(equalExp.getRightOperand());

    EXPECT_TRUE(rightColumnExpr.getTableName().empty());
    EXPECT_EQ(rightColumnExpr.getColumnName(), "B");
}

/** This test checks usage of attribute as column name. */
TEST(SqlParser_Query, SelectWithAttribute)
{
    // Here "real_name" and "description" are attributes.
    const std::string statement(
            "SELECT trid, real_name as name, description FROM my_database.my_table;");

    // Parse statement and prepare request
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    // Check request
    const auto& request = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_tables[0].m_name, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_resultExpressions.size(), 3U);

    checkColumnNameAndAlias(request.m_resultExpressions[0], "TRID", "");
    checkColumnNameAndAlias(request.m_resultExpressions[1], "REAL_NAME", "NAME");
    checkColumnNameAndAlias(request.m_resultExpressions[2], "DESCRIPTION", "");
}

/**
 * Test checks where statement with IS operator.
 * IS operator is checked
 */
TEST(SqlParser_Query, SelectWithWhereIsNull)
{
    // Parse statement
    const std::string statement = "SELECT c1 FROM t1 WHERE c1 IS NULL";

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kIsPredicate);

    // c1 IS NULL
    const auto& isExpr = dynamic_cast<const requests::IsOperator&>(*selectRequest.m_where);

    EXPECT_FALSE(isExpr.isNot());

    ASSERT_EQ(isExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(isExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);
}

/**
 * Test checks where statement with IS operator.
 * IS operator is checked
 */
TEST(SqlParser_Query, SelectWithWhereIsNotNull)
{
    // Parse statement
    const std::string statement = "SELECT c1 FROM t1 WHERE c1 IS NOT NULL";

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kIsPredicate);

    // c1 IS NOT NULL
    const auto& isExpr = dynamic_cast<const requests::IsOperator&>(*selectRequest.m_where);

    EXPECT_TRUE(isExpr.isNot());

    ASSERT_EQ(isExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    ASSERT_EQ(isExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);
}

/**
 * Test checks where statement with IS operator.
 * IS operator is checked
 */
TEST(SqlParser_Query, SelectWithWhereIsExpresssion)
{
    // Parse statement
    const std::string statement = "SELECT c1 FROM t1 WHERE c1 IS c2";

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_where != nullptr);
    ASSERT_EQ(selectRequest.m_where->getType(), requests::ExpressionType::kIsPredicate);

    // c1 IS c2
    const auto& isExpr = dynamic_cast<const requests::IsOperator&>(*selectRequest.m_where);

    EXPECT_FALSE(isExpr.isNot());

    EXPECT_EQ(isExpr.getLeftOperand().getType(), requests::ExpressionType::kSingleColumnReference);

    EXPECT_EQ(isExpr.getRightOperand().getType(), requests::ExpressionType::kSingleColumnReference);
}

/**
 * Test checks where statement with LIMIT clause.
 */
TEST(SqlParser_Query, SelectWithLimit)
{
    // Parse statement
    const std::string statement = "SELECT c1 FROM t1 LIMIT 10";

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_limit != nullptr);
    ASSERT_EQ(selectRequest.m_limit->getType(), requests::ExpressionType::kConstant);

    const auto& expr = dynamic_cast<const requests::ConstantExpression&>(*selectRequest.m_limit);
    ASSERT_TRUE(expr.getValue().compatibleEqual(10));
}

/**
 * Test checks where statement with LIMIT with compound expression clause.
 */
TEST(SqlParser_Query, SelectWithLimitCompoundExpression)
{
    // Parse statement
    const std::string statement = "SELECT c1 FROM t1 LIMIT 10 + 2";

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_limit != nullptr);
    ASSERT_EQ(selectRequest.m_limit->getType(), requests::ExpressionType::kAddOperator);

    const auto& expr = dynamic_cast<const requests::AddOperator&>(*selectRequest.m_limit);

    EXPECT_EQ(expr.getLeftOperand().getType(), requests::ExpressionType::kConstant);
    EXPECT_EQ(expr.getRightOperand().getType(), requests::ExpressionType::kConstant);
}

/**
 * Test checks where statement with LIMIT + OFFSET clause.
 */
TEST(SqlParser_Query, SelectWithLimitAndOffset)
{
    // Parse statement
    const std::string statement = "SELECT c1 FROM t1 LIMIT 10 OFFSET 10 + 2";

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_offset != nullptr);
    ASSERT_EQ(selectRequest.m_offset->getType(), requests::ExpressionType::kAddOperator);

    const auto& offsetExpr = dynamic_cast<const requests::AddOperator&>(*selectRequest.m_offset);

    EXPECT_EQ(offsetExpr.getLeftOperand().getType(), requests::ExpressionType::kConstant);
    EXPECT_EQ(offsetExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    ASSERT_TRUE(selectRequest.m_limit != nullptr);
    ASSERT_EQ(selectRequest.m_limit->getType(), requests::ExpressionType::kConstant);

    const auto& limitExpr =
            dynamic_cast<const requests::ConstantExpression&>(*selectRequest.m_limit);
    ASSERT_TRUE(limitExpr.getValue().compatibleEqual(10));
}

/**
 * Test checks where statement with LIMIT + OFFSET clause.
 * With using '... LIMIT <OFFSET>, <LIMIT> ...' contruction
 */
TEST(SqlParser_Query, SelectWithLimitAndOffset_2)
{
    // Parse statement
    const std::string statement = "SELECT c1 FROM t1 LIMIT 10 + 2, 10";

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineSqlRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);
    const auto& selectRequest = dynamic_cast<const requests::SelectRequest&>(*dbeRequest);
    ASSERT_TRUE(selectRequest.m_offset != nullptr);
    ASSERT_EQ(selectRequest.m_offset->getType(), requests::ExpressionType::kAddOperator);

    const auto& offsetExpr = dynamic_cast<const requests::AddOperator&>(*selectRequest.m_offset);

    EXPECT_EQ(offsetExpr.getLeftOperand().getType(), requests::ExpressionType::kConstant);
    EXPECT_EQ(offsetExpr.getRightOperand().getType(), requests::ExpressionType::kConstant);

    ASSERT_TRUE(selectRequest.m_limit != nullptr);
    ASSERT_EQ(selectRequest.m_limit->getType(), requests::ExpressionType::kConstant);

    const auto& limitExpr =
            dynamic_cast<const requests::ConstantExpression&>(*selectRequest.m_limit);
    ASSERT_TRUE(limitExpr.getValue().compatibleEqual(10));
}
