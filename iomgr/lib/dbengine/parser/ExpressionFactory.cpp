// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ExpressionFactory.h"

// Project headers
#include "AntlrHelpers.h"
#include "DBEngineRequestFactoryError.h"
#include "antlr_wrappers/SiodbParserWrapper.h"

// Common project headers
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/AllExpressions.h>

// Boost headers
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/uuid/string_generator.hpp>

namespace siodb::iomgr::dbengine::parser {

requests::ExpressionPtr ExpressionFactory::createExpression(antlr4::tree::ParseTree* node)
{
    const auto rule = helpers::getNonTerminalType(node);
    const auto childCount = node->children.size();

    // Expect literal or column name
    switch (rule) {
        case SiodbParser::RuleSigned_number: {
            std::size_t literalNodeIndex = 0;
            bool negate = false;
            if (childCount > 1) {
                literalNodeIndex = 1;
                negate = helpers::getMaybeTerminalType(node->children[0]) == SiodbParser::MINUS;
            }
            return createConstant(node, literalNodeIndex, negate);
        }
        case SiodbParser::RuleLiteral_value: return createConstant(node);
        case SiodbParser::RuleExpr: {
            if (childCount == 1) {
                // Only simple expression could be in this case
                return createSimpleExpression(node->children[0]);
            } else if (childCount == 2) {
                const auto leftNode = node->children[0];
                const auto rightNode = node->children[1];
                const bool rightNodeValid =
                        helpers::getNonTerminalType(rightNode) == SiodbParser::RuleColumn_name
                        || helpers::getNonTerminalType(rightNode) == SiodbParser::RuleExpr;
                if (helpers::getMaybeTerminalType(leftNode) == SiodbParser::K_NOT
                        && rightNodeValid) {
                    return std::make_unique<requests::LogicalNotOperator>(
                            createExpression(rightNode));
                }
            } else if (childCount == 3) {
                const auto leftNode = node->children[0];
                const auto midNode = node->children[1];
                const auto rightNode = node->children[2];
                // Check case: Expr Operator Expr
                if (helpers::getNonTerminalType(leftNode) == SiodbParser::RuleExpr
                        && isLogicalBinaryOperator(helpers::getMaybeTerminalType(midNode))
                        && helpers::getNonTerminalType(rightNode) == SiodbParser::RuleExpr) {
                    return createLogicalBinaryOperator(leftNode, midNode, rightNode);
                }
                // Check case ( expr )
                else if (helpers::getMaybeTerminalType(leftNode) == SiodbParser::OPEN_PAR
                         && helpers::getNonTerminalType(midNode) == SiodbParser::RuleExpr
                         && helpers::getMaybeTerminalType(rightNode) == SiodbParser::CLOSE_PAR) {
                    return createExpression(midNode);
                }
            }
            std::size_t line = 1, column = 1;
            helpers::findFirstTerminalAndCapturePosition(node, 0, line, column);
            throw DBEngineRequestFactoryError(
                    m_parser.injectError(line, column, "Expression is invalid"));
        }
        case SiodbParser::RuleFunction_call: {
            // TODO: Support functions
            std::size_t line = 1, column = 1;
            helpers::findFirstTerminalAndCapturePosition(node, 0, line, column);
            throw DBEngineRequestFactoryError(
                    m_parser.injectError(line, column, "Functions are not supported yet"));
        }
        case SiodbParser::RuleSimple_expr: return createSimpleExpression(node);
        default: break;
    }

    std::size_t line = 1, column = 1;
    helpers::findFirstTerminalAndCapturePosition(node, 0, line, column);
    throw DBEngineRequestFactoryError(m_parser.injectError(
            line, column, "SQL term is not valid expression or not supported"));
}

Variant ExpressionFactory::createConstantValue(antlr4::tree::ParseTree* node)
{
    // Expect literal
    const auto rule = helpers::getNonTerminalType(node);
    switch (rule) {
        case SiodbParser::RuleSigned_number: {
            std::size_t literalNodeIndex = 0;
            bool negate = false;
            if (node->children.size() > 1) {
                literalNodeIndex = 1;
                negate = helpers::getMaybeTerminalType(node->children[0]) == SiodbParser::MINUS;
            }
            return createConstantValue(node, literalNodeIndex, negate);
        }
        case SiodbParser::RuleLiteral_value: return createConstantValue(node, 0, false);
        case SIZE_MAX: {
            // Likely terminal node
            const auto terminal = dynamic_cast<antlr4::tree::TerminalNode*>(node);
            if (terminal) return createConstantValue(terminal, false);
            break;
        }
        default: break;
    }

    std::size_t line = 1, column = 1;
    helpers::findFirstTerminalAndCapturePosition(node, 0, line, column);
    throw DBEngineRequestFactoryError(m_parser.injectError(line, column, "Not a valid constant"));
}

// --- internals ---

requests::ExpressionPtr ExpressionFactory::createConstant(const antlr4::Token* token, bool negate)
{
    return std::make_unique<requests::ConstantExpression>(createConstantValue(token, negate));
}

requests::ExpressionPtr ExpressionFactory::createConstant(
        const antlr4::tree::ParseTree* node, std::size_t literalNodeIndex, bool negate)
{
    return std::make_unique<requests::ConstantExpression>(
            createConstantValue(node, literalNodeIndex, negate));
}

Variant ExpressionFactory::createConstantValue(
        const antlr4::tree::ParseTree* node, std::size_t literalNodeIndex, bool negate)
{
    const auto child = node->children.at(literalNodeIndex);
    const auto terminal = dynamic_cast<antlr4::tree::TerminalNode*>(child);
    if (terminal)
        return createConstantValue(terminal, negate);
    else {
        std::size_t line = 1, column = 1;
        helpers::findFirstTerminalAndCapturePosition(child, 0, line, column);
        throw DBEngineRequestFactoryError(m_parser.injectError(line, column,
                "Expression malformed: Literal node has no terminal after 2 children deep"));
    }
}

Variant ExpressionFactory::createConstantValue(antlr4::tree::TerminalNode* terminal, bool negate)
{
    const auto symbol = terminal->getSymbol();
    if (symbol) return createConstantValue(symbol, negate);
    std::size_t line = 1, column = 1;
    helpers::findFirstTerminalAndCapturePosition(terminal, 0, line, column);
    throw DBEngineRequestFactoryError(
            m_parser.injectError(line, column, "Expression malformed: terminal has no symbol"));
}

Variant ExpressionFactory::createConstantValue(const antlr4::Token* token, bool negate)
{
    switch (token->getType()) {
        case SiodbParser::K_NULL: return Variant();
        case SiodbParser::K_TRUE:
        case SiodbParser::K_FALSE: return token->getType() == SiodbParser::K_TRUE;
        case SiodbParser::NUMERIC_LITERAL: return createNumericConstantValue(token, negate);
        case SiodbParser::STRING_LITERAL: return createStringConstantValue(token);
        case SiodbParser::BLOB_LITERAL: return createBinaryConstantValue(token);
        case SiodbParser::K_CURRENT_TIME: {
            RawDateTime dt;
            dt.m_datePart = kZeroRawDate;
            dt.m_timePart = RawTime(std::time(nullptr));
            return Variant(dt);
        }
        case SiodbParser::K_CURRENT_DATE: {
            RawDateTime dt;
            dt.m_datePart = RawDate(std::time(nullptr));
            return Variant(dt);
        }
        case SiodbParser::K_CURRENT_TIMESTAMP: return Variant(RawDateTime(std::time(nullptr)));
        default: {
            throw DBEngineRequestFactoryError(m_parser.injectError(
                    token->getLine(), token->getCharPositionInLine() + 1, "Invalid constant type"));
        }
    }
}

Variant ExpressionFactory::createNumericConstantValue(const antlr4::Token* token, bool negate)
{
    const auto text = token->getText();
    try {
        std::size_t end = 0;
        auto n = std::stoull(text, &end);
        if (text.size() == end) {
            if (negate) n = -n;
            if (n > std::numeric_limits<std::uint32_t>::max())
                return static_cast<std::uint64_t>(n);
            else if (n > std::numeric_limits<std::uint16_t>::max())
                return static_cast<std::uint32_t>(n);
            else if (n > std::numeric_limits<std::uint8_t>::max())
                return static_cast<std::uint16_t>(n);
            else
                return static_cast<std::uint8_t>(n);
        }
    } catch (...) {
        // Ignore errors, try more variants
    }

    // Try signed integer
    try {
        std::size_t end = 0;
        auto n = std::stoll(text, &end);
        if (text.size() == end) {
            if (negate) n = -n;
            if (n < std::numeric_limits<std::int32_t>::min()
                    || n > std::numeric_limits<std::int32_t>::max())
                return static_cast<std::int64_t>(n);
            else if (n < std::numeric_limits<std::int16_t>::min()
                     || n > std::numeric_limits<std::int16_t>::max())
                return static_cast<std::int32_t>(n);
            else if (n < std::numeric_limits<std::int8_t>::min()
                     || n > std::numeric_limits<std::int8_t>::max())
                return static_cast<std::int16_t>(n);
            else
                return static_cast<std::int8_t>(n);
        }
    } catch (...) {
        // Ignore errors, try more variants
    }

    // Try double. Do not try float due to precision errors.
    try {
        const auto n = std::stod(text);
        return negate ? -n : n;
    } catch (...) {
        // No more variants to try, report error
        throw DBEngineRequestFactoryError(m_parser.injectError(
                token->getLine(), token->getCharPositionInLine() + 1, "Invalid numeric literal"));
    }
}

Variant ExpressionFactory::createStringConstantValue(const antlr4::Token* token)
{
    return helpers::unquoteString(token->getText());
}

Variant ExpressionFactory::createBinaryConstantValue(const antlr4::Token* token)
{
    const auto hexLiteral = token->getText();
    // Exclude leading "x'" and trailing "'"
    const auto hexLength = hexLiteral.length() - 3;
    if (hexLength % 2 == 1) {
        throw DBEngineRequestFactoryError(m_parser.injectError(token->getLine(),
                token->getCharPositionInLine() + 1, "Odd number of characters in the hex literal"));
    }
    BinaryValue binaryValue(hexLength / 2);
    if (hexLength > 0) {
        try {
            boost::algorithm::unhex(hexLiteral.data() + 2,
                    hexLiteral.data() + hexLiteral.length() - 1, binaryValue.data());
        } catch (boost::algorithm::non_hex_input& ex) {
            throw DBEngineRequestFactoryError(m_parser.injectError(token->getLine(),
                    token->getCharPositionInLine() + 1, "Invalid character in the hex literal"));
        }
    }
    return binaryValue;
}

requests::ExpressionPtr ExpressionFactory::createColumnValueExpression(
        antlr4::tree::ParseTree* tableNode, antlr4::tree::ParseTree* columnNode)
{
    if (!m_allowColumnExpressions) {
        std::size_t line = 1, column = 1;
        helpers::findFirstTerminalAndCapturePosition(tableNode, 0, line, column);
        throw DBEngineRequestFactoryError(m_parser.injectError(line, column,
                stdext::string_builder()
                        << "Column " << columnNode->getText() << " is not allowed"));
    }

    std::string tableName;
    if (tableNode) {
        tableName = helpers::getAnyNameText(tableNode->children.at(0));
        boost::to_upper(tableName);
    }

    std::string columnName;
    if (columnNode) {
        columnName = helpers::getAnyNameText(columnNode->children.at(0));
        boost::to_upper(columnName);
    } else {
        std::size_t line = 1, column = 1;
        helpers::findFirstTerminalAndCapturePosition(tableNode, 0, line, column);
        throw DBEngineRequestFactoryError(
                m_parser.injectError(line, column, "Missing column term"));
    }

    if (columnName.empty()) {
        std::size_t line = 1, column = 1;
        helpers::findFirstTerminalAndCapturePosition(tableNode, 0, line, column);
        throw DBEngineRequestFactoryError(
                m_parser.injectError(line, column, "Column term is invalid"));
    }

    return std::make_unique<requests::SingleColumnExpression>(
            std::move(tableName), std::move(columnName));
}

requests::ExpressionPtr ExpressionFactory::createBetweenExpression(
        antlr4::tree::ParseTree* expression, antlr4::tree::ParseTree* leftBound,
        antlr4::tree::ParseTree* rightBound, bool notBetween)
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
        antlr4::tree::ParseTree* operatorNode, antlr4::tree::ParseTree* operandNode)
{
    // operatorNode rule is unary_operator
    // unary_operator child is terminal with unary operator type
    if (operatorNode->children.size() != 1) {
        std::size_t line = 1, column = 1;
        helpers::findFirstTerminalAndCapturePosition(operatorNode, 0, line, column);
        throw DBEngineRequestFactoryError(m_parser.injectError(line, column,
                "Expression malformed: Unary operator should have exactly one operand"));
    }

    const auto type = helpers::getMaybeTerminalType(operatorNode->children.front());
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
        default: {
            std::size_t line = 1, column = 1;
            helpers::findFirstTerminalAndCapturePosition(operatorNode, 0, line, column);
            throw DBEngineRequestFactoryError(
                    m_parser.injectError(line, column, "Unrecognized unary operator"));
        }
    }
}

requests::ExpressionPtr ExpressionFactory::createNonLogicalBinaryOperator(
        antlr4::tree::ParseTree* leftNode, antlr4::tree::ParseTree* operatorNode,
        antlr4::tree::ParseTree* rightNode)
{
    switch (helpers::getMaybeTerminalType(operatorNode)) {
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
        default: {
            std::size_t line = 1, column = 1;
            helpers::findFirstTerminalAndCapturePosition(operatorNode, 0, line, column);
            throw DBEngineRequestFactoryError(
                    m_parser.injectError(line, column, "Unrecognized binary operator"));
        }
    }
}

requests::ExpressionPtr ExpressionFactory::createInOperator(antlr4::tree::ParseTree* node)
{
    auto valueExpr = createSimpleExpression(node->children[0]);
    const bool isNotIn = helpers::getMaybeTerminalType(node->children[1]) == SiodbParser::K_NOT;

    std::vector<requests::ExpressionPtr> variants;
    for (std::size_t i = isNotIn ? 4 : 3, n = node->children.size(); i < n; i += 2)
        variants.push_back(createSimpleExpression(node->children[i]));

    if (variants.empty()) {
        std::size_t line = 1, column = 1;
        helpers::findFirstTerminalAndCapturePosition(node, 0, line, column);
        throw DBEngineRequestFactoryError(
                m_parser.injectError(line, column, "Operator IN has no variants"));
    }

    return std::make_unique<requests::InOperator>(
            std::move(valueExpr), std::move(variants), isNotIn);
}

requests::ExpressionPtr ExpressionFactory::createLogicalBinaryOperator(
        antlr4::tree::ParseTree* leftNode, antlr4::tree::ParseTree* operatorNode,
        antlr4::tree::ParseTree* rightNode)
{
    switch (helpers::getMaybeTerminalType(operatorNode)) {
        case SiodbParser::K_AND: {
            return std::make_unique<requests::LogicalAndOperator>(
                    createExpression(leftNode), createExpression(rightNode));
        }
        case SiodbParser::K_OR: {
            return std::make_unique<requests::LogicalOrOperator>(
                    createExpression(leftNode), createExpression(rightNode));
        }
        default: {
            std::size_t line = 1, column = 1;
            helpers::findFirstTerminalAndCapturePosition(operatorNode, 0, line, column);
            throw DBEngineRequestFactoryError(
                    m_parser.injectError(line, column, "Unrecognized logical binary operator"));
        }
    }
}

requests::ExpressionPtr ExpressionFactory::createSimpleExpression(antlr4::tree::ParseTree* node)
{
    if (isInOperator(node)) return createInOperator(node);
    switch (node->children.size()) {
        case 1: {
            const auto childNode = node->children[0];
            const auto rule = helpers::getNonTerminalType(childNode);
            switch (rule) {
                case SiodbParser::RuleLiteral_value: return createConstant(childNode);
                case SiodbParser::RuleColumn_name:
                    return createColumnValueExpression(nullptr, childNode);
                default: break;
            }
            break;
        }
        case 2: {
            // the only case with 2 children is: unary_operator, [expression, column_name]
            const auto leftNode = node->children[0];
            const auto rightNode = node->children[1];
            const bool rightNodeValid =
                    helpers::getNonTerminalType(rightNode) == SiodbParser::RuleColumn_name
                    || helpers::getNonTerminalType(rightNode) == SiodbParser::RuleSimple_expr;
            // NOT EXPR is not under RuleUnary_operator
            if (helpers::getNonTerminalType(leftNode) == SiodbParser::RuleUnary_operator
                    && rightNodeValid) {
                return createUnaryOperator(leftNode, rightNode);
            }
            std::size_t line = 1, column = 1;
            helpers::findFirstTerminalAndCapturePosition(node, 0, line, column);
            throw DBEngineRequestFactoryError(
                    m_parser.injectError(line, column, "Invalid unary expression"));
        }
        case 3: {
            const auto leftNode = node->children[0];
            const auto midNode = node->children[1];
            const auto rightNode = node->children[2];
            // Check for the "Expr Operator Expr"
            if (helpers::getNonTerminalType(leftNode) == SiodbParser::RuleSimple_expr
                    && isNonLogicalBinaryOperator(helpers::getMaybeTerminalType(midNode))
                    && helpers::getNonTerminalType(rightNode) == SiodbParser::RuleSimple_expr) {
                return createNonLogicalBinaryOperator(leftNode, midNode, rightNode);
            }
            // Check for the  '(' simple_expr ')'
            else if (helpers::getMaybeTerminalType(leftNode) == SiodbParser::OPEN_PAR
                     && helpers::getNonTerminalType(midNode) == SiodbParser::RuleSimple_expr
                     && helpers::getMaybeTerminalType(rightNode) == SiodbParser::CLOSE_PAR) {
                return createExpression(midNode);
            }
            // Check for the "tableName . columnName"
            else if (helpers::getNonTerminalType(leftNode) == SiodbParser::RuleTable_name
                     && helpers::getMaybeTerminalType(midNode) == SiodbParser::DOT
                     && helpers::getNonTerminalType(rightNode) == SiodbParser::RuleColumn_name) {
                return createColumnValueExpression(leftNode, rightNode);
            }
            break;
        }
        case 4: {
            const auto& node0 = node->children[0];
            const auto& node1 = node->children[1];
            const auto& node2 = node->children[2];
            const auto& node3 = node->children[3];

            // Check for the "expr NOT LIKE expr"
            if (helpers::getNonTerminalType(node0) == SiodbParser::RuleSimple_expr
                    && helpers::getMaybeTerminalType(node1) == SiodbParser::K_NOT
                    && helpers::getMaybeTerminalType(node2) == SiodbParser::K_LIKE
                    && helpers::getNonTerminalType(node3) == SiodbParser::RuleSimple_expr) {
                return std::make_unique<requests::LikeOperator>(
                        createSimpleExpression(node0), createSimpleExpression(node3), true);
            }

            // Check for the "expr IS NOT expr"
            if (helpers::getNonTerminalType(node0) == SiodbParser::RuleSimple_expr
                    && helpers::getMaybeTerminalType(node1) == SiodbParser::K_IS
                    && helpers::getMaybeTerminalType(node2) == SiodbParser::K_NOT
                    && helpers::getNonTerminalType(node3) == SiodbParser::RuleSimple_expr) {
                return std::make_unique<requests::IsOperator>(
                        createSimpleExpression(node0), createSimpleExpression(node3), true);
            }
            break;
        }
        case 5: {
            // Check for the "expr BETWEEN expr AND expr"
            const auto& node0 = node->children[0];
            const auto& node1 = node->children[1];
            const auto& node2 = node->children[2];
            const auto& node3 = node->children[3];
            const auto& node4 = node->children[4];

            if (helpers::getNonTerminalType(node0) == SiodbParser::RuleSimple_expr
                    && helpers::getMaybeTerminalType(node1) == SiodbParser::K_BETWEEN
                    && helpers::getNonTerminalType(node2) == SiodbParser::RuleSimple_expr
                    && helpers::getMaybeTerminalType(node3) == SiodbParser::K_AND
                    && helpers::getNonTerminalType(node4) == SiodbParser::RuleSimple_expr) {
                return createBetweenExpression(node0, node2, node4, false);
            }
            // Check for the "database . table . column"
            else if (helpers::getNonTerminalType(node0) == SiodbParser::RuleDatabase_name
                     && helpers::getMaybeTerminalType(node1) == SiodbParser::DOT
                     && helpers::getNonTerminalType(node2) == SiodbParser::RuleTable_name
                     && helpers::getMaybeTerminalType(node3) == SiodbParser::DOT
                     && helpers::getNonTerminalType(node4) == SiodbParser::RuleColumn_name) {
                std::size_t line = 1, column = 1;
                helpers::findFirstTerminalAndCapturePosition(node, 0, line, column);
                throw DBEngineRequestFactoryError(m_parser.injectError(
                        line, column, "Column name with a database not supported"));
            }
            break;
        }
        case 6: {
            // Check for the "expr NOT BETWEEN expr AND expr"
            if (helpers::getNonTerminalType(node->children[0]) == SiodbParser::RuleSimple_expr
                    && helpers::getMaybeTerminalType(node->children[1]) == SiodbParser::K_NOT
                    && helpers::getMaybeTerminalType(node->children[2]) == SiodbParser::K_BETWEEN
                    && helpers::getNonTerminalType(node->children[3])
                               == SiodbParser::RuleSimple_expr
                    && helpers::getMaybeTerminalType(node->children[4]) == SiodbParser::K_AND
                    && helpers::getNonTerminalType(node->children[5])
                               == SiodbParser::RuleSimple_expr) {
                return createBetweenExpression(
                        node->children[0], node->children[3], node->children[5], true);
            }
        }
        default: break;
    }
    std::size_t line = 1, column = 1;
    helpers::findFirstTerminalAndCapturePosition(node, 0, line, column);
    throw DBEngineRequestFactoryError(m_parser.injectError(
            line, column, "Term is not valid simple expression or not supported"));
}

bool ExpressionFactory::isNonLogicalBinaryOperator(std::size_t terminalType) noexcept
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

bool ExpressionFactory::isLogicalBinaryOperator(std::size_t terminalType) noexcept
{
    switch (terminalType) {
        case SiodbParser::K_AND:
        case SiodbParser::K_OR: return true;
        default: return false;
    }
}

bool ExpressionFactory::isInOperator(const antlr4::tree::ParseTree* node) noexcept
{
    if (node->children.size() < 5) return false;

    if (helpers::getNonTerminalType(node->children[0]) != SiodbParser::RuleSimple_expr)
        return false;

    std::size_t openParIndex = 0;
    if (helpers::getMaybeTerminalType(node->children[1]) == SiodbParser::K_IN)
        openParIndex = 2;
    else if (helpers::getMaybeTerminalType(node->children[1]) == SiodbParser::K_NOT
             && helpers::getMaybeTerminalType(node->children[2]) == SiodbParser::K_IN)
        openParIndex = 3;
    else
        return false;

    if (helpers::getMaybeTerminalType(node->children[openParIndex]) != SiodbParser::OPEN_PAR)
        return false;

    if (helpers::getMaybeTerminalType(node->children[node->children.size() - 1])
            != SiodbParser::CLOSE_PAR)
        return false;

    return true;
}

}  // namespace siodb::iomgr::dbengine::parser
