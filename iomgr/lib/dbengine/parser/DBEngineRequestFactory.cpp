// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DBEngineRequestFactory.h"

// Project headers
#include "AntlrHelpers.h"
#include "antlr_wrappers/SiodbParserWrapper.h"
#include "expr/AllColumnsExpression.h"

// Common project headers
#include "expr/ExpressionFactory.h"
#include <siodb/common/log/Log.h>

// Boost headers
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/uuid/string_generator.hpp>

// Protobuf message headers
#include <siodb/common/proto/CommonTypes.pb.h>

namespace siodb::iomgr::dbengine::parser {

namespace {

bool parseStateNode(antlr4::tree::ParseTree* node, const char* errorMessage)
{
    switch (helpers::getTerminalType(node)) {
        case SiodbParser::K_ACTIVE: return true;
        case SiodbParser::K_INACTIVE: return false;
        default: throw std::invalid_argument(errorMessage);
    }
}

}  // namespace

const std::unordered_map<std::string, siodb::ColumnDataType>
        DBEngineRequestFactory::m_siodbDataTypeMap {
                {"INTEGER", siodb::COLUMN_DATA_TYPE_INT32},
                {"INT", siodb::COLUMN_DATA_TYPE_INT32},
                {"UINT", siodb::COLUMN_DATA_TYPE_UINT32},
                {"TINYINT", siodb::COLUMN_DATA_TYPE_INT8},
                {"TINYUINT", siodb::COLUMN_DATA_TYPE_UINT8},
                {"SMALLINT", siodb::COLUMN_DATA_TYPE_INT16},
                {"SMALLUINT", siodb::COLUMN_DATA_TYPE_UINT16},
                {"BIGINT", siodb::COLUMN_DATA_TYPE_INT64},
                {"BIGUINT", siodb::COLUMN_DATA_TYPE_UINT64},
                {"SMALLREAL", siodb::COLUMN_DATA_TYPE_FLOAT},
                {"REAL", siodb::COLUMN_DATA_TYPE_DOUBLE},
                {"FLOAT", siodb::COLUMN_DATA_TYPE_FLOAT},
                {"DOUBLE", siodb::COLUMN_DATA_TYPE_DOUBLE},
                {"TEXT", siodb::COLUMN_DATA_TYPE_TEXT},
                {"CHAR", siodb::COLUMN_DATA_TYPE_TEXT},
                {"VARCHAR", siodb::COLUMN_DATA_TYPE_TEXT},
                {"BLOB", siodb::COLUMN_DATA_TYPE_BINARY},
                {"TIMESTAMP", siodb::COLUMN_DATA_TYPE_TIMESTAMP},
        };

requests::DBEngineRequestPtr DBEngineRequestFactory::createRequest(antlr4::tree::ParseTree* node)
{
    if (!node) throw std::out_of_range("Statement doesn't exist");

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
        case SiodbParser::RuleUse_database_stmt: return createUseDatabaseRequest(node);
        case SiodbParser::RuleCreate_table_stmt: return createCreateTableRequest(node);
        case SiodbParser::RuleDrop_table_stmt: return createDropTableRequest(node);
        case SiodbParser::RuleAlter_table_stmt: {
            auto keyword = helpers::findTerminal(node, SiodbParser::K_RENAME);
            if (keyword) return createRenameTableRequest(node);

            keyword = helpers::findTerminal(node, SiodbParser::K_ADD);
            if (keyword) return createAddColumnRequest(node);

            keyword = helpers::findTerminal(node, SiodbParser::K_DROP);
            if (keyword) return createDropColumnRequest(node);

            throw std::runtime_error("ALTER TABLE unsupported transformation");
        }
        case SiodbParser::RuleCreate_index_stmt: return createCreateIndexRequest(node);
        case SiodbParser::RuleDrop_index_stmt: return createDropIndexRequest(node);
        case SiodbParser::RuleCreate_user_stmt: return createCreateUserRequest(node);
        case SiodbParser::RuleDrop_user_stmt: return createDropUserRequest(node);
        case SiodbParser::RuleAlter_user_stmt: {
            auto keywordTerminal = helpers::getTerminalType(node->children.at(3));
            if (keywordTerminal == SiodbParser::K_ADD)
                return createAddUserAccessKeyRequest(node);
            else if (keywordTerminal == SiodbParser::K_DROP)
                return createDropUserAccessKeyRequest(node);
            else if (keywordTerminal == SiodbParser::K_ALTER) {
                //  K_ALTER K_ACCESS KEY user_key_name SET user_key_option_list case
                return createAlterUserAccessKeyRequest(node);
            } else if (keywordTerminal == SiodbParser::K_SET) {
                // SET user_option case
                return createAlterUserRequest(node);
            }
            throw std::runtime_error("ALTER TABLE unsupported transformation");
        }
        default: {
            throw std::invalid_argument(
                    "Statement type " + std::to_string(statementType) + " is not supported");
        }
    }
}

// ----- internals -----

requests::DBEngineRequestPtr DBEngineRequestFactory::createSelectRequestForGeneralSelectStatement(
        [[maybe_unused]] antlr4::tree::ParseTree* node)
{
    throw std::runtime_error("SELECT: unsupported syntax");

    // TODO: Implement DBEngineRequestFactory::createSelectRequestForGeneralSelectStatement()

    // TODO: Capture WHERE values
    // TODO: Capture GROUP BY values
    // TODO: Capture HAVING values
    // TODO: Capture ORDER BY values
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createSelectRequestForSimpleSelectStatement(
        antlr4::tree::ParseTree* node)
{
    ExpressionFactory exprFactory(false);
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
                        throw std::runtime_error("SELECT: LIMIT does not contain expression");

                    if (node->children.size() + 2 > i
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
                        throw std::runtime_error("SELECT: OFFSET does not contain expression");
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

requests::DBEngineRequestPtr DBEngineRequestFactory::createSelectRequestForFactoredSelectStatement(
        antlr4::tree::ParseTree* node)
{
    // TODO: Implement DBEngineRequestFactory::createSelectRequestForFactoredSelectStatement()
    const auto selectCoreCount = std::count_if(
            node->children.cbegin(), node->children.cend(), [](const auto e) noexcept {
                return helpers::getNonTerminalType(e) == SiodbParser::RuleSelect_core;
            });

    if (selectCoreCount != 1) throw std::runtime_error("SELECT contains too much parts");

    // Now fallback to simple one
    return createSelectRequestForSimpleSelectStatement(node);

    // TODO: Capture WHERE values
    // TODO: Capture GROUP BY values
    // TODO: Capture HAVING values
    // TODO: Capture ORDER BY values
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createInsertRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) database = boost::to_upper_copy(databaseIdNode->getText());

    // Capture table ID
    std::string table;
    const auto tableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode)
        table = boost::to_upper_copy(tableIdNode->getText());
    else
        throw std::invalid_argument("INSERT missing table ID");

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
        if (!columnIdNode) throw std::runtime_error("INSERT missing column ID");

        columns.push_back(boost::to_upper_copy(columnIdNode->getText()));
    }

    if (!valuesFound) throw std::runtime_error("INSERT missing VALUES keyword");

    ExpressionFactory exprFactory(false);
    std::vector<std::vector<requests::ConstExpressionPtr>> values;
    if (!columns.empty()) values.reserve(columns.size());
    bool inValueGroup = false;
    while (index < node->children.size()) {
        const auto e = node->children[index++];
        // Handle opening and closing value group
        const auto terminalType = helpers::getTerminalType(e);
        if (terminalType == SiodbParser::OPEN_PAR) {
            if (inValueGroup)
                throw std::runtime_error("INSERT encoutered unexpected opening parenthesis");
            inValueGroup = true;
            values.emplace_back();
            continue;
        } else if (terminalType == SiodbParser::CLOSE_PAR) {
            if (!inValueGroup)
                throw std::runtime_error("INSERT encoutered unexpected closing parenthesis");
            inValueGroup = false;
            if (!columns.empty() && values.back().size() != columns.size()) {
                throw std::runtime_error(
                        "INSERT number of values doesn't match to number of columns");
            }
            continue;
        }
        if (helpers::getNonTerminalType(e) != SiodbParser::RuleExpr) continue;
        values.back().push_back(exprFactory.createExpression(e));
    }

    if (inValueGroup) throw std::runtime_error("INSERT values list is not closed");

    if (values.empty()) throw std::runtime_error("INSERT missing values");

    return std::make_unique<requests::InsertRequest>(
            std::move(database), std::move(table), std::move(columns), std::move(values));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createUpdateRequest(
        antlr4::tree::ParseTree* node)
{
    const ExpressionFactory exprFactory(true);
    std::string database, tableName, tableAlias;
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
                if (databaseNameNode) database = boost::to_upper_copy(databaseNameNode->getText());

                // Capture table name
                const auto tableNameNode = helpers::findTerminal(
                        e, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
                if (tableNameNode)
                    tableName = boost::to_upper_copy(tableNameNode->getText());
                else
                    throw std::invalid_argument("UPDATE missing table ID");

                // Capture table alias
                const auto tableAliasNode = helpers::findTerminal(
                        e, SiodbParser::RuleTable_alias, SiodbParser::IDENTIFIER);
                if (tableAliasNode) tableAlias = boost::to_upper_copy(tableAliasNode->getText());

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
                                    throw std::runtime_error(
                                            "UPDATE, WHERE does not contain expression");
                                }
                                where = exprFactory.createExpression(node->children[i]);
                            } else
                                throw std::runtime_error("UPDATE SET statement is broken");

                            break;
                        }

                        // SET statement should have at least one column = expr
                        if (i + 2 >= node->children.size())
                            throw std::runtime_error("UPDATE: missing expression in SET");

                        /// --------- Parse column/where ---------
                        const auto columnNode = node->children[i];

                        auto nonTerminalType = helpers::getNonTerminalType(columnNode);
                        if (nonTerminalType == SiodbParser::RuleColumn_name) {
                            std::string column;
                            if (columnNode->children.size() == 1) {
                                // Only column name is expected
                                column = boost::to_upper_copy(columnNode->children[0]->getText());
                            } else {
                                // Normally should never happen
                                throw std::runtime_error("UPDATE SET statement is broken");
                            }
                            columns.emplace_back(std::string(), std::move(column));
                        } else
                            throw std::runtime_error("UPDATE: SET Expression column not found");

                        /// --------- Parse '=' ---------
                        const auto assignNode = node->children[i + 1];
                        terminalType = helpers::getTerminalType(assignNode);
                        if (terminalType != SiodbParser::ASSIGN)
                            throw std::runtime_error("UPDATE missing = in SET");

                        /// --------- Parse value ---------
                        const auto valueExpr = node->children[i + 2];
                        nonTerminalType = helpers::getNonTerminalType(valueExpr);
                        if (nonTerminalType == SiodbParser::RuleExpr) {
                            values.push_back(exprFactory.createExpression(valueExpr));
                        } else
                            throw std::runtime_error("UPDATE missing SET value");

                        // +4, for Column, '=', expr', ',' + 3 for new set expr
                        if (i + 7 <= node->children.size()) {
                            const auto commaNode = node->children[i + 3];
                            terminalType = helpers::getTerminalType(commaNode);
                            if (terminalType != SiodbParser::COMMA)
                                throw std::runtime_error("UPDATE missing comma separator");
                            i += 4;
                        } else
                            i += 3;

                        ++updateValueCount;
                    }
                } else if (terminalType == SiodbParser::K_UPDATE)
                    continue;
                else
                    throw std::runtime_error("UPDATE: Expression is invalid or unsupported");

                break;
            }
            default: continue;
        }
    }

    if (columns.empty()) throw std::runtime_error("UPDATE: Missing columns");

    if (columns.size() != values.size())
        throw std::runtime_error("UPDATE: Column count is not equal to the value count");

    return std::make_unique<requests::UpdateRequest>(std::move(database),
            requests::SourceTable(std::move(tableName), std::move(tableAlias)), std::move(columns),
            std::move(values), std::move(where));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createDeleteRequest(
        antlr4::tree::ParseTree* node)
{
    std::string database;
    std::string tableName;
    std::string tableAlias;
    requests::ConstExpressionPtr where;

    for (std::size_t i = 0; i < node->children.size(); ++i) {
        const auto e = node->children[i];
        const auto nonTerminalType = helpers::getNonTerminalType(e);
        switch (nonTerminalType) {
            case SiodbParser::RuleAliased_qualified_table_name: {
                // Capture Database name
                const auto databaseNameNode = helpers::findTerminal(
                        e, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
                if (databaseNameNode) database = boost::to_upper_copy(databaseNameNode->getText());

                // Capture table name
                const auto tableNameNode = helpers::findTerminal(
                        e, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
                if (tableNameNode)
                    tableName = boost::to_upper_copy(tableNameNode->getText());
                else
                    throw std::invalid_argument("DELETE missing table ID");

                // Capture table alias
                const auto tableAliasNode = helpers::findTerminal(
                        e, SiodbParser::RuleTable_alias, SiodbParser::IDENTIFIER);
                if (tableAliasNode) tableAlias = boost::to_upper_copy(tableAliasNode->getText());
                break;
            }
            case kInvalidNodeType: {
                const auto terminalType = helpers::getTerminalType(e);
                if (terminalType == SiodbParser::K_WHERE) {
                    ++i;
                    if (i >= node->children.size())
                        throw std::runtime_error("DELETE, WHERE does not contain expression");

                    ExpressionFactory exprFactory(true);
                    where = exprFactory.createExpression(node->children[i]);
                }
                break;
            };
            default: continue;
        }
    }

    return std::make_unique<requests::DeleteRequest>(std::move(database),
            requests::SourceTable(std::move(tableName), std::move(tableAlias)), std::move(where));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createBeginTransactionRequest(
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
    if (transactionIdNode) transaction = boost::to_upper_copy(transactionIdNode->getText());

    return std::make_unique<requests::BeginTransactionRequest>(
            transactionType, std::move(transaction));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createCommitTransactionRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture transaction ID
    std::string transaction;
    const auto transactionIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (transactionIdNode) transaction = boost::to_upper_copy(transactionIdNode->getText());

    return std::make_unique<requests::CommitTransactionRequest>(std::move(transaction));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createRollbackTransactionRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture transaction ID
    std::string transaction;
    const auto transactionIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (transactionIdNode) transaction = boost::to_upper_copy(transactionIdNode->getText());

    // Capture savepoint ID
    std::string savepoint;
    const auto savepointIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (savepointIdNode) savepoint = boost::to_upper_copy(savepointIdNode->getText());

    return std::make_unique<requests::RollbackTransactionRequest>(
            std::move(transaction), std::move(savepoint));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createSavepointRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture savepoint ID
    std::string savepoint;
    const auto savepointIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (savepointIdNode)
        savepoint = boost::to_upper_copy(savepointIdNode->getText());
    else
        throw std::runtime_error("SAVEPOINT missing savepoint ID");

    return std::make_unique<requests::SavepointRequest>(std::move(savepoint));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createReleaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture savepoint ID
    std::string savepoint;
    const auto savepointIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTransaction_name, SiodbParser::IDENTIFIER);
    if (savepointIdNode)
        savepoint = boost::to_upper_copy(savepointIdNode->getText());
    else
        throw std::runtime_error("RELEASE missing savepoint ID");

    return std::make_unique<requests::SavepointRequest>(std::move(savepoint));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createAttachDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database UUID
    Uuid databaseUuid;
    const auto uuidNode =
            helpers::findTerminal(node, SiodbParser::RuleExpr, SiodbParser::STRING_LITERAL);
    if (uuidNode) {
        auto s = uuidNode->getText();
        // Remove quotes
        s.pop_back();
        s.erase(0, 1);
        databaseUuid = boost::uuids::string_generator()(s);
    } else
        throw std::invalid_argument("ATTACH DATABASE missing database UUID");

    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode)
        database = boost::to_upper_copy(databaseIdNode->getText());
    else
        throw std::invalid_argument("ATTACH DATABASE missing database ID");

    return std::make_unique<requests::AttachDatabaseRequest>(
            std::move(databaseUuid), std::move(database));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createDetachDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode)
        database = boost::to_upper_copy(databaseIdNode->getText());
    else
        throw std::invalid_argument("DETACH DATABASE missing database ID");

    // Check for "IF EXISTS" clause
    const auto ifNode = helpers::findTerminal(node, SiodbParser::K_IF);
    const bool ifExists = (ifNode != nullptr);

    return std::make_unique<requests::DetachDatabaseRequest>(std::move(database), ifExists);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createCreateDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Normally should never happen
    if (node->children.size() < 3)
        throw std::invalid_argument("CREATE DATABASE request is malformed");

    // Database node could be 2 or 3 ( in case of CREATE TEMPORARY DATABASE <name>)
    bool temporary = false;
    size_t databaseNodeIndex = 0;
    if (helpers::getNonTerminalType(node->children[2]) == SiodbParser::RuleDatabase_name) {
        databaseNodeIndex = 2;
    } else if (node->children.size() > 3
               && helpers::getNonTerminalType(node->children[3])
                          == SiodbParser::RuleDatabase_name) {
        databaseNodeIndex = 3;
        temporary = true;
    } else
        throw std::invalid_argument("CREATE DATABASE missing database name");

    auto database = boost::to_upper_copy(node->children[databaseNodeIndex]->getText());

    //  <name> + WITH + <List of options>
    requests::ConstExpressionPtr cipherId, cipherKeySeed;
    if (node->children.size() == databaseNodeIndex + 3) {
        if (helpers::getNonTerminalType(node->children[databaseNodeIndex + 2])
                != SiodbParser::RuleCreate_database_option_list)
            throw std::invalid_argument("CREATE DATABASE missing option list");

        auto optionsListNode = node->children[databaseNodeIndex + 2];
        // skip comma (option ',' option ... )
        for (std::size_t i = 0; i < optionsListNode->children.size(); i += 2) {
            auto optionNode = optionsListNode->children[i];
            ExpressionFactory exprFactory(false);
            switch (helpers::getTerminalType(optionNode->children.at(0))) {
                case SiodbParser::K_CIPHER_ID: {
                    cipherId = exprFactory.createExpression(optionNode->children.at(2));
                    break;
                }
                case SiodbParser::K_CIPHER_KEY_SEED: {
                    cipherKeySeed = exprFactory.createExpression(optionNode->children.at(2));
                    break;
                }
                default: throw std::invalid_argument("CREATE DATABASE invalid option");
            }
        }
    } else if (node->children.size() != databaseNodeIndex + 1)
        throw std::invalid_argument("CREATE DATABASE request is malformed");

    return std::make_unique<requests::CreateDatabaseRequest>(
            std::move(database), temporary, std::move(cipherId), std::move(cipherKeySeed));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createDropDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode)
        database = boost::to_upper_copy(databaseIdNode->getText());
    else
        throw std::invalid_argument("DROP DATABASE missing database ID");

    // Check for "IF EXISTS" clause
    const auto ifNode = helpers::findTerminal(node, SiodbParser::K_IF);
    const bool ifExists = (ifNode != nullptr);

    return std::make_unique<requests::DropDatabaseRequest>(std::move(database), ifExists);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createUseDatabaseRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode)
        database = boost::to_upper_copy(databaseIdNode->getText());
    else
        throw std::invalid_argument("USE DATABASE missing database ID");

    return std::make_unique<requests::UseDatabaseRequest>(std::move(database));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createCreateTableRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode = helpers::findNonTerminal(node, SiodbParser::RuleDatabase_name);
    if (databaseIdNode) database = boost::to_upper_copy(databaseIdNode->getText());

    // Capture table ID
    std::string table;
    const auto tableIdNode = helpers::findNonTerminal(node, SiodbParser::RuleTable_name);
    if (tableIdNode)
        table = boost::to_upper_copy(tableIdNode->getText());
    else
        throw std::invalid_argument("CREATE TABLE missing table ID");

    // Capture column definitions
    std::vector<requests::ColumnDefinition> columns;
    for (const auto columnDefNode : node->children) {
        if (helpers::getNonTerminalType(columnDefNode) != SiodbParser::RuleColumn_def) continue;

        // Find column ID
        const auto columnIdNode = helpers::findTerminal(
                columnDefNode, SiodbParser::RuleColumn_name, SiodbParser::IDENTIFIER);
        if (!columnIdNode) throw std::invalid_argument("CREATE TABLE missing column ID");
        auto columnName = boost::to_upper_copy(columnIdNode->getText());

        // Find column data type
        const auto typeNameNode =
                helpers::findNonTerminal(columnDefNode, SiodbParser::RuleType_name);
        if (typeNameNode == nullptr)
            throw std::invalid_argument("CREATE TABLE missing column data type");

        // Capture data type
        std::ostringstream typeName;
        std::size_t idNodeCount = 0;
        for (const auto childNode : typeNameNode->children) {
            const auto idNode = helpers::findTerminal(childNode, SiodbParser::IDENTIFIER);
            if (idNode) {
                if (idNodeCount > 0) typeName << ' ';
                typeName << boost::to_upper_copy(idNode->getText());
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
                    const auto s = boost::to_upper_copy(idNode->getText());
                    constraintName = s.substr(1, s.length() - 2);
                    break;
                }
                idNode = helpers::findTerminal(constraintNameNode, SiodbParser::IDENTIFIER);
                if (idNode) {
                    constraintName = boost::to_upper_copy(idNode->getText());
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
                // TODO(102): Parse columns
                std::vector<std::string> columns;
                constraints.emplace_back(std::make_unique<requests::UniqueConstraint>(
                        std::move(constraintName), std::move(columns)));
                continue;
            }

            // Check for DEFAULT constraint
            terminal = helpers::findTerminal(constraintNode, SiodbParser::K_DEFAULT);
            if (terminal) {
                // TODO(102): Parse default value
                requests::ExpressionPtr defaultValue;
                constraints.emplace_back(std::make_unique<requests::DefaultValueConstraint>(
                        std::move(constraintName), std::move(defaultValue)));
                continue;
            }

            // Check for PRIMARY KEY constraint
            terminal = helpers::findTerminal(constraintNode, SiodbParser::K_PRIMARY);
            if (terminal) {
                throw std::invalid_argument(
                        "CREATE TABLE: PRIMARY KEY constraint is not supported in the Siodb");
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
                // TODO(102): Parse CHECK expression
                std::string expression;
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
            throw std::invalid_argument("CREATE TABLE: Unsupported constraint type");
        }

        // Add column record
        columns.emplace_back(std::move(columnName), columnDataType, kDefaultDataFileDataAreaSize,
                std::move(constraints));
    }

    return std::make_unique<requests::CreateTableRequest>(
            std::move(database), std::move(table), std::move(columns));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createDropTableRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) database = boost::to_upper_copy(databaseIdNode->getText());

    // Capture table ID
    std::string table;
    const auto tableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode)
        table = boost::to_upper_copy(tableIdNode->getText());
    else
        throw std::invalid_argument("DROP TABLE missing table ID");

    // Check for "IF EXISTS" clause
    const auto ifNode = helpers::findTerminal(node, SiodbParser::K_IF);
    const bool ifExists = (ifNode != nullptr);

    return std::make_unique<requests::DropTableRequest>(
            std::move(database), std::move(table), ifExists);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createRenameTableRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) database = boost::to_upper_copy(databaseIdNode->getText());

    // Capture old table ID
    std::string oldTable;
    const auto oldTableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (oldTableIdNode)
        oldTable = boost::to_upper_copy(oldTableIdNode->getText());
    else
        throw std::invalid_argument("ALTER TABLE RENAME TO missing table ID");

    // Capture new table ID
    std::string newTable;
    const auto newTableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleNew_table_name, SiodbParser::IDENTIFIER);
    if (newTableIdNode)
        newTable = boost::to_upper_copy(newTableIdNode->getText());
    else
        throw std::invalid_argument("ALTER TABLE RENAME TO missing new table ID");

    // Check for "IF EXISTS" clause
    const auto ifNode = helpers::findTerminal(node, SiodbParser::K_IF);
    const bool ifExists = (ifNode != nullptr);

    return std::make_unique<requests::RenameTableRequest>(
            std::move(database), std::move(oldTable), std::move(newTable), ifExists);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createAddColumnRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) database = boost::to_upper_copy(databaseIdNode->getText());

    // Capture table ID
    std::string table;
    const auto tableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode)
        table = boost::to_upper_copy(tableIdNode->getText());
    else
        throw std::invalid_argument("ALTER TABLE ADD COLUMN missing table ID");

    // Find column ID
    const auto columnIdNode =
            helpers::findTerminal(node, SiodbParser::RuleColumn_name, SiodbParser::IDENTIFIER);
    if (!columnIdNode) throw std::invalid_argument("ALTER TABLE ADD COLUMN missing column ID");
    auto columnName = boost::to_upper_copy(columnIdNode->getText());

    // Find column data type
    const auto typeNameNode =
            helpers::findTerminal(node, SiodbParser::RuleType_name, SiodbParser::IDENTIFIER);
    if (typeNameNode == nullptr)
        throw std::invalid_argument("ALTER TABLE ADD COLUMN missing column data type");
    const auto columnDataType = getColumnDataType(boost::to_upper_copy(typeNameNode->getText()));

    // Fill new column info
    requests::ColumnDefinition column(
            std::move(columnName), columnDataType, kDefaultDataFileDataAreaSize, {});

    return std::make_unique<requests::AddColumnRequest>(
            std::move(database), std::move(table), std::move(column));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createDropColumnRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) database = boost::to_upper_copy(databaseIdNode->getText());

    // Capture table ID
    std::string table;
    const auto tableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode)
        table = boost::to_upper_copy(tableIdNode->getText());
    else
        throw std::invalid_argument("ALTER TABLE DROP COLUMN missing table ID");

    // Capture column ID
    std::string column;
    const auto columnIdNode =
            helpers::findTerminal(node, SiodbParser::RuleColumn_name, SiodbParser::IDENTIFIER);
    if (columnIdNode)
        column = boost::to_upper_copy(columnIdNode->getText());
    else
        throw std::invalid_argument("ALTER TABLE DROP COLUMN missing table ID");

    // Check for "IF EXISTS" clause
    const auto ifNode = helpers::findTerminal(node, SiodbParser::K_IF);
    const bool ifExists = (ifNode != nullptr);

    return std::make_unique<requests::DropColumnRequest>(
            std::move(database), std::move(table), std::move(column), ifExists);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createCreateIndexRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) database = boost::to_upper_copy(databaseIdNode->getText());

    // Capture index name
    std::string index;
    const auto indexNameNode =
            helpers::findTerminal(node, SiodbParser::RuleIndex_name, SiodbParser::IDENTIFIER);
    if (indexNameNode)
        index = boost::to_upper_copy(indexNameNode->getText());
    else
        throw std::invalid_argument("CREATE INDEX missing index name");

    // Capture table ID
    std::string table;
    const auto tableIdNode =
            helpers::findTerminal(node, SiodbParser::RuleTable_name, SiodbParser::IDENTIFIER);
    if (tableIdNode)
        table = boost::to_upper_copy(tableIdNode->getText());
    else
        throw std::invalid_argument("CREATE INDEX missing table ID");

    // Capture column definitions
    std::vector<requests::IndexColumnDefinition> columns;
    for (const auto e : node->children) {
        if (helpers::getNonTerminalType(e) != SiodbParser::RuleIndexed_column) continue;

        // Find column ID
        const auto columnIdNode =
                helpers::findTerminal(e, SiodbParser::RuleColumn_name, SiodbParser::IDENTIFIER);
        if (!columnIdNode) throw std::invalid_argument("CREATE INDEX missing column ID");

        // Find sort order
        const bool sortDescending = helpers::findTerminal(e, SiodbParser::K_DESC) != nullptr;

        // Add column record
        columns.emplace_back(boost::to_upper_copy(columnIdNode->getText()), sortDescending);
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

requests::DBEngineRequestPtr DBEngineRequestFactory::createDropIndexRequest(
        antlr4::tree::ParseTree* node)
{
    // Capture database ID
    std::string database;
    const auto databaseIdNode =
            helpers::findTerminal(node, SiodbParser::RuleDatabase_name, SiodbParser::IDENTIFIER);
    if (databaseIdNode) database = boost::to_upper_copy(databaseIdNode->getText());

    // Capture index name
    std::string index;
    const auto indexNameNode =
            helpers::findTerminal(node, SiodbParser::RuleIndex_name, SiodbParser::IDENTIFIER);
    if (indexNameNode)
        index = boost::to_upper_copy(indexNameNode->getText());
    else
        throw std::invalid_argument("DROP INDEX missing index name");

    // Check for "IF EXISTS" clause
    const auto ifNode = helpers::findTerminal(node, SiodbParser::K_IF);
    const bool ifExists = ifNode != nullptr;

    return std::make_unique<requests::DropIndexRequest>(
            std::move(database), std::move(index), ifExists);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createCreateUserRequest(
        antlr4::tree::ParseTree* node)
{
    // Normally should never happen
    if (node->children.size() < 3) throw std::invalid_argument("CREATE USER request is malformed");

    // Get node text as is without 'GetAnyText' call.
    auto name = boost::to_upper_copy(node->children[2]->getText());
    std::string realName;
    bool active = true;

    //  <name> + WITH + <List of options>
    if (node->children.size() > 4) {
        if (helpers::getNonTerminalType(node->children[4]) != SiodbParser::RuleUser_option_list)
            throw std::invalid_argument("CREATE USER missing options list");

        const auto optionsListNode = node->children[4];
        // skip comma (option ',' option ... )
        for (std::size_t i = 0; i < optionsListNode->children.size(); i += 2) {
            const auto optionNode = optionsListNode->children[i];
            switch (helpers::getTerminalType(optionNode->children.at(0))) {
                case SiodbParser::K_STATE: {
                    active = parseStateNode(
                            optionNode->children.at(2), "CREATE USER invalid user state");
                    break;
                }
                case SiodbParser::K_REAL_NAME: {
                    realName = optionNode->children.at(2)->getText();
                    // Remove quotes
                    realName.pop_back();
                    realName.erase(0, 1);
                    break;
                }
                default: throw std::invalid_argument("CREATE USER invalid option");
            }
        }
    }

    return std::make_unique<requests::CreateUserRequest>(
            std::move(name), std::move(realName), active);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createDropUserRequest(
        antlr4::tree::ParseTree* node)
{
    // Normally should never happen
    if (node->children.size() < 3) throw std::invalid_argument("DROP USER request is malformed");

    // Get node text as is without 'GetAnyText' call.
    auto name = boost::to_upper_copy(node->children[2]->getText());
    return std::make_unique<requests::DropUserRequest>(std::move(name), false);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createAlterUserRequest(
        antlr4::tree::ParseTree* node)
{
    // Normally should never happen
    if (node->children.size() < 5)
        throw std::invalid_argument("ALTER USER <username> SET <option_list> request is malformed");

    // Get node text as is without 'GetAnyText' call.
    auto name = boost::to_upper_copy(node->children[2]->getText());

    std::optional<std::string> realName;
    std::optional<bool> active;

    const auto optionsListNode = node->children[4];
    // skip comma (option ',' option ... )
    for (std::size_t i = 0; i < optionsListNode->children.size(); i += 2) {
        const auto optionNode = optionsListNode->children[i];
        switch (helpers::getTerminalType(optionNode->children.at(0))) {
            case SiodbParser::K_REAL_NAME: {
                auto s = optionNode->children.at(2)->getText();
                // Remove quotes
                s.pop_back();
                s.erase(0, 1);
                realName = std::move(s);
                break;
            }
            case SiodbParser::K_STATE: {
                active = parseStateNode(
                        optionNode->children.at(2), "ALTER USER: invalid user state");
                break;
            }
            default: throw std::invalid_argument("ALTER USER: invalid option");
        }
    }

    return std::make_unique<requests::AlterUserRequest>(
            std::move(name), std::move(realName), std::move(active));
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createAddUserAccessKeyRequest(
        antlr4::tree::ParseTree* node)
{
    // Get node text as is without 'GetAnyText' call.
    auto userName = boost::to_upper_copy(node->children.at(2)->getText());
    auto keyName = boost::to_upper_copy(node->children.at(6)->getText());
    auto keyText = node->children.at(7)->getText();
    // Remove quotes
    keyText.pop_back();
    keyText.erase(0, 1);

    bool active = true;

    if (node->children.size() > 8) {
        auto optionsListNode = node->children[8];
        for (std::size_t i = 0; i < optionsListNode->children.size(); i += 2) {
            const auto optionNode = optionsListNode->children[i];
            switch (helpers::getTerminalType(optionNode->children.at(0))) {
                case SiodbParser::K_STATE: {
                    active = parseStateNode(
                            optionNode->children.at(2), "ALTER USER ADD KEY: invalid key state");
                    break;
                }
                default: throw std::invalid_argument("ALTER USER ADD KEY: inavlid option");
            }
        }
    }

    return std::make_unique<requests::AddUserAccessKeyRequest>(
            std::move(userName), std::move(keyName), std::move(keyText), active);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createDropUserAccessKeyRequest(
        antlr4::tree::ParseTree* node)
{
    // Get node text as is without 'GetAnyText' call.
    auto userName = boost::to_upper_copy(node->children.at(2)->getText());
    auto keyName = boost::to_upper_copy(node->children.at(6)->getText());
    return std::make_unique<requests::DropUserAccessKeyRequest>(
            std::move(userName), std::move(keyName), false);
}

requests::DBEngineRequestPtr DBEngineRequestFactory::createAlterUserAccessKeyRequest(
        antlr4::tree::ParseTree* node)
{
    // Get node text as is without 'GetAnyText' call.
    auto userName = boost::to_upper_copy(node->children.at(2)->getText());
    auto keyName = boost::to_upper_copy(node->children.at(6)->getText());
    std::optional<bool> active;

    auto optionsListNode = node->children.at(8);
    for (std::size_t i = 0; i < optionsListNode->children.size(); i += 2) {
        const auto optionNode = optionsListNode->children[i];
        switch (helpers::getTerminalType(optionNode->children.at(0))) {
            case SiodbParser::K_STATE: {
                active = parseStateNode(
                        optionNode->children.at(2), "ALTER USER ALTER KEY: invalid key state");
                break;
            }
            default: throw std::invalid_argument("ALTER USER ALTER KEY: inavlid option");
        }
    }

    return std::make_unique<requests::AlterUserAccessKey>(
            std::move(userName), std::move(keyName), std::move(active));
}

siodb::ColumnDataType DBEngineRequestFactory::getColumnDataType(const std::string& typeName)
{
    const auto it = m_siodbDataTypeMap.find(typeName);
    if (it != m_siodbDataTypeMap.end()) return it->second;
    throw std::invalid_argument("Type '" + typeName + "' is not supported");
}

requests::ResultExpression DBEngineRequestFactory::createResultExpression(
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
                boost::to_upper_copy(node->children[0]->getText()));
    }
    // case: expr ( K_AS? column_alias)?
    else if (childrenCount > 0
             && helpers::getNonTerminalType(node->children[0]) == SiodbParser::RuleExpr) {
        ExpressionFactory exprFactory(true);
        expression = exprFactory.createExpression(node->children[0]);

        if (childrenCount > 1
                && helpers::getNonTerminalType(node->children.back())
                           == SiodbParser::RuleColumn_alias) {
            alias = boost::to_upper_copy(boost::to_upper_copy(node->children.back()->getText()));
        }
    } else
        throw std::invalid_argument("Result column node is invalid");
    return requests::ResultExpression(std::move(expression), std::move(alias));
}

void DBEngineRequestFactory::parseSelectCore(antlr4::tree::ParseTree* node, std::string& database,
        std::vector<requests::SourceTable>& tables,
        std::vector<requests::ResultExpression>& columns, requests::ConstExpressionPtr& where)
{
    std::size_t i = 0;
    for (; i < node->children.size(); ++i) {
        const auto e = node->children[i];
        const auto nonTerminalType = helpers::getNonTerminalType(e);
        switch (nonTerminalType) {
            case SiodbParser::RuleResult_column: {
                columns.push_back(createResultExpression(e));
                break;
            }
            case SiodbParser::RuleTable_or_subquery: {
                // Capture database ID
                const auto databaseIdNode =
                        helpers::findNonTerminal(e, SiodbParser::RuleDatabase_name);
                if (databaseIdNode) database = boost::to_upper_copy(databaseIdNode->getText());

                // Capture table ID
                const auto tableIdNode = helpers::findNonTerminal(e, SiodbParser::RuleTable_name);
                if (tableIdNode) {
                    auto tableName = boost::to_upper_copy(tableIdNode->getText());
                    // Capture table alias
                    std::string tableAlias;
                    const auto tableAliasIdNode =
                            helpers::findNonTerminal(e, SiodbParser::RuleTable_alias);
                    if (tableAliasIdNode)
                        tableAlias = boost::to_upper_copy(tableAliasIdNode->getText());
                    tables.emplace_back(std::move(tableName), std::move(tableAlias));
                } else
                    throw std::invalid_argument("SELECT: missing table ID");
                break;
            }
            case kInvalidNodeType: {
                const auto terminalType = helpers::getTerminalType(e);
                if (terminalType == SiodbParser::K_WHERE) {
                    ++i;
                    if (i >= node->children.size())
                        throw std::runtime_error("SELECT: WHERE does not contain expression");

                    ExpressionFactory exprFactory(true);
                    where = exprFactory.createExpression(node->children[i]);
                }
                break;
            };
            default: {
                throw std::runtime_error("SELECT: query contains unsupported non-terminal of type "
                                         + std::to_string(nonTerminalType));
            }
        }
    }
}

}  // namespace siodb::iomgr::dbengine::parser
