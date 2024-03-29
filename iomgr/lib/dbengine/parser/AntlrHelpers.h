// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
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
 * Converts inplace two single quote into one single quote, as per SQL string literal syntax.
 * NOTE: in SQL, a single quote is escaped with a single quote.
 * @param s A string.
 * @return Same string.
 */
std::string& fixSingleQuotes(std::string& s);

/**
 * Removes leading and trailing quotes and fixes single quotes as per SQL string literal syntax.
 * NOTE: String must be quoted.
 * @param s A string.
 * @return Same string.
 */
inline std::string& unquoteString(std::string& s)
{
    s.pop_back();
    s.erase(0, 1);
    return fixSingleQuotes(s);
}

/**
 * Removes leading and trailing quotes and fixes single quotes as per SQL string literal syntax.
 * NOTE: String must be quoted.
 * @param s A string.
 * @return Same string.
 */
inline std::string&& unquoteString(std::string&& s)
{
    s.pop_back();
    s.erase(0, 1);
    return std::move(fixSingleQuotes(s));
}

/**
 * Extracts object name from a given node.
 * @param node A node.
 * @return Capitalized object name.
 */
std::string extractObjectName(antlr4::tree::ParseTree* node);

/**
 * Extracts object name from a child node.
 * @param node A node.
 * @param childNodeIndex Index of a child node containing name.
 * @return Capitalized object name.
 */
std::string extractObjectName(const antlr4::tree::ParseTree* node, std::size_t childNodeIndex);

/**
 * The recursive part of the numberOfStatements() method.
 * @param tree The part of the tree to check.
 * @return A number of statements under this part of tree.
 */
std::size_t getStatementCount(const antlr4::tree::ParseTree* tree) noexcept;

/**
 * The recursive part of the method findStatement().
 * @param node Current node.
 * @param statementIndex The index of the statement we want to find.
 * @param nextIndex The index of the next statement if we find it.
 * @return The node that is the root of the statement or nullptr if statement is not
 *         found.
 */
antlr4::tree::ParseTree* findStatement(const antlr4::tree::ParseTree* node,
        const std::size_t statementIndex, std::size_t& nextIndex) noexcept;

/**
 * Finds the first terminal of a given type from a given point of the tree.
 * @param node Starting node.
 * @param type Type of the terminal.
 * @return Node of the terminal, if found, nullptr otherwise.
 */
antlr4::tree::TerminalNode* findTerminal(antlr4::tree::ParseTree* node, std::size_t type) noexcept;

/**
 * Finds the first terminal of one of a given types from a given point of the tree.
 * @param node Starting node.
 * @param types Types of the terminal.
 * @param n Number of terminal types.
 * @return Node of the terminal, if found, nullptr otherwise.
 */
antlr4::tree::TerminalNode* findTerminal(
        antlr4::tree::ParseTree* node, const std::size_t* types, std::size_t n) noexcept;

/**
 * Finds the first terminal child node from a given point of the tree.
 * @param node Starting node.
 * @param type Type of the terminal.
 * @return Terminal child index or std::numeric_limits<std::size_t>::max() if not found
 */
std::size_t findTerminalChild(antlr4::tree::ParseTree* node, std::size_t type) noexcept;

/**
 * Returns indication of existence of a terminal child node from a given point of the tree.
 * @param node Starting node.
 * @param type Type of the terminal.
 * @param startIndex Search start index.
 * @return true if terminal child of a given type exists, false otherwise.
 */
bool hasTerminalChild(
        antlr4::tree::ParseTree* node, std::size_t type, std::size_t startIndex = 0) noexcept;

/**
 * Captures terminal position.
 * @param terminal Terminal node.
 * @param[out] line Terminal line.
 * @param[out] column Terminal column.
 * @return true if position captured, false otherwise.
 */
bool captureTerminalPosition(
        antlr4::tree::TerminalNode* node, std::size_t& line, std::size_t& column) noexcept;

/**
 * Finds first terminal and gets its position.
 * @param node A node.
 * @param type Terminal type.
 * @param[out] line Terminal line.
 * @param[out] column Terminal column.
 * @return true if terminal found and position captured, false otherwise.
 */
bool findFirstTerminalAndCapturePosition(antlr4::tree::ParseTree* node, std::size_t type,
        std::size_t& line, std::size_t& column) noexcept;

/**
 * Finds the first non-terminal of a given type from a given point of the tree.
 * @param node Starting node.
 * @param type Type of the non-terminal.
 * @return Node of the non-terminal if found, nullptr otherwise.
 */
antlr4::tree::ParseTree* findNonTerminal(antlr4::tree::ParseTree* node, std::size_t type) noexcept;

/**
 * Finds the first non-terminal child node of a given type from a given node.
 * @param node A node.
 * @param type Type of the non-terminal.
 * @return Node of the non-terminal if found, nullptr otherwise.
 */
antlr4::tree::ParseTree* findNonTerminalChild(
        antlr4::tree::ParseTree* node, std::size_t type) noexcept;

/**
 * Finds the first terminal from a given point of the tree
 * under a non-terminal node of a given type.
 * @param node Starting node.
 * @param nonTerminalType Type of the upper-level non-terminal.
 * @param type Type of the terminal as it is defined in the SQLiteParser class.
 * @return Node of the terminal, if found, nullptr otherwise.
 */
antlr4::tree::TerminalNode* findTerminal(
        antlr4::tree::ParseTree* node, size_t nonTerminalType, size_t terminalType) noexcept;

/**
 * Returns the type of a maybe terminal node.
 * @param node The node to check.
 * @return Type of the terminal if node is terminal, kInvalidNodeType otherwise.
 */
std::size_t getMaybeTerminalType(antlr4::tree::ParseTree* node) noexcept;

/**
 * Returns the type of a terminal node.
 * @param terminal A terminal node to check.
 * @return Type of the terminal if node is terminal, kInvalidNodeType otherwise.
 */
std::size_t getTerminalType(antlr4::tree::TerminalNode* terminal) noexcept;

/**
 * Returns the type of a non-terminal.
 * @param node The node to check.
 * @return Type of the non-terminal if node is non-terminal, kInvalidNodeType otherwise.
 */
std::size_t getNonTerminalType(antlr4::tree::ParseTree* node) noexcept;

/**
 * Returns any_name node text.
 * any_name node can be:
 * - identifier
 * - SQL keyword
 * - attribute name
 * - string literal
 * - '(' any_name ')'
 * @param node A node.
 * @throw std::invalid_argument if node is invalid or unsupported.
 */
std::string getAnyNameText(antlr4::tree::ParseTree* node);

}  // namespace helpers

}  // namespace siodb::iomgr::dbengine::parser
