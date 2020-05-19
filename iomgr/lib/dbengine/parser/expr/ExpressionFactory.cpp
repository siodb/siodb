// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ExpressionFactory.h"

// Project headers
#include "AllExpressions.h"
#include "../AntlrHelpers.h"
#include "../antlr_wrappers/SiodbParserWrapper.h"

// Common project headers
#include <siodb/common/utils/StringBuilder.h>

// Boost headers
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/uuid/string_generator.hpp>

namespace siodb::iomgr::dbengine::parser {

ExpressionFactory::ExpressionFactory(bool allowColumnExpressions) noexcept
    : m_allowColumnExpressions(allowColumnExpressions)
{
}

requests::ExpressionPtr ExpressionFactory::createExpression(antlr4::tree::ParseTree* node) const
{
    auto rule = helpers::getNonTerminalType(node);

    // Expect literal or column name
    switch (rule) {
        case SiodbParser::RuleExpr: {
            const auto childCount = node->children.size();
            if (childCount == 1) {
                // Only simple expression could be in this case
                return createSimpleExpression(node->children[0]);
            } else if (childCount == 2) {
                const auto leftNode = node->children[0];
                const auto rightNode = node->children[1];
                const bool rightNodeValid =
                        helpers::getNonTerminalType(rightNode) == SiodbParser::RuleColumn_name
                        || helpers::getNonTerminalType(rightNode) == SiodbParser::RuleExpr;
                if (helpers::getTerminalType(leftNode) == SiodbParser::K_NOT && rightNodeValid) {
                    return std::make_unique<requests::LogicalNotOperator>(
                            createExpression(rightNode));
                }
            } else if (childCount == 3) {
                const auto leftNode = node->children[0];
                const auto midNode = node->children[1];
                const auto rightNode = node->children[2];
                // Check case: Expr Operator Expr
                if (helpers::getNonTerminalType(leftNode) == SiodbParser::RuleExpr
                        && isLogicalBinaryOperator(helpers::getTerminalType(midNode))
                        && helpers::getNonTerminalType(rightNode) == SiodbParser::RuleExpr) {
                    return createLogicalBinaryOperator(leftNode, midNode, rightNode);
                }
                // Check case ( expr )
                else if (helpers::getTerminalType(leftNode) == SiodbParser::OPEN_PAR
                         && helpers::getNonTerminalType(midNode) == SiodbParser::RuleExpr
                         && helpers::getTerminalType(rightNode) == SiodbParser::CLOSE_PAR) {
                    return createExpression(midNode);
                }
            }
            throw std::runtime_error("Expression is invalid");
        }
        case SiodbParser::RuleFunction_call: {
            // TODO: Support functions
            throw std::runtime_error("Functions aren't not supported now");
        }
        case SiodbParser::RuleSimple_expr: return createSimpleExpression(node);
        default: break;
    }

    throw std::invalid_argument("Node is not valid expression or not supported");
}

requests::ExpressionPtr ExpressionFactory::createNumericConstant(const antlr4::Token* token) const
{
    auto value = token->getText();
    try {
        std::size_t end = 0;
        const auto n = std::stoull(value, &end);
        if (value.size() == end) {
            Variant value;
            if (n > std::numeric_limits<std::uint32_t>::max())
                value = static_cast<std::uint64_t>(n);
            else if (n > std::numeric_limits<std::uint16_t>::max())
                value = static_cast<std::uint32_t>(n);
            else if (n > std::numeric_limits<std::uint8_t>::max())
                value = static_cast<std::uint16_t>(n);
            else
                value = static_cast<std::uint8_t>(n);
            return std::make_unique<requests::ConstantExpression>(std::move(value));
        }
    } catch (...) {
        // Ignore errors, try more variants
    }

    // Try signed integer
    try {
        std::size_t end = 0;
        const auto n = std::stoll(value, &end);
        if (value.size() == end) {
            Variant value;
            if (n < std::numeric_limits<std::int32_t>::min()
                    || n > std::numeric_limits<std::int32_t>::max())
                value = static_cast<std::int64_t>(n);
            else if (n < std::numeric_limits<std::int16_t>::min()
                     || n > std::numeric_limits<std::int16_t>::max())
                value = static_cast<std::int32_t>(n);
            else if (n < std::numeric_limits<std::int8_t>::min()
                     || n > std::numeric_limits<std::int8_t>::max())
                value = static_cast<std::int16_t>(n);
            else
                value = static_cast<std::int8_t>(n);

            return std::make_unique<requests::ConstantExpression>(std::move(value));
        }
    } catch (...) {
        // Ignore errors, try more variants
    }

    // Next one would be float, but do not try it, due to precision errors.

    // Try double
    try {
        const auto n = std::stod(value);
        return std::make_unique<requests::ConstantExpression>(n);
    } catch (...) {
        // No more variants to try, report error
        throw std::invalid_argument("Invalid numeric literal");
    }
}

requests::ExpressionPtr ExpressionFactory::createStringConstant(const antlr4::Token* token) const
{
    auto s = token->getText();
    // Remove quotes
    s.pop_back();
    s.erase(0, 1);
    return std::make_unique<requests::ConstantExpression>(Variant(std::move(s)));
}

requests::ExpressionPtr ExpressionFactory::createBinaryConstant(const antlr4::Token* token) const
{
    const auto hexLiteral = token->getText();
    // Exclude leading "x'" and trailing "'"
    const auto hexLength = hexLiteral.length() - 3;
    if (hexLength % 2 == 1) throw std::runtime_error("Odd number of characters in the hex string");
    BinaryValue binaryValue(hexLength / 2);
    if (hexLength > 0) {
        boost::algorithm::unhex(hexLiteral.data() + 2, hexLiteral.data() + hexLiteral.length() - 1,
                binaryValue.data());
    }
    return std::make_unique<requests::ConstantExpression>(Variant(std::move(binaryValue)));
}

requests::ExpressionPtr ExpressionFactory::createConstant(const antlr4::Token* token) const
{
    auto tokenType = token->getType();

    switch (tokenType) {
        case SiodbParser::NUMERIC_LITERAL: return createNumericConstant(token);
        case SiodbParser::STRING_LITERAL: return createStringConstant(token);
        case SiodbParser::BLOB_LITERAL: return createBinaryConstant(token);
        case SiodbParser::K_NULL: return std::make_unique<requests::ConstantExpression>();
        case SiodbParser::K_CURRENT_TIME:
        case SiodbParser::K_CURRENT_DATE:
        case SiodbParser::K_CURRENT_TIMESTAMP: {
            const auto rawTime = std::time(nullptr);
            struct tm timeInfo;
            char buffer[80];
            localtime_r(&rawTime, &timeInfo);
            strftime(buffer, sizeof(buffer), Variant::kDefaultDateTimeFormat, &timeInfo);
            Variant value(buffer, Variant::AsDateTime(), Variant::kDefaultDateTimeFormat);
            return std::make_unique<requests::ConstantExpression>(std::move(value));
        }
        case SiodbParser::K_TRUE:
        case SiodbParser::K_FALSE:
            return std::make_unique<requests::ConstantExpression>(tokenType == SiodbParser::K_TRUE);
        default: throw std::invalid_argument("Invalid constant type");
    }
}

requests::ExpressionPtr ExpressionFactory::createConstant(const antlr4::tree::ParseTree* node) const
{
    const auto terminal = dynamic_cast<antlr4::tree::TerminalNode*>(node->children.front());
    if (!terminal) {
        throw std::invalid_argument(
                "Expression malformed: Literal node has no terminal after 2 childs deep");
    }
    const auto symbol = terminal->getSymbol();
    if (!symbol) throw std::invalid_argument("Expression malformed: terminal has no symbol");
    return createConstant(symbol);
}

bool ExpressionFactory::isNonLogicalBinaryOperator(std::size_t terminalType) const noexcept
{
    switch (terminalType) {
        case SiodbParser::LT:
        case SiodbParser::LT_EQ:
        case SiodbParser::EQ:
        case SiodbParser::GT:
        case SiodbParser::GT_EQ:
        case SiodbParser::PLUS:
        case SiodbParser::MINUS:
        case SiodbParser::STAR:
        case SiodbParser::DIV:
        case SiodbParser::MOD:
        case SiodbParser::ASSIGN:
        case SiodbParser::NOT_EQ1:
        case SiodbParser::NOT_EQ2:
        case SiodbParser::K_LIKE:
        case SiodbParser::PIPE:
        case SiodbParser::AMP:
        case SiodbParser::LT2:
        case SiodbParser::GT2:
        case SiodbParser::CARAT:
        case SiodbParser::PIPE2:
        case SiodbParser::K_IS: return true;
        default: return false;
    }
}

bool ExpressionFactory::isLogicalBinaryOperator(std::size_t terminalType) const noexcept
{
    switch (terminalType) {
        case SiodbParser::K_AND:
        case SiodbParser::K_OR: return true;
        default: return false;
    }
}

requests::ExpressionPtr ExpressionFactory::createColumnValueExpression(
        antlr4::tree::ParseTree* tableNode, antlr4::tree::ParseTree* columnNode) const
{
    if (!m_allowColumnExpressions) {
        throw std::invalid_argument(utils::StringBuilder() << "Column " << columnNode->getText()
                                                           << " is not allowed in this context");
    }

    std::string tableName;
    std::string columnName;

    if (tableNode)
        tableName = boost::to_upper_copy(helpers::getAnyNameText(tableNode->children.at(0)));

    if (columnNode)
        columnName = boost::to_upper_copy(helpers::getAnyNameText(columnNode->children.at(0)));
    else
        throw std::invalid_argument("Column node is nullptr");

    if (columnName.empty()) throw std::invalid_argument("Column node is invalid");

    return std::make_unique<requests::SingleColumnExpression>(
            std::move(tableName), std::move(columnName));
}

requests::ExpressionPtr ExpressionFactory::createBetweenExpression(
        antlr4::tree::ParseTree* expression, antlr4::tree::ParseTree* leftBound,
        antlr4::tree::ParseTree* rightBound, bool notBetween) const
{
    auto valueExpr = createSimpleExpression(expression);
    auto leftBoundExpr = createSimpleExpression(leftBound);
    auto rightBoundExpr = createSimpleExpression(rightBound);

    if (valueExpr->getType() == requests::ExpressionType::kConstant
            && leftBoundExpr->getType() == requests::ExpressionType::kConstant
            && rightBoundExpr->getType() == requests::ExpressionType::kConstant) {
        const auto& value = dynamic_cast<requests::ConstantExpression&>(*valueExpr).getValue();
        const auto& leftBound =
                dynamic_cast<requests::ConstantExpression&>(*leftBoundExpr).getValue();
        const auto& rightBound =
                dynamic_cast<requests::ConstantExpression&>(*rightBoundExpr).getValue();
        return std::make_unique<requests::ConstantExpression>(
                value >= leftBound && value <= rightBound);
    }

    return std::make_unique<requests::BetweenOperator>(
            std::move(valueExpr), std::move(leftBoundExpr), std::move(rightBoundExpr), notBetween);
}

requests::ExpressionPtr ExpressionFactory::createUnaryOperator(
        antlr4::tree::ParseTree* operatorNode, antlr4::tree::ParseTree* operandNode) const
{
    // operatorNode rule is unary_operator
    // unary_operator child is terminal with unary operator type
    if (operatorNode->children.size() != 1) {
        throw std::invalid_argument(
                "Expression malformed: Unary operator should have exactly one child");
    }

    auto type = helpers::getTerminalType(operatorNode->children.front());
    switch (type) {
        case SiodbParser::PLUS:
            return std::make_unique<requests::UnaryPlusOperator>(
                    createSimpleExpression(operandNode));
        case SiodbParser::MINUS: {
            return std::make_unique<requests::UnaryMinusOperator>(
                    createSimpleExpression(operandNode));
        }
        case SiodbParser::TILDE: {
            return std::make_unique<requests::ComplementOperator>(
                    createSimpleExpression(operandNode));
        }
        default: throw std::invalid_argument("Unrecognized unary operator");
    }
}

requests::ExpressionPtr ExpressionFactory::createNonLogicalBinaryOperator(
        antlr4::tree::ParseTree* leftNode, antlr4::tree::ParseTree* operatorNode,
        antlr4::tree::ParseTree* rightNode) const
{
    switch (helpers::getTerminalType(operatorNode)) {
        case SiodbParser::LT: {
            return std::make_unique<requests::LessOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::LT_EQ: {
            return std::make_unique<requests::LessOrEqualOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::ASSIGN: {
            return std::make_unique<requests::EqualOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::GT: {
            return std::make_unique<requests::GreaterOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::GT_EQ: {
            return std::make_unique<requests::GreaterOrEqualOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::PLUS: {
            return std::make_unique<requests::AddOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::MINUS: {
            return std::make_unique<requests::SubtractOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::MOD: {
            return std::make_unique<requests::ModuloOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::STAR: {
            return std::make_unique<requests::MultiplyOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::DIV: {
            return std::make_unique<requests::DivideOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::PIPE: {
            return std::make_unique<requests::BitwiseOrOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::AMP: {
            return std::make_unique<requests::BitwiseAndOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::CARAT: {
            return std::make_unique<requests::BitwiseXorOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::LT2: {
            return std::make_unique<requests::LeftShiftOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::GT2: {
            return std::make_unique<requests::RightShiftOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::K_LIKE: {
            return std::make_unique<requests::LikeOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode), false);
        }
        case SiodbParser::NOT_EQ1:
        case SiodbParser::NOT_EQ2: {
            return std::make_unique<requests::NotEqualOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::PIPE2: {
            return std::make_unique<requests::ConcatenationOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode));
        }
        case SiodbParser::K_IS: {
            return std::make_unique<requests::IsOperator>(
                    createSimpleExpression(leftNode), createSimpleExpression(rightNode), false);
        }
        default: throw std::invalid_argument("Unrecognized binary operator");
    }
}

bool ExpressionFactory::checkInOperator(const antlr4::tree::ParseTree* node) const noexcept
{
    if (node->children.size() < 5) return false;

    if (helpers::getNonTerminalType(node->children[0]) != SiodbParser::RuleSimple_expr)
        return false;

    std::size_t openParIndex = 0;
    if (helpers::getTerminalType(node->children[1]) == SiodbParser::K_IN)
        openParIndex = 2;
    else if (helpers::getTerminalType(node->children[1]) == SiodbParser::K_NOT
             && helpers::getTerminalType(node->children[2]) == SiodbParser::K_IN)
        openParIndex = 3;
    else
        return false;

    if (helpers::getTerminalType(node->children[openParIndex]) != SiodbParser::OPEN_PAR)
        return false;

    if (helpers::getTerminalType(node->children[node->children.size() - 1])
            != SiodbParser::CLOSE_PAR)
        return false;

    return true;
}

requests::ExpressionPtr ExpressionFactory::createInOperator(antlr4::tree::ParseTree* node) const
{
    auto valueExpression = createSimpleExpression(node->children[0]);
    bool notIn = helpers::getTerminalType(node->children[1]) == SiodbParser::K_NOT;
    std::vector<requests::ExpressionPtr> variants;
    std::size_t curValueIndex = notIn ? 4 : 3;
    for (; curValueIndex < node->children.size(); curValueIndex += 2) {
        variants.push_back(createSimpleExpression(node->children[curValueIndex]));
        // Skip comma
    }

    if (variants.empty()) throw std::invalid_argument("In operator has no variants");

    return std::make_unique<requests::InOperator>(
            std::move(valueExpression), std::move(variants), notIn);
}

requests::ExpressionPtr ExpressionFactory::createLogicalBinaryOperator(
        antlr4::tree::ParseTree* leftNode, antlr4::tree::ParseTree* operatorNode,
        antlr4::tree::ParseTree* rightNode) const
{
    switch (helpers::getTerminalType(operatorNode)) {
        case SiodbParser::K_AND: {
            return std::make_unique<requests::LogicalAndOperator>(
                    createExpression(leftNode), createExpression(rightNode));
        }
        case SiodbParser::K_OR: {
            return std::make_unique<requests::LogicalOrOperator>(
                    createExpression(leftNode), createExpression(rightNode));
        }
        default: throw std::invalid_argument("Unrecognized logical binary operator");
    }
}

requests::ExpressionPtr ExpressionFactory::createSimpleExpression(
        antlr4::tree::ParseTree* node) const
{
    const auto childCount = node->children.size();
    if (checkInOperator(node))
        return createInOperator(node);
    else if (childCount == 1) {
        const auto childNode = node->children[0];
        const auto rule = helpers::getNonTerminalType(childNode);
        if (rule == SiodbParser::RuleLiteral_value)
            return createConstant(childNode);
        else if (rule == SiodbParser::RuleColumn_name)
            return createColumnValueExpression(nullptr, childNode);

    } else if (childCount == 2) {
        // the only case with 2 childs is: unary_operator, [expression, column_name]
        const auto leftNode = node->children[0];
        const auto rightNode = node->children[1];
        const bool rightNodeValid =
                helpers::getNonTerminalType(rightNode) == SiodbParser::RuleColumn_name
                || helpers::getNonTerminalType(rightNode) == SiodbParser::RuleSimple_expr;
        // NOT EXPR is not under RuleUnary_operator
        if (helpers::getNonTerminalType(leftNode) == SiodbParser::RuleUnary_operator
                && rightNodeValid) {
            return createUnaryOperator(leftNode, rightNode);
        } else {
            throw std::invalid_argument("Invalid unary expression");
        }
    } else if (childCount == 3) {
        const auto leftNode = node->children[0];
        const auto midNode = node->children[1];
        const auto rightNode = node->children[2];
        // Check case: Expr Operator Expr
        if (helpers::getNonTerminalType(leftNode) == SiodbParser::RuleSimple_expr
                && isNonLogicalBinaryOperator(helpers::getTerminalType(midNode))
                && helpers::getNonTerminalType(rightNode) == SiodbParser::RuleSimple_expr) {
            return createNonLogicalBinaryOperator(leftNode, midNode, rightNode);
        }
        // Check case ( simple_expr )
        else if (helpers::getTerminalType(leftNode) == SiodbParser::OPEN_PAR
                 && helpers::getNonTerminalType(midNode) == SiodbParser::RuleSimple_expr
                 && helpers::getTerminalType(rightNode) == SiodbParser::CLOSE_PAR) {
            return createExpression(midNode);
        }
        // Check case tableName . columnName
        else if (helpers::getNonTerminalType(leftNode) == SiodbParser::RuleTable_name
                 && helpers::getTerminalType(midNode) == SiodbParser::DOT
                 && helpers::getNonTerminalType(rightNode) == SiodbParser::RuleColumn_name) {
            return createColumnValueExpression(leftNode, rightNode);
        }
    } else if (childCount == 4) {
        const auto& node0 = node->children[0];
        const auto& node1 = node->children[1];
        const auto& node2 = node->children[2];
        const auto& node3 = node->children[3];

        // Check expr NOT LIKE expr
        if (helpers::getNonTerminalType(node0) == SiodbParser::RuleSimple_expr
                && helpers::getTerminalType(node1) == SiodbParser::K_NOT
                && helpers::getTerminalType(node2) == SiodbParser::K_LIKE
                && helpers::getNonTerminalType(node3) == SiodbParser::RuleSimple_expr) {
            return std::make_unique<requests::LikeOperator>(
                    createSimpleExpression(node0), createSimpleExpression(node3), true);
        }
        // Check expr IS NOT expr
        if (helpers::getNonTerminalType(node0) == SiodbParser::RuleSimple_expr
                && helpers::getTerminalType(node1) == SiodbParser::K_IS
                && helpers::getTerminalType(node2) == SiodbParser::K_NOT
                && helpers::getNonTerminalType(node3) == SiodbParser::RuleSimple_expr) {
            return std::make_unique<requests::IsOperator>(
                    createSimpleExpression(node0), createSimpleExpression(node3), true);
        }
    } else if (childCount == 5) {
        // Check expr BETWEEN expr AND Expr
        const auto& node0 = node->children[0];
        const auto& node1 = node->children[1];
        const auto& node2 = node->children[2];
        const auto& node3 = node->children[3];
        const auto& node4 = node->children[4];

        if (helpers::getNonTerminalType(node0) == SiodbParser::RuleSimple_expr
                && helpers::getTerminalType(node1) == SiodbParser::K_BETWEEN
                && helpers::getNonTerminalType(node2) == SiodbParser::RuleSimple_expr
                && helpers::getTerminalType(node3) == SiodbParser::K_AND
                && helpers::getNonTerminalType(node4) == SiodbParser::RuleSimple_expr) {
            return createBetweenExpression(node0, node2, node4, false);
        }
        // Check database . table . column
        else if (helpers::getNonTerminalType(node0) == SiodbParser::RuleDatabase_name
                 && helpers::getTerminalType(node1) == SiodbParser::DOT
                 && helpers::getNonTerminalType(node2) == SiodbParser::RuleTable_name
                 && helpers::getTerminalType(node3) == SiodbParser::DOT
                 && helpers::getNonTerminalType(node4) == SiodbParser::RuleColumn_name) {
            throw std::runtime_error("Column name with a database not supported");
        }
    } else if (childCount == 6) {
        // Check expr NOT BETWEEN expr AND Expr
        if (helpers::getNonTerminalType(node->children[0]) == SiodbParser::RuleSimple_expr
                && helpers::getTerminalType(node->children[1]) == SiodbParser::K_NOT
                && helpers::getTerminalType(node->children[2]) == SiodbParser::K_BETWEEN
                && helpers::getNonTerminalType(node->children[3]) == SiodbParser::RuleSimple_expr
                && helpers::getTerminalType(node->children[4]) == SiodbParser::K_AND
                && helpers::getNonTerminalType(node->children[5]) == SiodbParser::RuleSimple_expr) {
            return createBetweenExpression(
                    node->children[0], node->children[3], node->children[5], true);
        }
    }

    throw std::invalid_argument("Node is not valid simple expression or not supported");
}

}  // namespace siodb::iomgr::dbengine::parser
