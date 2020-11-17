// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DBEngineSqlRequestFactory.h"

// Project headers
#include "AntlrHelpers.h"
#include "DBEngineRequestFactoryError.h"
#include "antlr_wrappers/SiodbParserWrapper.h"
#include "expr/AllColumnsExpression.h"
#include "expr/ConstantExpression.h"
#include "expr/ExpressionFactory.h"
#include "expr/SingleColumnExpression.h"

// Common project headers
#include <siodb/common/config/SiodbDataFileDefs.h>
#include <siodb/common/log/Log.h>
#include <siodb/iomgr/shared/dbengine/SystemObjectNames.h>

// Boost headers
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/uuid/string_generator.hpp>

// Protobuf message headers
#include <siodb/common/proto/CommonMessages.pb.h>

namespace siodb::iomgr::dbengine::parser {

namespace {

/**
 * Parses active/inactive state.
 * @param node A node to parse from.
 * @param errorMessage Error message to use if parse error occurs.
 * @return true for the ACTIVE state, false for the INACTIVE state.
 * @throw DBEngineRequestFactoryError if state could not be parsed.
 */
bool parseState(antlr4::tree::ParseTree* node, const char* errorMessage)
{
    switch (helpers::getTerminalType(node)) {
        case SiodbParser::K_ACTIVE: return true;
        case SiodbParser::K_INACTIVE: return false;
        default: throw DBEngineRequestFactoryError(errorMessage);
    }
}

/**
 * Parses string as expiration timestamp.
 * @param s A string to parse.
 * @return Expiration timestamp as epoch seconds.
 * @throw DBEngineRequestFactoryError if parsing failed.
 */
time_t parseExpirationTimestamp(const std::string& s)
{
    const Variant t(s, Variant::AsDateTime());
    return t.getDateTime().toEpochTimestamp();
}

}  // namespace

const std::unordered_map<std::string, siodb::ColumnDataType>
        DBEngineSqlRequestFactory::m_siodbDataTypeMap {
                {"BOOLEAN", siodb::COLUMN_DATA_TYPE_BOOL},
                {"INTEGER", siodb::COLUMN_DATA_TYPE_INT32},
                {"INT", siodb::COLUMN_DATA_TYPE_INT32},
                {"INT32", siodb::COLUMN_DATA_TYPE_INT32},
                {"UINT", siodb::COLUMN_DATA_TYPE_UINT32},
                {"UINT32", siodb::COLUMN_DATA_TYPE_UINT32},
                {"INT8", siodb::COLUMN_DATA_TYPE_INT8},
                {"TINYINT", siodb::COLUMN_DATA_TYPE_INT8},
                {"UINT8", siodb::COLUMN_DATA_TYPE_UINT8},
                {"TINYUINT", siodb::COLUMN_DATA_TYPE_UINT8},
                {"INT16", siodb::COLUMN_DATA_TYPE_INT16},
                {"SMALLINT", siodb::COLUMN_DATA_TYPE_INT16},
                {"UINT16", siodb::COLUMN_DATA_TYPE_UINT16},
                {"SMALLUINT", siodb::COLUMN_DATA_TYPE_UINT16},
                {"INT64", siodb::COLUMN_DATA_TYPE_INT64},
                {"BIGINT", siodb::COLUMN_DATA_TYPE_INT64},
                {"UINT64", siodb::COLUMN_DATA_TYPE_UINT64},
                {"BIGUINT", siodb::COLUMN_DATA_TYPE_UINT64},
                {"SMALLREAL", siodb::COLUMN_DATA_TYPE_FLOAT},
                {"REAL", siodb::COLUMN_DATA_TYPE_DOUBLE},
                {"FLOAT", siodb::COLUMN_DATA_TYPE_FLOAT},
                {"DOUBLE", siodb::COLUMN_DATA_TYPE_DOUBLE},
                {"TEXT", siodb::COLUMN_DATA_TYPE_TEXT},
                {"NTEXT", siodb::COLUMN_DATA_TYPE_TEXT},
                {"CHAR", siodb::COLUMN_DATA_TYPE_TEXT},
                {"VARCHAR", siodb::COLUMN_DATA_TYPE_TEXT},
                {"BLOB", siodb::COLUMN_DATA_TYPE_BINARY},
                {"BINARY", siodb::COLUMN_DATA_TYPE_BINARY},
                {"TIMESTAMP", siodb::COLUMN_DATA_TYPE_TIMESTAMP},
                {"XML", siodb::COLUMN_DATA_TYPE_XML},
                {"JSON", siodb::COLUMN_DATA_TYPE_JSON},
        };

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSqlRequest(std::size_t index)
{
    return createSqlRequest(m_parser.findStatement(index));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSqlRequest(
        antlr4::tree::ParseTree* node)
{
    if (!node) throw DBEngineRequestFactoryError("Statement doesn't exist");

    const auto statementType = helpers::getNonTerminalType(node);
    switch (statementType) {
        case SiodbParser::RuleSelect_stmt:
            return createSelectRequestForGeneralSelectStatement(node);
        case SiodbParser::RuleSimple_select_stmt:
            return createSelectRequestForSimpleSelectStatement(node);
        case SiodbParser::RuleFactored_select_stmt:
            return createSelectRequestForFactoredSelectStatement(node);
        case SiodbParser::RuleShow_databases_stmt:
            return std::make_unique<requests::ShowDatabasesRequest>();
        case SiodbParser::RuleShow_tables_stmt:
            return std::make_unique<requests::ShowTablesRequest>();
        case SiodbParser::RuleInsert_stmt: return createInsertRequest(node);
        case SiodbParser::RuleUpdate_stmt: return createUpdateRequest(node);
        case SiodbParser::RuleDelete_stmt: return createDeleteRequest(node);
        case SiodbParser::RuleBegin_stmt: return createBeginTransactionRequest(node);
        case SiodbParser::RuleCommit_stmt: return createCommitTransactionRequest(node);
        case SiodbParser::RuleRollback_stmt: return createRollbackTransactionRequest(node);
        case SiodbParser::RuleSavepoint_stmt: return createSavepointRequest(node);
        case SiodbParser::RuleRelease_stmt: return createReleaseRequest(node);
        case SiodbParser::RuleAttach_stmt: return createAttachDatabaseRequest(node);
        case SiodbParser::RuleDetach_stmt: return createDetachDatabaseRequest(node);
        case SiodbParser::RuleCreate_database_stmt: return createCreateDatabaseRequest(node);
        case SiodbParser::RuleDrop_database_stmt: return createDropDatabaseRequest(node);
        case SiodbParser::RuleAlter_database_stmt: {
            auto keyword = helpers::findTerminal(node, SiodbParser::K_RENAME);
            if (keyword) return createRenameDatabaseRequest(node);

            keyword = helpers::findTerminal(node, SiodbParser::K_SET);
            if (keyword) return createSetDatabaseAttributesRequest(node);

            throw DBEngineRequestFactoryError("ALTER DATABASE: unsupported operation");
        }
        case SiodbParser::RuleUse_database_stmt: return createUseDatabaseRequest(node);
        case SiodbParser::RuleCreate_table_stmt: return createCreateTableRequest(node);
        case SiodbParser::RuleDrop_table_stmt: return createDropTableRequest(node);
        case SiodbParser::RuleAlter_table_stmt: {
            if (helpers::hasTerminalChild(node, SiodbParser::K_RENAME)) {
                return helpers::hasTerminalChild(node, SiodbParser::K_COLUMN)
                               ? createRenameColumnRequest(node)
                               : createRenameTableRequest(node);
            }
            if (helpers::hasTerminalChild(node, SiodbParser::K_ADD))
                return createAddColumnRequest(node);
            if (helpers::hasTerminalChild(node, SiodbParser::K_DROP))
                return createDropColumnRequest(node);
            if (helpers::hasTerminalChild(node, SiodbParser::K_SET))
                return createSetTableAttributesRequest(node);
            if (helpers::hasTerminalChild(node, SiodbParser::K_ALTER, 1))
                return createRedefineColumnRequest(node);
            throw DBEngineRequestFactoryError("ALTER TABLE: unsupported operation");
        }
        case SiodbParser::RuleCreate_index_stmt: return createCreateIndexRequest(node);
        case SiodbParser::RuleDrop_index_stmt: return createDropIndexRequest(node);
        case SiodbParser::RuleCreate_user_stmt: return createCreateUserRequest(node);
        case SiodbParser::RuleDrop_user_stmt: return createDropUserRequest(node);
        case SiodbParser::RuleAlter_user_stmt: {
            const auto operationType = helpers::getTerminalType(node->children.at(3));
            switch (operationType) {
                case SiodbParser::K_ADD: {
                    const auto objectType = helpers::getTerminalType(node->children.at(4));
                    switch (objectType) {
                        case SiodbParser::K_ACCESS: return createAddUserAccessKeyRequest(node);
                        case SiodbParser::K_TOKEN: return createAddUserTokenRequest(node);
                        default:
                            throw DBEngineRequestFactoryError(
                                    "ALTER USER ADD: unsupported object type");
                    }
                }
                case SiodbParser::K_DROP: {
                    const auto objectType = helpers::getTerminalType(node->children.at(4));
                    switch (objectType) {
                        case SiodbParser::K_ACCESS: return createDropUserAccessKeyRequest(node);
                        case SiodbParser::K_TOKEN: return createDropUserTokenRequest(node);
                        default:
                            throw DBEngineRequestFactoryError(
                                    "ALTER USER DROP: unsupported object type");
                    }
                }
                case SiodbParser::K_ALTER: {
                    const auto objectType = helpers::getTerminalType(node->children.at(4));
                    switch (objectType) {
                        case SiodbParser::K_ACCESS: {
                            const auto actionType = helpers::getTerminalType(node->children.at(7));
                            switch (actionType) {
                                case SiodbParser::K_SET: {
                                    return createSetUserAccessKeyAttributesRequest(node);
                                }
                                case SiodbParser::K_RENAME: {
                                    return createRenameUserAccessKeyRequest(node);
                                }
                                default: {
                                    throw DBEngineRequestFactoryError(
                                            "ALTER USER ALTER ACCESS KEY: unsupported operation");
                                }
                            }
                        }
                        case SiodbParser::K_TOKEN: {
                            const auto actionType = helpers::getTerminalType(node->children.at(6));
                            switch (actionType) {
                                case SiodbParser::K_SET:
                                    return createSetUserTokenAttributesRequest(node);
                                case SiodbParser::K_RENAME: {
                                    return createRenameUserTokenRequest(node);
                                }
                                default: {
                                    throw DBEngineRequestFactoryError(
                                            "ALTER USER ALTER TOKEN: unsupported operation");
                                }
                            }
                        }
                        default:
                            throw DBEngineRequestFactoryError(
                                    "ALTER USER ALTER: unsupported object type");
                    }
                }
                case SiodbParser::K_SET: return createSetUserAttributesRequest(node);
                default: throw DBEngineRequestFactoryError("ALTER USER: unsupported operation");
            }
        }
        case SiodbParser::RuleCheck_user_token_stmt: return createCheckUserTokenRequest(node);
        default: {
            throw DBEngineRequestFactoryError(
                    "Unsupported statement type " + std::to_string(statementType));
        }
    }
}

// ----- internals -----

requests::DBEngineRequestPtr
DBEngineSqlRequestFactory::createSelectRequestForGeneralSelectStatement(
        [[maybe_unused]] antlr4::tree::ParseTree* node)
{
    throw DBEngineRequestFactoryError("SELECT: unsupported syntax");

    // TODO: Implement DBEngineSqlRequestFactory::createSelectRequestForGeneralSelectStatement()

    // TODO: Capture WHERE values
    // TODO: Capture GROUP BY values
    // TODO: Capture HAVING values
    // TODO: Capture ORDER BY values
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSelectRequestForSimpleSelectStatement(
        antlr4::tree::ParseTree* node)
{
    ExpressionFactory exprFactory(m_parser);
    std::string database;
    std::vector<requests::SourceTable> tables;
    std::vector<requests::ResultExpression> columns;
    requests::ConstExpressionPtr where, offset, limit;

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        const auto child = node->children[i];
        const auto childTerminalType = helpers::getNonTerminalType(child);

        if (childTerminalType == SiodbParser::RuleSelect_core)
            parseSelectCore(child, database, tables, columns, where);
        else if (childTerminalType == kInvalidNodeType) {
            const auto terminalType = helpers::getTerminalType(child);
            switch (terminalType) {
                case SiodbParser::K_LIMIT: {
                    ++i;
                    if (i >= node->children.size())
                        throw DBEngineRequestFactoryError(
                                "SELECT: LIMIT does not contain expression");

                    if (node->children.size() > i + 2
                            && helpers::getTerminalType(node->children[i + 1])
                                       == SiodbParser::COMMA) {
                        // '... LIMIT <OFFSET> , <LIMIT> ...' case
                        offset = exprFactory.createExpression(node->children[i]);
                        limit = exprFactory.createExpression(node->children[i + 2]);
                        i += 2;  // skip ',' and '<LIMIT>'
                    } else {
                        // Simple LIMIT case
                        limit = exprFactory.createExpression(node->children[i]);
                    }

                    break;
                }
                case SiodbParser::K_OFFSET: {
                    ++i;
                    if (i >= node->children.size())
                        throw DBEngineRequestFactoryError(
                                "SELECT: OFFSET does not contain expression");
                    offset = exprFactory.createExpression(node->children[i]);
                    break;
                }
                default: break;
            }
        }
    }

    // TODO: Capture GROUP BY values
    std::vector<requests::ConstExpressionPtr> groupBy;

    // TODO: Capture HAVING values
    requests::ConstExpressionPtr having;

    // TODO: Capture ORDER BY values
    std::vector<requests::ConstExpressionPtr> orderBy;

    return std::make_unique<requests::SelectRequest>(std::move(database), std::move(tables),
            std::move(columns), std::move(where), std::move(groupBy), std::move(having),
            std::move(orderBy), std::move(offset), std::move(limit));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSelectRequestForShowTablesStatement()
{
    std::string database;
    requests::ConstExpressionPtr where, offset, limit;
    requests::ConstExpressionPtr having;
    std::vector<requests::ConstExpressionPtr> groupBy;
    std::vector<requests::ConstExpressionPtr> orderBy;

    std::vector<requests::SourceTable> tables;
    tables.emplace_back(kSysTablesTableName, "");

    std::vector<requests::ResultExpression> columns;
    columns.push_back(
            requests::ResultExpression(std::make_unique<requests::SingleColumnExpression>(
                                               kSysTablesTableName, kSysTables_Name_ColumnName),
                    ""));
    columns.push_back(requests::ResultExpression(
            std::make_unique<requests::SingleColumnExpression>(
                    kSysTablesTableName, kSysTables_Description_ColumnName),
            ""));

    return std::make_unique<requests::SelectRequest>(std::move(database), std::move(tables),
            std::move(columns), std::move(where), std::move(groupBy), std::move(having),
            std::move(orderBy), std::move(offset), std::move(limit));
}

requests::DBEngineRequestPtr
DBEngineSqlRequestFactory::createSelectRequestForFactoredSelectStatement(
        antlr4::tree::ParseTree* node)
{
    // TODO: Implement DBEngineSqlRequestFactory::createSelectRequestForFactoredSelectStatement()
    const auto selectCoreCount = std::count_if(
            node->children.cbegin(), node->children.cend(), [](const auto e) noexcept {
                return helpers::getNonTerminalType(e) == SiodbParser::RuleSelect_core;
            });

    if (selectCoreCount != 1) throw DBEngineRequestFactoryError("SELECT contains too much parts");

    // Now fallback to simple one
    return createSelectRequestForSimpleSelectStatement(node);

    // TODO: Capture WHERE values
    // TODO: Capture GROUP BY values
    // TODO: Capture HAVING values
    // TODO: Capture ORDER BY values
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createInsertRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture table ID
    std::string table;
    const auto tableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode) {
        table = tableIdNode->getText();
        boost::to_upper(table);
    } else
        throw DBEngineRequestFactoryError("INSERT: missing table ID");

    // Capture column IDs
    std::vector<std::string> columns;
    bool valuesFound = false;
    std::size_t index = 0;
    while (index < node->children.size()) {
        const auto e = node->children[index++];
        if (helpers::getNonTerminalType(e) != SiodbParser::RuleColumn_name) {
            if (helpers::getTerminalType(e) == SiodbParser::K_VALUES) {
                valuesFound = true;
                break;
            } else
                continue;
        }

        const auto columnIdNode = helpers::findTerminal(e, SiodbParser::IDENTIFIER);
        if (!columnIdNode) throw DBEngineRequestFactoryError("INSERT missing column ID");

        auto columnId = columnIdNode->getText();
        boost::to_upper(columnId);
        columns.push_back(std::move(columnId));
    }

    if (!valuesFound) throw DBEngineRequestFactoryError("INSERT missing VALUES keyword");

    ExpressionFactory exprFactory(m_parser);
    std::vector<std::vector<requests::ConstExpressionPtr>> values;
    if (!columns.empty()) values.reserve(columns.size());
    bool inValueGroup = false;
    while (index < node->children.size()) {
        const auto e = node->children[index++];
        // Handle opening and closing value group
        const auto terminalType = helpers::getTerminalType(e);
        if (terminalType == SiodbParser::OPEN_PAR) {
            if (inValueGroup)
                throw DBEngineRequestFactoryError("INSERT: unexpected opening parenthesis");
            inValueGroup = true;
            values.emplace_back();
            continue;
        } else if (terminalType == SiodbParser::CLOSE_PAR) {
            if (!inValueGroup)
                throw DBEngineRequestFactoryError("INSERT: unexpected closing parenthesis");
            inValueGroup = false;
            if (!columns.empty() && values.back().size() != columns.size()) {
                throw DBEngineRequestFactoryError(
                        "INSERT: number of values doesn't match to number of columns");
            }
            continue;
        }
        if (helpers::getNonTerminalType(e) != SiodbParser::RuleExpr) continue;
        values.back().push_back(exprFactory.createExpression(e));
    }

    if (inValueGroup) throw DBEngineRequestFactoryError("INSERT: values list is not closed");

    if (values.empty()) throw DBEngineRequestFactoryError("INSERT: missing values");

    return std::make_unique<requests::InsertRequest>(
            std::move(database), std::move(table), std::move(columns), std::move(values));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createUpdateRequest(
        antlr4::tree::ParseTree* node)
{
    ExpressionFactory exprFactory(m_parser, true);
    std::string database, table, tableAlias;
    requests::ConstExpressionPtr where;
    std::vector<requests::ColumnReference> columns;
    std::vector<requests::ConstExpressionPtr> values;

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        const auto e = node->children[i];
        const auto nonTerminalType = helpers::getNonTerminalType(e);
        switch (nonTerminalType) {
            case SiodbParser::RuleAliased_qualified_table_name: {
                // Capture Database name
                const auto databaseNameNode = helpers::findTerminal(
                        e, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
                if (databaseNameNode) {
                    database = databaseNameNode->getText();
                    boost::to_upper(database);
                }

                // Capture table name
                const auto tableNameNode = helpers::findTerminal(
                        e, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
                if (tableNameNode) {
                    table = tableNameNode->getText();
                    boost::to_upper(table);
                } else
                    throw DBEngineRequestFactoryError("UPDATE: missing table ID");

                // Capture table alias
                const auto tableAliasNode = helpers::findTerminal(
                        e, SiodbParser::RuleTable_alias, SiodbParser::IDENTIFIER);
                if (tableAliasNode) {
                    tableAlias = tableAliasNode->getText();
                    boost::to_upper(tableAlias);
                }

                break;
            }
            case kInvalidNodeType: {
                auto terminalType = helpers::getTerminalType(e);
                if (terminalType == SiodbParser::K_SET) {
                    ++i;
                    std::size_t updateValueCount = 0;

                    while (i < node->children.size()) {
                        if (updateValueCount > 0 && i + 2 == node->children.size()) {
                            // if only 2 nodes remain the only possible variant is
                            // WHERE + expr

                            const auto whereNode = node->children[i];
                            terminalType = helpers::getTerminalType(whereNode);
                            if (terminalType == SiodbParser::K_WHERE) {
                                ++i;
                                if (i >= node->children.size()) {
                                    throw DBEngineRequestFactoryError(
                                            "UPDATE: WHERE clause does not contain expression");
                                }
                                where = exprFactory.createExpression(node->children[i]);
                            } else
                                throw DBEngineRequestFactoryError("UPDATE: Invalid SET statement");

                            break;
                        }

                        // SET statement should have at least one column = expr
                        if (i + 2 >= node->children.size())
                            throw DBEngineRequestFactoryError("UPDATE: missing expression in SET");

                        /// --------- Parse column/where ---------
                        const auto columnNode = node->children[i];

                        auto nonTerminalType = helpers::getNonTerminalType(columnNode);
                        if (nonTerminalType == SiodbParser::RuleColumn_name) {
                            std::string column;
                            if (columnNode->children.size() == 1) {
                                // Only column name is expected
                                column = helpers::extractObjectName(columnNode, 0);
                            } else {
                                // Normally should never happen
                                throw DBEngineRequestFactoryError("UPDATE: Invalid SET statement");
                            }
                            columns.emplace_back(std::string(), std::move(column));
                        } else {
                            throw DBEngineRequestFactoryError(
                                    "UPDATE: SET Expression column not found");
                        }

                        /// --------- Parse '=' ---------
                        const auto assignNode = node->children[i + 1];
                        terminalType = helpers::getTerminalType(assignNode);
                        if (terminalType != SiodbParser::ASSIGN)
                            throw DBEngineRequestFactoryError("UPDATE: missing = in SET");

                        /// --------- Parse value ---------
                        const auto valueExpr = node->children[i + 2];
                        nonTerminalType = helpers::getNonTerminalType(valueExpr);
                        if (nonTerminalType == SiodbParser::RuleExpr) {
                            values.push_back(exprFactory.createExpression(valueExpr));
                        } else
                            throw DBEngineRequestFactoryError("UPDATE: missing SET value");

                        // +4, for Column, '=', expr', ',' + 3 for new set expr
                        if (i + 7 <= node->children.size()) {
                            const auto commaNode = node->children[i + 3];
                            terminalType = helpers::getTerminalType(commaNode);
                            if (terminalType != SiodbParser::COMMA) {
                                throw DBEngineRequestFactoryError(
                                        "UPDATE: missing comma separator");
                            }
                            i += 4;
                        } else
                            i += 3;

                        ++updateValueCount;
                    }
                } else if (terminalType == SiodbParser::K_UPDATE)
                    continue;
                else {
                    throw DBEngineRequestFactoryError(
                            "UPDATE: Expression is invalid or unsupported");
                }

                break;
            }
            default: continue;
        }
    }

    if (columns.empty()) throw DBEngineRequestFactoryError("UPDATE: Missing columns");

    if (columns.size() != values.size())
        throw DBEngineRequestFactoryError("UPDATE: Column count is not equal to the value count");

    return std::make_unique<requests::UpdateRequest>(std::move(database),
            requests::SourceTable(std::move(table), std::move(tableAlias)), std::move(columns),
            std::move(values), std::move(where));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createDeleteRequest(
        antlr4::tree::ParseTree* node)
{
    std::string database, table, tableAlias;
    requests::ConstExpressionPtr where;

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        const auto e = node->children[i];
        const auto nonTerminalType = helpers::getNonTerminalType(e);
        switch (nonTerminalType) {
            case SiodbParser::RuleAliased_qualified_table_name: {
                // Capture Database name
                const auto databaseNameNode = helpers::findTerminal(
                        e, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
                if (databaseNameNode) {
                    database = databaseNameNode->getText();
                    boost::to_upper(database);
                }

                // Capture table name
                const auto tableNameNode = helpers::findTerminal(
                        e, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
                if (tableNameNode) {
                    table = tableNameNode->getText();
                    boost::to_upper(table);
                } else
                    throw DBEngineRequestFactoryError("DELETE: missing table ID");

                // Capture table alias
                const auto tableAliasNode = helpers::findTerminal(
                        e, SiodbParser::RuleTable_alias, SiodbParser::IDENTIFIER);
                if (tableAliasNode) {
                    tableAlias = tableAliasNode->getText();
                    boost::to_upper(tableAlias);
                }
                break;
            }
            case kInvalidNodeType: {
                const auto terminalType = helpers::getTerminalType(e);
                if (terminalType == SiodbParser::K_WHERE) {
                    ++i;
                    if (i >= node->children.size()) {
                        throw DBEngineRequestFactoryError(
                                "DELETE: WHERE clause does not contain expression");
                    }

                    ExpressionFactory exprFactory(m_parser, true);
                    where = exprFactory.createExpression(node->children[i]);
                }
                break;
            };
            default: continue;
        }
    }

    return std::make_unique<requests::DeleteRequest>(std::move(database),
            requests::SourceTable(std::move(table), std::move(tableAlias)), std::move(where));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createBeginTransactionRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture transaction type. Default one is "deferred".
    requests::TransactionType transactionType = requests::TransactionType::kDeferred;
    do {
        auto transactionTypeNode = helpers::findTerminal(node, SiodbParser::K_DEFERRED);
        if (transactionTypeNode) break;

        transactionTypeNode = helpers::findTerminal(node, SiodbParser::K_IMMEDIATE);
        if (transactionTypeNode) {
            transactionType = requests::TransactionType::kImmediate;
            break;
        }

        transactionTypeNode = helpers::findTerminal(node, SiodbParser::K_EXCLUSIVE);
        if (transactionTypeNode) {
            transactionType = requests::TransactionType::kExclusive;
            break;
        }
    } while (false);

    // Capture transaction ID
    std::string transaction;
    const auto transactionIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (transactionIdNode) {
        transaction = transactionIdNode->getText();
        boost::to_upper(transaction);
    }

    return std::make_unique<requests::BeginTransactionRequest>(
            transactionType, std::move(transaction));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createCommitTransactionRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture transaction ID
    std::string transaction;
    const auto transactionIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (transactionIdNode) {
        transaction = transactionIdNode->getText();
        boost::to_upper(transaction);
    }

    return std::make_unique<requests::CommitTransactionRequest>(std::move(transaction));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createRollbackTransactionRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture transaction ID
    std::string transaction;
    const auto transactionIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (transactionIdNode) {
        transaction = transactionIdNode->getText();
        boost::to_upper(transaction);
    }

    // Capture savepoint ID
    std::string savepoint;
    const auto savepointIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (savepointIdNode) {
        savepoint = savepointIdNode->getText();
        boost::to_upper(savepoint);
    }

    return std::make_unique<requests::RollbackTransactionRequest>(
            std::move(transaction), std::move(savepoint));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSavepointRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture savepoint ID
    std::string savepoint;
    const auto savepointIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (savepointIdNode) {
        savepoint = savepointIdNode->getText();
        boost::to_upper(savepoint);
    } else
        throw DBEngineRequestFactoryError("SAVEPOINT: missing savepoint ID");

    return std::make_unique<requests::SavepointRequest>(std::move(savepoint));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createReleaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture savepoint ID
    std::string savepoint;
    const auto savepointIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (savepointIdNode) {
        savepoint = savepointIdNode->getText();
        boost::to_upper(savepoint);
    } else
        throw DBEngineRequestFactoryError("RELEASE: missing savepoint ID");

    return std::make_unique<requests::SavepointRequest>(std::move(savepoint));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createAttachDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database UUID
    Uuid databaseUuid;
    const auto uuidNode =
            helpers::findTerminal(node, SiodbParser::RuleExpr, SiodbParser::STRING_LITERAL);
    if (uuidNode) {
        databaseUuid =
                boost::uuids::string_generator()(helpers::unquoteString(uuidNode->getText()));
    } else
        throw DBEngineRequestFactoryError("ATTACH DATABASE: missing database UUID");

    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    } else
        throw DBEngineRequestFactoryError("ATTACH DATABASE: missing database ID");

    return std::make_unique<requests::AttachDatabaseRequest>(
            std::move(databaseUuid), std::move(database));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createDetachDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    } else
        throw DBEngineRequestFactoryError("DETACH DATABASE: missing database ID");

    // Check for "IF EXISTS" clause
    const auto ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);

    return std::make_unique<requests::DetachDatabaseRequest>(std::move(database), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createCreateDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Normally should never happen
    if (node->children.size() < 3)
        throw DBEngineRequestFactoryError("CREATE DATABASE: malformed statement");

    // Database node could be 2 or 3 (CREATE TEMPORARY DATABASE <name>)
    bool isTemporary = false;
    std::uint32_t maxTableCount = 0;
    size_t databaseNodeIndex = 0;
    if (helpers::getNonTerminalType(node->children[2]) == SiodbParser::RuleDatabase_name) {
        databaseNodeIndex = 2;
    } else if (node->children.size() > 3
               && helpers::getNonTerminalType(node->children[3])
                          == SiodbParser::RuleDatabase_name) {
        databaseNodeIndex = 3;
        isTemporary = true;
    } else
        throw DBEngineRequestFactoryError("CREATE DATABASE: missing database name");

    auto database = helpers::extractObjectName(node, databaseNodeIndex);

    //  <name> + WITH + <List of options>
    requests::ConstExpressionPtr cipherId, cipherKeySeed;
    if (node->children.size() == databaseNodeIndex + 3) {
        if (helpers::getNonTerminalType(node->children[databaseNodeIndex + 2])
                != SiodbParser::RuleCreate_database_attr_list)
            throw DBEngineRequestFactoryError("CREATE DATABASE: missing option list");

        const auto attrListNode = node->children[databaseNodeIndex + 2];
        ExpressionFactory exprFactory(m_parser);
        for (std::size_t i = 0, n = attrListNode->children.size(); i < n; i += 2) {
            auto attrNode = attrListNode->children[i];
            switch (helpers::getTerminalType(attrNode->children.at(0))) {
                case SiodbParser::K_CIPHER_ID: {
                    cipherId = exprFactory.createExpression(attrNode->children.at(2));
                    break;
                }
                case SiodbParser::K_CIPHER_KEY_SEED: {
                    cipherKeySeed = exprFactory.createExpression(attrNode->children.at(2));
                    break;
                }
                default: throw DBEngineRequestFactoryError("CREATE DATABASE: invalid attribute");
            }
        }
    } else if (node->children.size() != databaseNodeIndex + 1)
        throw DBEngineRequestFactoryError("CREATE DATABASE: malformed statement");

    return std::make_unique<requests::CreateDatabaseRequest>(std::move(database), isTemporary,
            std::move(cipherId), std::move(cipherKeySeed), maxTableCount);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createDropDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    const auto ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);
    auto database = helpers::extractObjectName(node, ifExists ? 4 : 2);
    return std::make_unique<requests::DropDatabaseRequest>(std::move(database), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createRenameDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    auto database = helpers::extractObjectName(node, 2);
    const auto ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);
    auto newDatabase = helpers::extractObjectName(node, ifExists ? 7 : 5);
    return std::make_unique<requests::RenameDatabaseRequest>(
            std::move(database), std::move(newDatabase), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSetDatabaseAttributesRequest(
        [[maybe_unused]] antlr4::tree::ParseTree* node)
{
    auto database = helpers::extractObjectName(node, 2);
    std::optional<std::optional<std::string>> description;
    const auto attrListNode = node->children.at(4);
    for (std::size_t i = 0, n = attrListNode->children.size(); i < n; i += 2) {
        const auto attrNode = attrListNode->children[i];
        switch (helpers::getTerminalType(attrNode->children.at(0))) {
            case SiodbParser::K_DESCRIPTION: {
                const auto valueNode = attrNode->children.at(2);
                description = (helpers::getTerminalType(valueNode) == SiodbParser::K_NULL)
                                      ? std::optional<std::string>()
                                      : std::optional<std::string>(
                                              helpers::unquoteString(valueNode->getText()));
                break;
            }
            default: {
                throw DBEngineRequestFactoryError(
                        "ALTER DATABASE SET ATTRIBUTES: invalid attribute");
            }
        }
    }
    return std::make_unique<requests::SetDatabaseAttributesRequest>(
            std::move(database), std::move(description));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createUseDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    } else
        throw DBEngineRequestFactoryError("USE DATABASE: missing database ID");

    return std::make_unique<requests::UseDatabaseRequest>(std::move(database));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createCreateTableRequest(
        antlr4::tree::ParseTree* node)
{
    const auto tableSpecNode = helpers::findNonTerminalChild(node, SiodbParser::RuleTable_spec);

    // Capture database ID
    std::string database;
    const auto databaseIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture table ID
    std::string table;
    const auto tableIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode) {
        table = tableIdNode->getText();
        boost::to_upper(table);
    } else
        throw DBEngineRequestFactoryError("CREATE TABLE: missing table ID");

    // Capture column definitions
    std::vector<requests::ColumnDefinition> columns;
    for (const auto columnDefNode : node->children) {
        if (helpers::getNonTerminalType(columnDefNode) != SiodbParser::RuleColumn_def) continue;

        // Find column ID
        const auto columnIdNode = helpers::findTerminal(
                columnDefNode, SiodbParser::RuleColumn_name, SiodbParser::IDENTIFIER);
        if (!columnIdNode) throw DBEngineRequestFactoryError("CREATE TABLE: missing column ID");
        auto column = columnIdNode->getText();
        boost::to_upper(column);

        // Find column data type
        const auto typeNameNode =
                helpers::findNonTerminal(columnDefNode, SiodbParser::RuleType_name);
        if (typeNameNode == nullptr)
            throw DBEngineRequestFactoryError("CREATE TABLE: missing column data type");

        // Capture data type
        std::ostringstream typeName;
        std::size_t idNodeCount = 0;
        for (const auto childNode : typeNameNode->children) {
            const auto idNode = helpers::findTerminal(childNode, SiodbParser::IDENTIFIER);
            if (idNode) {
                if (idNodeCount > 0) typeName << ' ';
                auto id = idNode->getText();
                boost::to_upper(id);
                typeName << id;
                ++idNodeCount;
            }
        }
        const auto columnDataType = getColumnDataType(typeName.str());

        // Capture constraints
        std::vector<std::unique_ptr<requests::Constraint>> constraints;
        for (const auto constraintNode : columnDefNode->children) {
            if (helpers::getNonTerminalType(constraintNode) != SiodbParser::RuleColumn_constraint)
                continue;

            // Capture constraint name if provided
            std::string constraintName;
            const auto constraintNameNode =
                    helpers::findNonTerminal(constraintNode, SiodbParser::RuleName);
            while (constraintNameNode) {
                auto idNode =
                        helpers::findTerminal(constraintNameNode, SiodbParser::STRING_LITERAL);
                if (idNode) {
                    constraintName = idNode->getText();
                    boost::to_upper(constraintName);
                    constraintName = constraintName.substr(1, constraintName.length() - 2);
                    break;
                }
                idNode = helpers::findTerminal(constraintNameNode, SiodbParser::IDENTIFIER);
                if (idNode) {
                    constraintName = idNode->getText();
                    boost::to_upper(constraintName);
                    break;
                }
            }

            // Check for NOT NULL constraint
            auto terminal = helpers::findTerminal(constraintNode, SiodbParser::K_NULL);
            if (terminal) {
                const auto notTerminal = helpers::findTerminal(constraintNode, SiodbParser::K_NOT);
                const bool notNull = notTerminal != nullptr;
                constraints.emplace_back(std::make_unique<requests::NotNullConstraint>(
                        std::move(constraintName), notNull));
                continue;
            }

            // Check for UNIQUE constraint
            terminal = helpers::findTerminal(constraintNode, SiodbParser::K_UNIQUE);
            if (terminal) {
                std::vector<std::string> columns;
                columns.push_back(column);
                constraints.emplace_back(std::make_unique<requests::UniqueConstraint>(
                        std::move(constraintName), std::move(columns)));
                continue;
            }

            // Check for DEFAULT constraint
            auto terminalIndex = helpers::findTerminalChild(constraintNode, SiodbParser::K_DEFAULT);
            if (terminalIndex != std::numeric_limits<std::size_t>::max()) {
                //DBG_LOG_DEBUG("Parsing default value for column " << columnName);
                auto expressionIndex = terminalIndex + 1;
                const auto terminalType =
                        helpers::getTerminalType(constraintNode->children[expressionIndex]);
                if (terminalType == SiodbParser::OPEN_PAR) ++expressionIndex;
                ExpressionFactory exprFactory(m_parser);
                auto defaultValue =
                        exprFactory.createExpression(constraintNode->children[expressionIndex]);
                constraints.emplace_back(std::make_unique<requests::DefaultValueConstraint>(
                        std::move(constraintName), std::move(defaultValue)));
                continue;
            }

            // Check for PRIMARY KEY constraint
            terminal = helpers::findTerminal(constraintNode, SiodbParser::K_PRIMARY);
            if (terminal) {
                throw DBEngineRequestFactoryError(
                        "CREATE TABLE: PRIMARY KEY constraint is not supported");
            }

            // Check for REFERENCES constraint
            terminal = helpers::findTerminal(constraintNode, SiodbParser::K_REFERENCES);
            if (terminal) {
                // TODO: Parse target table and column
                std::string targetTable;
                std::string targetTableColumn;
                constraints.emplace_back(
                        std::make_unique<requests::ReferencesConstraint>(std::move(constraintName),
                                std::move(targetTable), std::move(targetTableColumn)));
                continue;
            }

            // Check for CHECK constraint
            terminal = helpers::findTerminal(constraintNode, SiodbParser::K_CHECK);
            if (terminal) {
                // TODO: Parse CHECK expression
                requests::ExpressionPtr expression;
                constraints.emplace_back(std::make_unique<requests::CheckConstraint>(
                        std::move(constraintName), std::move(expression)));
                continue;
            }

            // Check for COLLATE constraint
            terminal = helpers::findTerminal(constraintNode, SiodbParser::K_COLLATE);
            if (terminal) {
                // TODO: Parse collation type
                const auto collationType = requests::CollationType::kBinary;
                constraints.emplace_back(std::make_unique<requests::CollateConstraint>(
                        std::move(constraintName), collationType));
                continue;
            }

            // Should never reach here
            throw DBEngineRequestFactoryError("CREATE TABLE: Unsupported constraint type");
        }

        // Add column record
        columns.emplace_back(std::move(column), columnDataType, kDefaultDataFileDataAreaSize,
                std::move(constraints));
    }

    return std::make_unique<requests::CreateTableRequest>(
            std::move(database), std::move(table), std::move(columns));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createDropTableRequest(
        antlr4::tree::ParseTree* node)
{
    const auto tableSpecNode = helpers::findNonTerminalChild(node, SiodbParser::RuleTable_spec);

    // Capture database ID
    std::string database;
    const auto databaseIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture table ID
    std::string table;
    const auto tableIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode) {
        table = tableIdNode->getText();
        boost::to_upper(table);
    } else
        throw DBEngineRequestFactoryError("DROP TABLE: missing table ID");

    // Check for "IF EXISTS" clause
    const auto ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);

    return std::make_unique<requests::DropTableRequest>(
            std::move(database), std::move(table), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createRenameTableRequest(
        antlr4::tree::ParseTree* node)
{
    const auto tableSpecNode = node->children.at(2);

    // Capture database ID
    std::string database;
    const auto databaseIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture old table ID
    std::string oldTable;
    const auto oldTableIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (oldTableIdNode) {
        oldTable = oldTableIdNode->getText();
        boost::to_upper(oldTable);
    } else
        throw DBEngineRequestFactoryError("ALTER TABLE RENAME TO: missing table ID");

    // Capture new table ID
    std::string newTable;
    const auto newTableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleNew_table_name, SiodbParser::IDENTIFIER);
    if (newTableIdNode) {
        newTable = newTableIdNode->getText();
        boost::to_upper(newTable);
    } else
        throw DBEngineRequestFactoryError("ALTER TABLE RENAME TO: missing new table ID");

    // Check for "IF EXISTS" clause
    const bool ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);

    return std::make_unique<requests::RenameTableRequest>(
            std::move(database), std::move(oldTable), std::move(newTable), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSetTableAttributesRequest(
        antlr4::tree::ParseTree* node)
{
    const auto tableSpecNode = node->children.at(2);

    // Capture database ID
    std::string database;
    const auto databaseIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture table ID
    std::string table;
    const auto tableIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode) {
        table = tableIdNode->getText();
        boost::to_upper(table);
    } else
        throw DBEngineRequestFactoryError("ALTER TABLE SET ATTRIBUTES: missing table ID");

    std::optional<std::uint64_t> nextTrid;

    const auto attrListNode = helpers::findNonTerminal(node, SiodbParser::RuleTable_attr_list);
    if (attrListNode == nullptr)
        throw DBEngineRequestFactoryError("ALTER TABLE SET ATTRIBUTES: missing attribute list");
    for (std::size_t i = 0, n = attrListNode->children.size(); i < n; i += 2) {
        const auto attrNode = attrListNode->children[i];
        switch (helpers::getTerminalType(attrNode->children.at(0))) {
            case SiodbParser::K_NEXT_TRID: {
                const auto value = attrNode->children.at(2)->getText();
                try {
                    size_t index = 0;
                    nextTrid = std::stoull(value, &index, 10);
                    if (index != value.length()) throw std::invalid_argument("parse error");
                } catch (std::exception& ex) {
                    throw DBEngineRequestFactoryError(
                            "ALTER TABLE SET ATTRIBUTES: invalid integer value of the attribute "
                            "NEXT_TRID");
                }
                break;
            }
            default:
                throw DBEngineRequestFactoryError("ALTER TABLE SET ATTRIBUTES: invalid attribute");
        }
    }

    return std::make_unique<requests::SetTableAttributesRequest>(
            std::move(database), std::move(table), std::move(nextTrid));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createAddColumnRequest(
        antlr4::tree::ParseTree* node)
{
    const auto tableSpecNode = node->children.at(2);

    // Capture database ID
    std::string database;
    const auto databaseIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture table ID
    std::string table;
    const auto tableIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode) {
        table = tableIdNode->getText();
        boost::to_upper(table);
    } else
        throw DBEngineRequestFactoryError("ALTER TABLE ADD COLUMN: missing table ID");

    // Find column ID
    const auto columnIdNode =
            helpers::findTerminal(node, SiodbParser::RuleColumn_name, SiodbParser::IDENTIFIER);
    if (!columnIdNode)
        throw DBEngineRequestFactoryError("ALTER TABLE ADD COLUMN: missing column ID");
    auto column = columnIdNode->getText();
    boost::to_upper(column);

    // Find column data type
    const auto typeNameNode =
            helpers::findTerminal(node, SiodbParser::RuleType_name, SiodbParser::IDENTIFIER);
    if (typeNameNode == nullptr)
        throw DBEngineRequestFactoryError("ALTER TABLE ADD COLUMN: missing column data type");
    auto typeName = typeNameNode->getText();
    boost::to_upper(typeName);
    const auto columnDataType = getColumnDataType(typeName);

    // Fill new column info
    requests::ColumnDefinition columnDefinition(
            std::move(column), columnDataType, kDefaultDataFileDataAreaSize, {});

    return std::make_unique<requests::AddColumnRequest>(
            std::move(database), std::move(table), std::move(columnDefinition));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createDropColumnRequest(
        antlr4::tree::ParseTree* node)
{
    const auto tableSpecNode = node->children.at(2);

    // Capture database ID
    std::string database;
    const auto databaseIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture table ID
    std::string table;
    const auto tableIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode) {
        table = tableIdNode->getText();
        boost::to_upper(table);
    } else
        throw DBEngineRequestFactoryError("ALTER TABLE DROP COLUMN: missing table ID");

    // Capture column ID
    std::string column;
    const auto columnIdNode =
            helpers::findTerminal(node, SiodbParser::RuleColumn_name, SiodbParser::IDENTIFIER);
    if (columnIdNode) {
        column = columnIdNode->getText();
        boost::to_upper(column);
    } else
        throw DBEngineRequestFactoryError("ALTER TABLE DROP COLUMN: missing table ID");

    // Check for "IF EXISTS" clause
    const auto ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);

    return std::make_unique<requests::DropColumnRequest>(
            std::move(database), std::move(table), std::move(column), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createRenameColumnRequest(
        antlr4::tree::ParseTree* node)
{
    const auto tableSpecNode = node->children.at(2);

    std::string database;
    const auto databaseIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    std::string table;
    const auto tableIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode) {
        table = tableIdNode->getText();
        boost::to_upper(table);
    } else
        throw DBEngineRequestFactoryError("ALTER TABLE ALTER COLUMN RENAME TO: missing table ID");

    auto column = node->children.at(5)->getText();
    boost::to_upper(column);
    const auto ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);
    auto newColumn = helpers::extractObjectName(node, ifExists ? 10 : 8);

    return std::make_unique<requests::RenameColumnRequest>(std::move(database), std::move(table),
            std::move(column), std::move(newColumn), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createRedefineColumnRequest(
        antlr4::tree::ParseTree* node)
{
    const auto tableSpecNode = node->children.at(2);

    // Capture database ID
    std::string database;
    const auto databaseIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture table ID
    std::string table;
    const auto tableIdNode = helpers::findTerminal(
            tableSpecNode, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode) {
        table = tableIdNode->getText();
        boost::to_upper(table);
    } else
        throw DBEngineRequestFactoryError("ALTER TABLE ALTER COLUMN: missing table ID");

    // Find column ID
    const auto columnIdNode =
            helpers::findTerminal(node, SiodbParser::RuleColumn_name, SiodbParser::IDENTIFIER);
    if (!columnIdNode)
        throw DBEngineRequestFactoryError("ALTER TABLE ALTER COLUMN: missing column ID");
    auto columnName = columnIdNode->getText();
    boost::to_upper(columnName);

    // Find column data type
    const auto typeNameNode =
            helpers::findTerminal(node, SiodbParser::RuleType_name, SiodbParser::IDENTIFIER);
    if (typeNameNode == nullptr)
        throw DBEngineRequestFactoryError("ALTER TABLE ALTER COLUMN: missing column data type");
    auto typeName = typeNameNode->getText();
    boost::to_upper(typeName);
    const auto columnDataType = getColumnDataType(typeName);

    // Fill new column info
    requests::ColumnDefinition column(
            std::move(columnName), columnDataType, kDefaultDataFileDataAreaSize, {});

    return std::make_unique<requests::RedefineColumnRequest>(
            std::move(database), std::move(table), std::move(column));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createCreateIndexRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture index name
    std::string index;
    const auto indexNameNode =
            helpers::findTerminal(node, SiodbParser::RuleIndex_name, SiodbParser::IDENTIFIER);
    if (indexNameNode) {
        index = indexNameNode->getText();
        boost::to_upper(index);
    } else
        throw DBEngineRequestFactoryError("CREATE INDEX: missing index name");

    // Capture table ID
    std::string table;
    const auto tableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode) {
        table = tableIdNode->getText();
        boost::to_upper(table);
    } else
        throw DBEngineRequestFactoryError("CREATE INDEX: missing table ID");

    // Capture column definitions
    std::vector<requests::IndexColumnDefinition> columns;
    for (const auto e : node->children) {
        if (helpers::getNonTerminalType(e) != SiodbParser::RuleIndexed_column) continue;

        // Find column ID
        const auto columnIdNode =
                helpers::findTerminal(e, SiodbParser::RuleColumn_name, SiodbParser::IDENTIFIER);
        if (!columnIdNode) throw DBEngineRequestFactoryError("CREATE INDEX: missing column ID");

        // Find sort order
        const bool sortDescending = helpers::findTerminal(e, SiodbParser::K_DESC) != nullptr;

        // Add column record
        auto columnId = columnIdNode->getText();
        boost::to_upper(columnId);
        columns.emplace_back(std::move(columnId), sortDescending);
    }

    // Check "UNIQUE" presence
    const auto uniqueNode = helpers::findTerminal(node, SiodbParser::K_UNIQUE);
    const bool unique = (uniqueNode != nullptr);

    // Check for "IF NOT EXISTS" clause
    const auto ifNode = helpers::findTerminal(node, SiodbParser::K_IF);
    const bool ifDoesntExist = (ifNode != nullptr);

    return std::make_unique<requests::CreateIndexRequest>(std::move(database), std::move(table),
            std::move(index), std::move(columns), unique, ifDoesntExist);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createDropIndexRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) {
        database = databaseIdNode->getText();
        boost::to_upper(database);
    }

    // Capture index name
    std::string index;
    const auto indexNameNode =
            helpers::findTerminal(node, SiodbParser::RuleIndex_name, SiodbParser::IDENTIFIER);
    if (indexNameNode) {
        index = indexNameNode->getText();
        boost::to_upper(index);
    } else
        throw DBEngineRequestFactoryError("DROP INDEX: missing index name");

    // Check for "IF EXISTS" clause
    const auto ifNode = helpers::findTerminal(node, SiodbParser::K_IF);
    const bool ifExists = ifNode != nullptr;

    return std::make_unique<requests::DropIndexRequest>(
            std::move(database), std::move(index), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createCreateUserRequest(
        antlr4::tree::ParseTree* node)
{
    // Normally should never happen
    if (node->children.size() < 3) DBEngineRequestFactoryError("CREATE USER: malformed statement");

    // Get node text as is without 'GetAnyText' call.
    auto name = helpers::extractObjectName(node, 2);
    std::optional<std::string> realName, description;
    bool active = true;

    //  <name> + WITH + <List of options>
    if (node->children.size() > 4) {
        if (helpers::getNonTerminalType(node->children[4]) != SiodbParser::RuleUser_attr_list)
            throw DBEngineRequestFactoryError("CREATE USER: missing options list");

        const auto attrListNode = node->children[4];
        for (std::size_t i = 0, n = attrListNode->children.size(); i < n; i += 2) {
            const auto attrNode = attrListNode->children[i];
            switch (helpers::getTerminalType(attrNode->children.at(0))) {
                case SiodbParser::K_REAL_NAME: {
                    const auto valueNode = attrNode->children.at(2);
                    if (helpers::getTerminalType(valueNode) == SiodbParser::K_NULL)
                        realName.reset();
                    else
                        realName = helpers::unquoteString(valueNode->getText());
                    break;
                }
                case SiodbParser::K_DESCRIPTION: {
                    const auto valueNode = attrNode->children.at(2);
                    if (helpers::getTerminalType(valueNode) == SiodbParser::K_NULL)
                        description.reset();
                    else
                        description = helpers::unquoteString(attrNode->children.at(2)->getText());
                    break;
                }
                case SiodbParser::K_STATE: {
                    active =
                            parseState(attrNode->children.at(2), "CREATE USER: invalid user state");
                    break;
                }
                default: throw DBEngineRequestFactoryError("CREATE USER: invalid attribute");
            }
        }
    }

    return std::make_unique<requests::CreateUserRequest>(
            std::move(name), std::move(realName), std::move(description), active);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createDropUserRequest(
        antlr4::tree::ParseTree* node)
{
    // Normally should never happen
    if (node->children.size() < 3)
        throw DBEngineRequestFactoryError("DROP USER: request is malformed");

    // Get node text as is without 'GetAnyText' call.
    auto name = helpers::extractObjectName(node, 2);
    return std::make_unique<requests::DropUserRequest>(std::move(name), false);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSetUserAttributesRequest(
        antlr4::tree::ParseTree* node)
{
    // Normally should never happen
    if (node->children.size() < 5)
        throw DBEngineRequestFactoryError("ALTER USER: malformed statement");

    auto name = helpers::extractObjectName(node, 2);

    std::optional<std::optional<std::string>> realName, description;
    std::optional<bool> active;

    const auto attrListNode = node->children[4];
    // skip comma (option ',' option ... )
    for (std::size_t i = 0, n = attrListNode->children.size(); i < n; i += 2) {
        const auto attrNode = attrListNode->children[i];
        switch (helpers::getTerminalType(attrNode->children.at(0))) {
            case SiodbParser::K_REAL_NAME: {
                const auto valueNode = attrNode->children.at(2);
                realName = (helpers::getTerminalType(valueNode) == SiodbParser::K_NULL)
                                   ? std::optional<std::string>()
                                   : std::optional<std::string>(
                                           helpers::unquoteString(valueNode->getText()));
                break;
            }
            case SiodbParser::K_DESCRIPTION: {
                const auto valueNode = attrNode->children.at(2);
                description = (helpers::getTerminalType(valueNode) == SiodbParser::K_NULL)
                                      ? std::optional<std::string>()
                                      : std::optional<std::string>(
                                              helpers::unquoteString(valueNode->getText()));
                break;
            }
            case SiodbParser::K_STATE: {
                active = parseState(attrNode->children.at(2), "ALTER USER: invalid user state");
                break;
            }
            default: throw DBEngineRequestFactoryError("ALTER USER: invalid attribute");
        }
    }

    return std::make_unique<requests::SetUserAttributesRequest>(
            std::move(name), std::move(realName), std::move(description), std::move(active));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createAddUserAccessKeyRequest(
        antlr4::tree::ParseTree* node)
{
    auto userName = helpers::extractObjectName(node, 2);
    auto keyName = helpers::extractObjectName(node, 6);
    auto keyText = helpers::unquoteString(node->children.at(7)->getText());
    std::optional<std::string> description;
    bool active = true;

    if (node->children.size() > 9) {
        const auto attrListNode = node->children[9];
        for (std::size_t i = 0, n = attrListNode->children.size(); i < n; i += 2) {
            const auto attrNode = attrListNode->children[i];
            switch (helpers::getTerminalType(attrNode->children.at(0))) {
                case SiodbParser::K_DESCRIPTION: {
                    const auto valueNode = attrNode->children.at(2);
                    if (helpers::getTerminalType(valueNode) == SiodbParser::K_NULL)
                        description.reset();
                    else
                        description = helpers::unquoteString(attrNode->children.at(2)->getText());
                    break;
                }
                case SiodbParser::K_STATE: {
                    active = parseState(attrNode->children.at(2),
                            "ALTER USER ADD ACCESS KEY: invalid key state");
                    break;
                }
                default:
                    throw DBEngineRequestFactoryError(
                            "ALTER USER ADD ACCESS KEY: invalid attribute");
            }
        }
    }

    return std::make_unique<requests::AddUserAccessKeyRequest>(std::move(userName),
            std::move(keyName), std::move(keyText), std::move(description), active);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createDropUserAccessKeyRequest(
        antlr4::tree::ParseTree* node)
{
    auto userName = helpers::extractObjectName(node, 2);
    const bool ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);
    auto keyName = helpers::extractObjectName(node, ifExists ? 8 : 6);
    return std::make_unique<requests::DropUserAccessKeyRequest>(
            std::move(userName), std::move(keyName), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSetUserAccessKeyAttributesRequest(
        antlr4::tree::ParseTree* node)
{
    auto userName = helpers::extractObjectName(node, 2);
    auto keyName = helpers::extractObjectName(node, 6);
    std::optional<std::optional<std::string>> description;
    std::optional<bool> active;

    const auto attrListNode = node->children.at(8);
    for (std::size_t i = 0, n = attrListNode->children.size(); i < n; i += 2) {
        const auto attrNode = attrListNode->children[i];
        switch (helpers::getTerminalType(attrNode->children.at(0))) {
            case SiodbParser::K_DESCRIPTION: {
                const auto valueNode = attrNode->children.at(2);
                description = (helpers::getTerminalType(valueNode) == SiodbParser::K_NULL)
                                      ? std::optional<std::string>()
                                      : std::optional<std::string>(helpers::unquoteString(
                                              attrNode->children.at(2)->getText()));
                break;
            }
            case SiodbParser::K_STATE: {
                active = parseState(
                        attrNode->children.at(2), "ALTER USER ALTER ACCESS KEY: invalid key state");
                break;
            }
            default:
                throw DBEngineRequestFactoryError("ALTER USER ALTER ACCESS KEY: invalid attribute");
        }
    }

    return std::make_unique<requests::SetUserAccessKeyAttributesRequest>(
            std::move(userName), std::move(keyName), std::move(description), std::move(active));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createRenameUserAccessKeyRequest(
        antlr4::tree::ParseTree* node)
{
    auto userName = helpers::extractObjectName(node, 2);
    auto keyName = helpers::extractObjectName(node, 6);
    const bool ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);
    auto newKeyName = helpers::extractObjectName(node, ifExists ? 11 : 9);
    return std::make_unique<requests::RenameUserAccessKeyRequest>(
            std::move(userName), std::move(keyName), std::move(newKeyName), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createAddUserTokenRequest(
        antlr4::tree::ParseTree* node)
{
    ExpressionFactory exprFactory(m_parser);
    auto userName = helpers::extractObjectName(node, 2);
    auto tokenName = helpers::extractObjectName(node, 5);
    std::optional<BinaryValue> tokenValue;
    std::optional<std::time_t> expirationTimestamp;
    std::optional<std::string> description;
    antlr4::tree::ParseTree* attrListNode = nullptr;

    if (node->children.size() > 6) {
        auto node6 = node->children.at(6);
        if (helpers::getTerminalType(node6) == SiodbParser::K_WITH)
            attrListNode = node->children.at(7);
        else {
            auto v = exprFactory.createConstantValue(node6->children.at(0));
            tokenValue = std::move(v.getBinary());
            if (node->children.size() > 8) attrListNode = node->children.at(8);
        }
    }

    if (attrListNode) {
        for (std::size_t i = 0, n = attrListNode->children.size(); i < n; i += 2) {
            const auto attrNode = attrListNode->children[i];
            const auto valueNode = attrNode->children.at(2);
            const bool isNullValue = helpers::getTerminalType(valueNode) == SiodbParser::K_NULL;
            switch (helpers::getTerminalType(attrNode->children.at(0))) {
                case SiodbParser::K_DESCRIPTION: {
                    if (isNullValue)
                        description.reset();
                    else
                        description = helpers::unquoteString(attrNode->children.at(2)->getText());
                    break;
                }
                case SiodbParser::K_EXPIRATION_TIMESTAMP: {
                    if (isNullValue)
                        expirationTimestamp.reset();
                    else {
                        expirationTimestamp = parseExpirationTimestamp(
                                helpers::unquoteString(attrNode->children.at(2)->getText()));
                    }
                    break;
                }
                default:
                    throw DBEngineRequestFactoryError("ALTER USER ADD TOKEN: invalid attribute");
            }
        }
    }

    return std::make_unique<requests::AddUserTokenRequest>(std::move(userName),
            std::move(tokenName), std::move(tokenValue), std::move(expirationTimestamp),
            std::move(description));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createDropUserTokenRequest(
        antlr4::tree::ParseTree* node)
{
    auto userName = helpers::extractObjectName(node, 2);
    const bool ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);
    auto tokenName = helpers::extractObjectName(node, ifExists ? 7 : 5);
    return std::make_unique<requests::DropUserTokenRequest>(
            std::move(userName), std::move(tokenName), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createSetUserTokenAttributesRequest(
        antlr4::tree::ParseTree* node)
{
    auto userName = helpers::extractObjectName(node, 2);
    auto tokenName = helpers::extractObjectName(node, 5);
    std::optional<std::optional<std::time_t>> expirationTimestamp;
    std::optional<std::optional<std::string>> description;
    if (node->children.size() > 7) {
        const auto attrListNode = node->children.at(7);
        for (std::size_t i = 0, n = attrListNode->children.size(); i < n; i += 2) {
            const auto attrNode = attrListNode->children[i];
            const auto valueNode = attrNode->children.at(2);
            const bool isNullValue = helpers::getTerminalType(valueNode) == SiodbParser::K_NULL;
            switch (helpers::getTerminalType(attrNode->children.at(0))) {
                case SiodbParser::K_DESCRIPTION: {
                    description = isNullValue ? std::optional<std::string>()
                                              : std::optional<std::string>(helpers::unquoteString(
                                                      attrNode->children.at(2)->getText()));
                    break;
                }
                case SiodbParser::K_EXPIRATION_TIMESTAMP: {
                    expirationTimestamp =
                            isNullValue ? std::optional<std::time_t>()
                                        : std::optional<std::time_t>(
                                                parseExpirationTimestamp(helpers::unquoteString(
                                                        attrNode->children.at(2)->getText())));
                    break;
                }
                default: {
                    throw DBEngineRequestFactoryError(
                            "ALTER USER ALTER TOKEN SET ATTRIBUTES: invalid attribute");
                }
            }
        }
    }
    return std::make_unique<requests::SetUserTokenAttributesRequest>(std::move(userName),
            std::move(tokenName), std::move(expirationTimestamp), std::move(description));
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createRenameUserTokenRequest(
        [[maybe_unused]] antlr4::tree::ParseTree* node)
{
    auto userName = helpers::extractObjectName(node, 2);
    auto tokenName = helpers::extractObjectName(node, 5);
    const bool ifExists = helpers::hasTerminalChild(node, SiodbParser::K_IF);
    auto newTokenName = helpers::extractObjectName(node, ifExists ? 10 : 8);
    return std::make_unique<requests::RenameUserTokenRequest>(
            std::move(userName), std::move(tokenName), std::move(newTokenName), ifExists);
}

requests::DBEngineRequestPtr DBEngineSqlRequestFactory::createCheckUserTokenRequest(
        antlr4::tree::ParseTree* node)
{
    ExpressionFactory exprFactory(m_parser);
    auto userName = helpers::extractObjectName(node, 2);
    auto tokenName = helpers::extractObjectName(node, 4);
    auto v = exprFactory.createConstantValue(node->children.at(5)->children.at(0));
    auto tokenValue = std::move(v.getBinary());
    return std::make_unique<requests::CheckUserTokenRequest>(
            std::move(userName), std::move(tokenName), std::move(tokenValue));
}

siodb::ColumnDataType DBEngineSqlRequestFactory::getColumnDataType(const std::string& typeName)
{
    const auto it = m_siodbDataTypeMap.find(typeName);
    if (it != m_siodbDataTypeMap.end()) return it->second;
    throw DBEngineRequestFactoryError("Unsupported data type '" + typeName + "'");
}

requests::ResultExpression DBEngineSqlRequestFactory::createResultExpression(
        antlr4::tree::ParseTree* node)
{
    requests::ConstExpressionPtr expression;
    std::string alias;
    const auto childrenCount = node->children.size();
    // case: '*'
    if (childrenCount == 1 && helpers::getTerminalType(node->children[0]) == SiodbParser::STAR)
        expression = std::make_unique<requests::AllColumnsExpression>("");
    // case: table_name '.' '*'
    else if (childrenCount == 3
             && helpers::getTerminalType(node->children[2]) == SiodbParser::STAR) {
        expression = std::make_unique<requests::AllColumnsExpression>(
                helpers::extractObjectName(node, 0));
    }
    // case: expr ( K_AS? column_alias)?
    else if (childrenCount > 0
             && helpers::getNonTerminalType(node->children[0]) == SiodbParser::RuleExpr) {
        ExpressionFactory exprFactory(m_parser, true);
        expression = exprFactory.createExpression(node->children[0]);

        if (childrenCount > 1
                && helpers::getNonTerminalType(node->children.back())
                           == SiodbParser::RuleColumn_alias) {
            alias = helpers::extractObjectName(node, node->children.size() - 1);
        }
    } else
        throw DBEngineRequestFactoryError("Result column node is invalid");
    return requests::ResultExpression(std::move(expression), std::move(alias));
}

void DBEngineSqlRequestFactory::parseSelectCore(antlr4::tree::ParseTree* node,
        std::string& database, std::vector<requests::SourceTable>& tables,
        std::vector<requests::ResultExpression>& columns, requests::ConstExpressionPtr& where)
{
    ExpressionFactory exprFactory(m_parser, true);
    for (std::size_t i = 0, n = node->children.size(); i < n; ++i) {
        const auto e = node->children[i];
        const auto nonTerminalType = helpers::getNonTerminalType(e);
        switch (nonTerminalType) {
            case SiodbParser::RuleResult_column: {
                columns.push_back(createResultExpression(e));
                break;
            }
            case SiodbParser::RuleTable_or_subquery: {
                const auto databaseIdNode =
                        helpers::findNonTerminal(e, SiodbParser::RuleDatabase_name);
                if (databaseIdNode) {
                    database = databaseIdNode->getText();
                    boost::to_upper(database);
                }

                const auto tableIdNode = helpers::findNonTerminal(e, SiodbParser::RuleTable_name);
                if (tableIdNode) {
                    auto tableName = tableIdNode->getText();
                    boost::to_upper(tableName);
                    std::string tableAlias;
                    const auto tableAliasIdNode =
                            helpers::findNonTerminal(e, SiodbParser::RuleTable_alias);
                    if (tableAliasIdNode) {
                        tableAlias = tableAliasIdNode->getText();
                        boost::to_upper(tableAlias);
                    }
                    tables.emplace_back(std::move(tableName), std::move(tableAlias));
                } else
                    throw DBEngineRequestFactoryError("SELECT: missing table ID");
                break;
            }
            case kInvalidNodeType: {
                const auto terminalType = helpers::getTerminalType(e);
                if (terminalType == SiodbParser::K_WHERE) {
                    ++i;
                    if (i >= node->children.size()) {
                        throw DBEngineRequestFactoryError(
                                "SELECT: WHERE clause does not contain expression");
                    }
                    where = exprFactory.createExpression(node->children[i]);
                }
                break;
            };
            default: {
                throw DBEngineRequestFactoryError(
                        "SELECT: query contains unsupported non-terminal of type "
                        + std::to_string(nonTerminalType));
            }
        }
    }
}

}  // namespace siodb::iomgr::dbengine::parser
