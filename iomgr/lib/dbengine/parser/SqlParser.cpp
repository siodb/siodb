// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SqlParser.h"

// Project headers
#include "AntlrHelpers.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/utils/EmptyString.h>

namespace siodb::iomgr::dbengine::parser {

SqlParser::SqlParser(const std::string& inputString)
    : m_inputString(inputString)
    , m_inputStream(m_inputString)
    , m_sqliteLexer(&m_inputStream)
    , m_tokens(&m_sqliteLexer)
    , m_siodbParser(&m_tokens)
    , m_parseTree(nullptr)
{
    m_siodbParser.addErrorListener(this);
    m_siodbParser.setBuildParseTree(true);
}

std::size_t SqlParser::getStatementCount() const noexcept
{
    return helpers::getStatementCount(m_parseTree);
}

bool SqlParser::isStatement(std::size_t statementIndex, std::size_t statementType) const noexcept
{
    const auto tree = findStatement(statementIndex);
    return tree ? helpers::getNonTerminalType(tree) == statementType : false;
}

antlr4::tree::ParseTree* SqlParser::findStatement(const std::size_t statementIndex) const noexcept
{
    std::size_t nextIndex = 0;
    return helpers::findStatement(m_parseTree, statementIndex, nextIndex);
}

void SqlParser::parse()
{
    m_tokens.fill();
    m_parseTree = m_siodbParser.parse();
}

void SqlParser::dump(bool flush) const
{
    dump(std::cout, flush);
}

void SqlParser::dump(antlr4::tree::ParseTree* tree, std::ostream& os, bool flush) const
{
    dump(tree, 0, 0, "", false, os);
    if (flush) os << std::flush;
}

void SqlParser::dump(antlr4::tree::ParseTree* tree, std::size_t index, std::size_t recursionLevel,
        const std::string indentString, bool isLast, std::ostream& os) const
{
    if (tree == nullptr) return;
    const auto nodeString = isLast ? "└── " : "├── ";
    os << indentString << nodeString;

    // If this is a non-terminal we print its name and ID.
    const auto context = dynamic_cast<antlr4::RuleContext*>(tree);
    if (context) {
        const auto& names = m_siodbParser.getRuleNames();
        const auto ruleIndex = context->getRuleIndex();
        const auto& name = (ruleIndex < names.size()) ? names[ruleIndex] : utils::g_emptyString;
        os << '[' << index << "] " << name << " (NT " << ruleIndex << ')';
    } else {
        // If this is a terminal we print the name, ID and value.
        const auto terminal = dynamic_cast<antlr4::tree::TerminalNode*>(tree);
        const auto token = terminal ? terminal->getSymbol() : nullptr;
        if (token) {
            const auto type = token->getType();
            os << '[' << index << "] " << m_siodbParser.getVocabulary().getSymbolicName(type)
               << " (T " << type << ") " << token->getText();
        }
    }

    os << '\n';

    for (std::size_t i = 0; i < tree->children.size(); ++i) {
        const bool last = (i == tree->children.size() - 1);
        const auto newIndentString =
                (recursionLevel > 0) ? indentString + "    " : indentString + "│   ";
        dump(tree->children[i], i, recursionLevel + 1, newIndentString, last, os);
    }
}

std::string SqlParser::injectError(std::size_t line, std::size_t column, const std::string& msg)
{
    std::string s;
    {
        std::ostringstream err;
        err << "at (" << line << ", " << column << "): " << msg;
        s = err.str();
    }
    m_errorMessage = s;
    return s;
}

// --- internals ----

void SqlParser::syntaxError([[maybe_unused]] antlr4::Recognizer* recognizer,
        [[maybe_unused]] antlr4::Token* offendingSymbol, [[maybe_unused]] std::size_t line,
        [[maybe_unused]] std::size_t charPositionInLine, [[maybe_unused]] const std::string& msg,
        [[maybe_unused]] std::exception_ptr e)
{
    throw std::runtime_error(injectError(line, charPositionInLine, msg));
}

}  // namespace siodb::iomgr::dbengine::parser
