// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "antlr_wrappers/SiodbLexerWrapper.h"
#include "antlr_wrappers/SiodbParserWrapper.h"

// Common project headers
#include <siodb/common/utils/HelperMacros.h>

// ANTLR4 headers
#include <antlr4-runtime.h>

namespace siodb::iomgr::dbengine::parser {

/** SQL parser that generates IO Manager protocol messages */
class SqlParser : public antlr4::BaseErrorListener {
public:
    /**
     * Initializes object of class SqlParser.
     * @param inputString Input string.
     */
    explicit SqlParser(const std::string& inputString);

    DECLARE_NONCOPYABLE(SqlParser);

    /**
     * Returns parse tree.
     * @return Parse tree.
     */
    antlr4::tree::ParseTree* getParseTree() const noexcept
    {
        return m_parseTree;
    }

    /**
     * The human readable description of the last error.
     * @return The message that should be seen by the user about the last error.
     */
    const std::string& getErrorMessage() const noexcept
    {
        return m_errorMessage;
    }

    /**
     * Counts number of parsed SQL statements.
     * @return Number of parsed SQL statemens.
     */
    std::size_t getStatementCount() const noexcept;

    /**
     * Returns indication that a statement at a given index is of a desired type.
     * @param statementIndex Statement index.
     * @param statementType Desired statement type.
     * @return true if statement at a given index is of a desired type, false otherwise.
     */
    bool isStatement(std::size_t statementIndex, std::size_t statementType) const noexcept;

    /**
     * Finds the node that is the root node of the SQL statement.
     * @param statementIndex Statement index.
     * @return Tree node holding the statement or nullptr if statement with given
     *         index doesn't exist.
     */
    antlr4::tree::ParseTree* findStatement(const std::size_t statementIndex) const noexcept;

    /** Parses input string */
    void parse();

    /**
     * Dumps full parse tree to stdout.
     * @param flush Indicates that stream must be flushed after dumping.
     */
    void dump(bool flush = true) const;

    /**
     * Dumps full parse tree to a given output stream.
     * @param os Output stream.
     * @param flush Indicates that stream must be flushed after dumping.
     */
    void dump(std::ostream& os, bool flush = true) const
    {
        dump(m_parseTree, os, flush);
    }

    /**
     * Dumps the parse tree starting at a given node.
     * @param tree A parse tree node where the dump starts.
     * @param os Output stream.
     * @param flush Indicates that stream must be flushed after dumping.
     */
    void dump(antlr4::tree::ParseTree* tree, std::ostream& os, bool flush = true) const;

private:
    /**
     * The recursive part of the dump().
     * @param tree The node where the dumping happens.
     * @param index Index of the node in the upper level tree.
     * @param recursionLevel Node depth.
     * @param indentString Indent string for pretty printing.
     * @param isLast Flag which indicates that last child of the parent node is being dumped.
     * @param os Output stream.
     */
    void dump(antlr4::tree::ParseTree* tree, std::size_t index, std::size_t recursionLevel,
            const std::string indentString, bool isLast, std::ostream& os) const;

    /**
     * This method is called by ANTLR Runtime to report errors.
     * @see antlr4::BaseErrorListener::syntaxError()
     */
    void syntaxError(antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol,
            std::size_t line, std::size_t charPositionInLine, const std::string& msg,
            std::exception_ptr e) override;

private:
    /** Input string */
    const std::string& m_inputString;

    /** ANTLR input steam */
    antlr4::ANTLRInputStream m_inputStream;

    /** ANTLR generated lexer object */
    SiodbLexer m_sqliteLexer;

    /** ANTLR token stream object */
    antlr4::CommonTokenStream m_tokens;

    /** ANTLR generated parser object */
    SiodbParser m_siodbParser;

    /** Parse tree of an expression */
    SiodbParser::ParseContext* m_parseTree;

    /** Error message*/
    std::string m_errorMessage;
};

}  // namespace siodb::iomgr::dbengine::parser
