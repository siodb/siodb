// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "antlr_wrappers/Antlr4RuntimeWrapper.h"

namespace siodb::iomgr::dbengine::parser {

/** The value used to indicate that a tree node was not found. */
constexpr std::size_t kInvalidNodeType = std::numeric_limits<std::size_t>::max();

namespace helpers {

/**
 * Removes leading and trailing quotes from string.
 * NOTE: String must be quoted.
 * @param s A string.
 * @return Same string.
 */
inline std::string& unquoteString(std::string& s)
{
    s.pop_back();
    s.erase(0, 1);
    return s;
}

/**
 * Removes leading and trailing quotes from string.
 * NOTE: String must be quoted.
 * @param s A string.
 * @return Same string.
 */
inline std::string&& unquoteString(std::string&& s)
{
    s.pop_back();
    s.erase(0, 1);
    return std::move(s);
}

/**
 * The recursive part of the numberOfStatements() method.
 * @param tree The part of the tree to check.
 * @return A number of statements under this part of tree.
 */
std::size_t getStatementCount(const antlr4::tree::ParseTree* tree) noexcept;

/**
 * The recursive part of the method findStatement().
 * @param node Place where we are in the tree.
 * @param statementIndex The index of the statement we want to find.
 * @param nextIndex The index of the next statement if we find it.
 * @return The node that is the root of the statement or nullptr if statement is not
 *         found.
 */
antlr4::tree::ParseTree* findStatement(const antlr4::tree::ParseTree* node,
        const std::size_t statementIndex, std::size_t& nextIndex) noexcept;

/**
 * Finds the first terminal from a given point of the tree.
 * @param node Place where we start the search.
 * @param type Type of the terminal.
 * @return Node of the terminal, if found, nullptr otherwise.
 */
antlr4::tree::ParseTree* findTerminal(antlr4::tree::ParseTree* node, std::size_t type) noexcept;

/**
 * Finds the first non-terminal from a given point of the tree.
 * @param node Place where we start the search.
 * @param type Type of the non-terminal.
 * @return Node of the non-terminal if found, nullptr otherwise.
 */
antlr4::tree::ParseTree* findNonTerminal(antlr4::tree::ParseTree* node, std::size_t type) noexcept;

/**
 * Finds the first terminal from a given point of the tree
 * under a non-terminal node of a given type.
 * @param node Place where we start the search.
 * @param nonTerminalType Type of the upper-level non-terminal.
 * @param type Type of the terminal as it is defined in the SQLiteParser class.
 * @return Node of the terminal, if found, nullptr otherwise.
 */
antlr4::tree::ParseTree* findTerminal(
        antlr4::tree::ParseTree* node, size_t nonTerminalType, size_t terminalType) noexcept;

/**
 * Returns the type of a terminal.
 * @param node The node to check.
 * @return Type of the terminal if node is terminal, kInvalidNodeType otherwise.
 */
std::size_t getTerminalType(antlr4::tree::ParseTree* node) noexcept;

/**
 * Returns the type of a non-terminal.
 * @param node The node to check.
 * @return Type of the non-terminal if node is non-terminal, kInvalidNodeType otherwise.
 */
std::size_t getNonTerminalType(antlr4::tree::ParseTree* node) noexcept;

/**
 * Returns any_name node text value.
 * Any text value could be:
 * - identifier;
 * - SQL keyword;
 * - string literal
 * - '(' any_name ')'
 * @param node Node
 * @throw std::invalid_argument in case of invalid or not supported node.
 */
std::string getAnyNameText(antlr4::tree::ParseTree* node);

}  // namespace helpers

}  // namespace siodb::iomgr::dbengine::parser
