// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "AntlrHelpers.h"

// Project headers
#include "antlr_wrappers/SiodbParserWrapper.h"

namespace siodb::iomgr::dbengine::parser::helpers {

std::size_t getStatementCount(const antlr4::tree::ParseTree* tree) noexcept
{
    const auto context = dynamic_cast<const antlr4::RuleContext*>(tree);
    if (!context) return 0;

    if (context->getRuleIndex() == SiodbParser::RuleSql_stmt) return 1;

    std::size_t result = 0;
    for (const auto e : tree->children)
        result += getStatementCount(e);
    return result;
}

antlr4::tree::ParseTree* findStatement(const antlr4::tree::ParseTree* node,
        std::size_t statementIndex, std::size_t& nextIndex) noexcept
{
    const auto context = dynamic_cast<const antlr4::RuleContext*>(node);
    if (!context) return nullptr;

    if (context->getRuleIndex() == SiodbParser::RuleSql_stmt) {
        // Found sql_stmt node
        if (nextIndex == statementIndex)
            return node->children.empty() ? nullptr : node->children.front();
        else {
            ++nextIndex;
            return nullptr;
        }
    }

    // Check statements under this node.
    for (const auto e : node->children) {
        const auto node1 = findStatement(e, statementIndex, nextIndex);
        if (node1) return node1;
    }
    return nullptr;
}

antlr4::tree::ParseTree* findNonTerminal(antlr4::tree::ParseTree* node, const size_t type) noexcept
{
    const auto context = dynamic_cast<antlr4::RuleContext*>(node);
    if (!context) return nullptr;

    if (context->getRuleIndex() == type) return node;

    for (const auto e : node->children) {
        const auto node1 = findNonTerminal(e, type);
        if (node1) return node1;
    }
    return nullptr;
}

antlr4::tree::ParseTree* findTerminal(antlr4::tree::ParseTree* tree, const size_t nonTerminalType,
        const size_t terminalType) noexcept
{
    const auto nonTerminal = findNonTerminal(tree, nonTerminalType);
    return nonTerminal ? findTerminal(nonTerminal, terminalType) : nullptr;
}

antlr4::tree::ParseTree* findTerminal(antlr4::tree::ParseTree* node, std::size_t type) noexcept
{
    // If this node is a terminal, check its type.
    // If type doesn't match, there is no way forward.
    if (!node) return nullptr;
    const auto terminal = dynamic_cast<antlr4::tree::TerminalNode*>(node);
    if (terminal) {
        const auto symbol = terminal->getSymbol();
        return (symbol && symbol->getType() == type) ? node : nullptr;
    }

    // Search for the terminal recursively.
    for (const auto e : node->children) {
        const auto node1 = findTerminal(e, type);
        if (node1) return node1;
    }
    return nullptr;
}

std::size_t getNonTerminalType(antlr4::tree::ParseTree* node) noexcept
{
    const auto context = dynamic_cast<const antlr4::RuleContext*>(node);
    return context ? context->getRuleIndex() : kInvalidNodeType;
}

std::size_t getTerminalType(antlr4::tree::ParseTree* node) noexcept
{
    const auto terminal = dynamic_cast<antlr4::tree::TerminalNode*>(node);
    if (!terminal) return kInvalidNodeType;
    const auto symbol = terminal->getSymbol();
    return symbol ? symbol->getType() : kInvalidNodeType;
}

std::string getAnyNameText(antlr4::tree::ParseTree* node)
{
    const auto nodeTerminalType = helpers::getTerminalType(node->children.at(0));
    if (nodeTerminalType == SiodbParser::IDENTIFIER) return node->getText();

    if (nodeTerminalType == SiodbParser::STRING_LITERAL) {
        auto s = node->getText();
        // Remove quotes
        s.pop_back();
        s.erase(0, 1);
        return s;
    }

    const auto nodeNonTerminalType = helpers::getNonTerminalType(node->children.at(0));
    if (nodeNonTerminalType == SiodbParser::RuleKeyword) return node->getText();

    // '(' any_text ')' case
    if (node->children.size() == 3) return getAnyNameText(node->children[1]);

    // No match
    throw std::invalid_argument("AnyName node is invalid or not supported");
}

}  // namespace siodb::iomgr::dbengine::parser::helpers
