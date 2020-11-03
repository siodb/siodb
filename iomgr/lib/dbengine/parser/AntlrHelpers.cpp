// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "AntlrHelpers.h"

// Project headers
#include "antlr_wrappers/SiodbParserWrapper.h"

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>

namespace siodb::iomgr::dbengine::parser::helpers {

std::string extractObjectName(const antlr4::tree::ParseTree* node, std::size_t childNodeIndex)
{
    auto name = node->children.at(childNodeIndex)->getText();
    boost::to_upper(name);
    return name;
}

std::size_t getStatementCount(const antlr4::tree::ParseTree* tree) noexcept
{
    const auto context = dynamic_cast<const antlr4::RuleContext*>(tree);
    if (!context) return 0;

    if (context->getRuleIndex() == SiodbParser::RuleSql_stmt) return 1;

    std::size_t result = 0;
    for (const auto childNode : tree->children)
        result += getStatementCount(childNode);
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
    for (const auto childNode : node->children) {
        const auto node1 = findStatement(childNode, statementIndex, nextIndex);
        if (node1) return node1;
    }
    return nullptr;
}

antlr4::tree::ParseTree* findNonTerminal(antlr4::tree::ParseTree* node, const size_t type) noexcept
{
    const auto context = dynamic_cast<antlr4::RuleContext*>(node);
    if (!context) return nullptr;

    if (context->getRuleIndex() == type) return node;

    for (const auto childNode : node->children) {
        const auto node1 = findNonTerminal(childNode, type);
        if (node1) return node1;
    }
    return nullptr;
}

antlr4::tree::ParseTree* findNonTerminalChild(
        antlr4::tree::ParseTree* node, const size_t type) noexcept
{
    for (const auto childNode : node->children) {
        const auto context = dynamic_cast<antlr4::RuleContext*>(childNode);
        if (context && context->getRuleIndex() == type) return childNode;
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
        return (symbol && (type == 0 || symbol->getType() == type)) ? node : nullptr;
    }

    // Search for the terminal recursively.
    for (const auto childNode : node->children) {
        const auto node1 = findTerminal(childNode, type);
        if (node1) return node1;
    }
    return nullptr;
}

std::size_t findTerminalChild(antlr4::tree::ParseTree* node, std::size_t type) noexcept
{
    std::size_t index = 0;
    for (const auto childNode : node->children) {
        const auto terminal = dynamic_cast<antlr4::tree::TerminalNode*>(childNode);
        if (terminal) {
            const auto symbol = terminal->getSymbol();
            if (symbol && (type == 0 || symbol->getType() == type)) return index;
        }
        ++index;
    }
    return std::numeric_limits<std::size_t>::max();
}

bool hasTerminalChild(
        antlr4::tree::ParseTree* node, std::size_t type, std::size_t startIndex) noexcept
{
    if (startIndex < node->children.size()) {
        for (auto it = node->children.begin() + startIndex; it != node->children.end(); ++it) {
            const auto terminal = dynamic_cast<antlr4::tree::TerminalNode*>(*it);
            if (terminal) {
                const auto symbol = terminal->getSymbol();
                if (symbol && symbol->getType() == type) return true;
            }
        }
    }
    return false;
}

bool captureTerminalPosition(
        antlr4::tree::ParseTree* node, std::size_t& line, std::size_t& column) noexcept
{
    const auto terminal = dynamic_cast<antlr4::tree::TerminalNode*>(node);
    if (terminal) {
        const auto symbol = terminal->getSymbol();
        if (symbol) {
            line = symbol->getLine();
            column = symbol->getCharPositionInLine() + 1;
            return true;
        }
    }
    return false;
}

bool findFirstTerminalAndCapturePosition(antlr4::tree::ParseTree* node, std::size_t type,
        std::size_t& line, std::size_t& column) noexcept
{
    const auto terminal = findTerminal(node, type);
    return terminal ? captureTerminalPosition(terminal, line, column) : false;
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
    const auto firstChild = node->children.at(0);
    switch (helpers::getTerminalType(firstChild)) {
        case SiodbParser::IDENTIFIER: return node->getText();
        case SiodbParser::STRING_LITERAL: return unquoteString(node->getText());
        default: break;
    }

    switch (helpers::getNonTerminalType(firstChild)) {
        case SiodbParser::RuleAttribute:
        case SiodbParser::RuleKeyword: return node->getText();
        default: break;
    }

    // '(' any_text ')' case
    if (node->children.size() == 3) return getAnyNameText(node->children[1]);

    // No match
    throw std::invalid_argument("any_name node is invalid or unsupported");
}

}  // namespace siodb::iomgr::dbengine::parser::helpers
