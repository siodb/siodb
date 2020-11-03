// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Expression.h"
#include "../SqlParser.h"

namespace siodb::iomgr::dbengine::parser {

class ExpressionFactory {
public:
    /**
     * Initializes object of class ExpressionFactory.
     * @param parser SQL parser object.
     * @param allowColumnExpressions Indication that parser should allow columns in expressions.
     */
    explicit ExpressionFactory(SqlParser& parser, bool allowColumnExpressions = false) noexcept
        : m_parser(parser)
        , m_allowColumnExpressions(allowColumnExpressions)
    {
    }

    /**
     * Creates an expression from expression node.
     * @param node A pointer to a parse tree with expression.
     * @return New expression object.
     * @throw invalid_argument if invalid node encountered.
     * @throw runtime_error if unsupported node encountered.
     * @throw VariantTypeCastError if there is incompatible constant type.
     */
    requests::ExpressionPtr createExpression(antlr4::tree::ParseTree* node);

    /**
     * Creates constant value from expression node.
     * Throws exception if node is not valid constant node expression.
     * @param node A pointer to a parse tree with expression.
     * @return Value object.
     * @throw invalid_argument if invalid node encountered.
     * @throw runtime_error if unsupported node encountered.
     * @throw VariantTypeCastError if there is incompatible constant type.
     */
    Variant createConstantValue(antlr4::tree::ParseTree* node);

private:
    /**
     * Creates constant expression from a token.
     * @param token Token with value.
     * @param negate Indicates that numeric value must be negated.
     * @return New constant expression object.
     * @throw std::invalid_argument if any argument is invalid.
     */
    requests::ExpressionPtr createConstant(const antlr4::Token* token, bool negate = false);

    /**
     * Creates constant expression from a node.
     * @param node Node with value.
     * @param literalNodeIndex Index of child node with constant literal.
     * @param negate Indicates that numeric value must be negated.
     * @return New constant expression object.
     * @throw std::invalid_argument if any argument is invalid.
     */
    requests::ExpressionPtr createConstant(const antlr4::tree::ParseTree* node,
            std::size_t literalNodeIndex = 0, bool negate = false);

    /**
     * Creates constant value from a node.
     * @param node Node with value.
     * @param literalNodeIndex Index of child node with constant literal.
     * @param negate Indicates that numeric value must be negated.
     * @return Value object.
     * @throw std::invalid_argument if any argument is invalid.
     */
    Variant createConstantValue(
            const antlr4::tree::ParseTree* node, std::size_t literalNodeIndex, bool negate);

    /**
     * Creates constant value from a terminal.
     * @param terminal Terminal node.
     * @param negate Indicates that numeric value must be negated.
     * @return Value object.
     * @throw std::invalid_argument if any argument is invalid.
     */
    Variant createConstantValue(antlr4::tree::TerminalNode* terminal, bool negate);

    /**
     * Creates constant value from a token.
     * @param token Token with value.
     * @param negate Indicates that numeric value must be negated.
     * @return Value object.
     * @throw std::invalid_argument if any argument is invalid.
     */
    Variant createConstantValue(const antlr4::Token* token, bool negate);

    /**
     * Creates a numeric constant value.
     * @param token A token with numeric constant.
     * @param negate Indicates that numeric constant must be negated.
     * @return Value object.
     * @throw std::invalid_argument if token is invalid.
     */
    Variant createNumericConstantValue(const antlr4::Token* token, bool negate = false);

    /**
     * Creates a string constant.
     * @param token A token with string constant.
     * @return Value object.
     * @throw std::invalid_argument if token is invalid.
     */
    static Variant createStringConstantValue(const antlr4::Token* token);

    /**
     * Creates a binary constant.
     * @param token A token with binary constant.
     * @return New constant expression object.
     * @throw std::invalid_argument if token is invalid.
     * @throw runtime_error if unsupported node encountered.
     */
    Variant createBinaryConstantValue(const antlr4::Token* token);

    /**
     * Creates column expression.
     * @param tableNode A token with a table.
     * @param columnNode A token with a column.
     * @return New column expression object.
     * @throw std::invalid_argument if any argument is invalid or column expression is not supported.
     */
    requests::ExpressionPtr createColumnValueExpression(
            antlr4::tree::ParseTree* tableNode, antlr4::tree::ParseTree* columnNode);

    /**
     * Creates between expression.
     * @param expression Expression with value.
     * @param leftBound Left bound of between.
     * @param rightBound Right bound of between.
     * @param notBetween Indication wheither between expression has NOT.
     * @return New between expression object.
     * @throw std::invalid_argument if any argument is invalid.
     */
    requests::ExpressionPtr createBetweenExpression(antlr4::tree::ParseTree* expression,
            antlr4::tree::ParseTree* leftBound, antlr4::tree::ParseTree* rightBound,
            bool notBetween);

    /**
     * Creates unary operator expression.
     * @param operatorNode An operator node.
     * @param operandNode An operand node.
     * @return New unary operator expression object.
     * @throw std::invalid_argument if any argument is invalid.
     */
    requests::ExpressionPtr createUnaryOperator(
            antlr4::tree::ParseTree* operatorNode, antlr4::tree::ParseTree* operandNode);

    /**
     * Creates non-logical binary operator expression.
     * @param leftNode Left operand node.
     * @param operatorNode An operator node.
     * @param rightNode Right operand node.
     * @return New non-logical binary operator expression object.
     * @throw std::invalid_argument if any argument is invalid.
     */
    requests::ExpressionPtr createNonLogicalBinaryOperator(antlr4::tree::ParseTree* leftNode,
            antlr4::tree::ParseTree* operatorNode, antlr4::tree::ParseTree* rightNode);

    /**
     * Creates IN operator expression.
     * @param node A node with IN expression.
     * @return New IN operator expression object.
     * @throw std::invalid_argument if @ref node is invalid.
     */
    requests::ExpressionPtr createInOperator(antlr4::tree::ParseTree* node);

    /**
     * Creates logical binary operator expression.
     * @param leftNode Left operand node.
     * @param operatorNode An operator node.
     * @param rightNode Right operand node.
     * @return New logical binary operator expression object.
     * @throw std::invalid_argument if any argument is invalid.
     */
    requests::ExpressionPtr createLogicalBinaryOperator(antlr4::tree::ParseTree* leftNode,
            antlr4::tree::ParseTree* operatorNode, antlr4::tree::ParseTree* rightNode);

    /**
     * Creates an expression from simple expression node.
     * @param node A pointer to a parse tree with expression.
     * @return New expression object.
     * @throw invalid_argument if invalid node occurred.
     * @throw runtime_error if not supported node occurred.
     * @throw VariantTypeCastError if incompatible constant types occurred.
     */
    requests::ExpressionPtr createSimpleExpression(antlr4::tree::ParseTree* node);

    /**
     * Returns indication that specified terminal is non-logical binary operator.
     * @param terminalType Terminal type.
     * @return true if specified terminal is non-logical binary operator, false otherwise.
     */
    static bool isNonLogicalBinaryOperator(std::size_t terminalType) noexcept;

    /**
     * Returns indication that specified terminal is logical binary operator.
     * @param terminalType Terminal type.
     * @return true if specified terminal is logical binary operator, false otherwise.
     */
    static bool isLogicalBinaryOperator(std::size_t terminalType) noexcept;

    /**
     * Returns indication whether node is IN operaror expression.
     * @param node Node.
     * @return New logical binary operator expression object.
     */
    static bool isInOperator(const antlr4::tree::ParseTree* node) noexcept;

private:
    /** SQL parser object */
    SqlParser& m_parser;

    /* Indication that parser should allow columns in expressions */
    const bool m_allowColumnExpressions;
};

}  // namespace siodb::iomgr::dbengine::parser
