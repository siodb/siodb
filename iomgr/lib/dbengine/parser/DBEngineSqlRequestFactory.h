// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "DBEngineSqlRequest.h"
#include "SqlParser.h"

namespace siodb::iomgr::dbengine::parser {

/** DBEngineSqlRequestFactory produces DB Engine requests for the SQL statements. */
class DBEngineSqlRequestFactory {
public:
    /**
     * Initializes object of class DBEngineSqlRequestFactory.
     * @param parser Parser object.
     */
    explicit DBEngineSqlRequestFactory(SqlParser& parser) noexcept
        : m_parser(parser)
    {
    }

    /**
     * Creates database engine request from a given statement.
     * @param node A statement node.
     * @return Database engine request object filled with parsed data.
     */
    requests::DBEngineRequestPtr createSqlRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates database engine request from a given parsed statement.
     * @param index Statement index.
     * @return Database engine request object filled with parsed data.
     */
    requests::DBEngineRequestPtr createSqlRequest(size_t index = 0);

private:
    /**
     * Creates SELECT request.
     * @param node Parse tree node with SQL statement.
     * @param tag Tag idicating general form of SELECT statement.
     * @return SELECT request.
     */
    requests::DBEngineRequestPtr createSelectRequestForGeneralSelectStatement(
            antlr4::tree::ParseTree* node);

    /**
     * Creates SELECT request.
     * @param node Parse tree node with SQL statement.
     * @param tag Tag idicating simple form of SELECT statement.
     * @return SELECT request.
     */
    requests::DBEngineRequestPtr createSelectRequestForSimpleSelectStatement(
            antlr4::tree::ParseTree* node);

    /**
     * Creates SELECT request.
     * @param node Parse tree node with SQL statement.
     * @param tag Tag idicating factored form of SELECT statement.
     * @return SELECT request.
     */
    requests::DBEngineRequestPtr createSelectRequestForFactoredSelectStatement(
            antlr4::tree::ParseTree* node);

    /**
     * Creates INSERT request.
     * @param node Parse tree node with SQL statement.
     * @return INSERT request.
     */
    requests::DBEngineRequestPtr createInsertRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates UPDATE request.
     * @param node Parse tree node with SQL statement.
     * @return UPDATE request.
     */
    requests::DBEngineRequestPtr createUpdateRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DELETE request.
     * @param node Parse tree node with SQL statement.
     * @return DELETE request.
     */
    requests::DBEngineRequestPtr createDeleteRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates BEGIN TRANSACTION request.
     * @param node Parse tree node with SQL statement.
     * @return BEGIN TRANSACTION request.
     */
    requests::DBEngineRequestPtr createBeginTransactionRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates COMMIT TRANSACTION request.
     * @param node Parse tree node with SQL statement.
     * @return COMMIT TRANSACTION request.
     */
    requests::DBEngineRequestPtr createCommitTransactionRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ROLLBACK TRANSACTION request.
     * @param node Parse tree node with SQL statement.
     * @return ROLLBACK TRANSACTION request.
     */
    requests::DBEngineRequestPtr createRollbackTransactionRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates SAVEPOINT request.
     * @param node Parse tree node with SQL statement.
     * @return SAVEPOINT request.
     */
    requests::DBEngineRequestPtr createSavepointRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates RELEASE request.
     * @param node Parse tree node with SQL statement.
     * @return RELEASE request.
     */
    requests::DBEngineRequestPtr createReleaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ATTACH DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return ATTACH DATABASE request.
     */
    requests::DBEngineRequestPtr createAttachDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DETACH DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return DETACH DATABASE request.
     */
    requests::DBEngineRequestPtr createDetachDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates CREATE DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return CREATE DATABASE request.
     */
    requests::DBEngineRequestPtr createCreateDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DROP DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return DROP DATABASE request.
     */
    requests::DBEngineRequestPtr createDropDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER DATABASE RENAME TO request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER DATABASE RENAME TO request.
     */
    requests::DBEngineRequestPtr createRenameDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER DATABASE SET attributes request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER DATABASE SET attributes request.
     */
    requests::DBEngineRequestPtr createSetDatabaseAttributesRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates USE DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return USE DATABASE request.
     */
    requests::DBEngineRequestPtr createUseDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates CREATE TABLE request.
     * @param node Parse tree node with SQL statement.
     * @return CREATE TABLE request.
     */
    requests::DBEngineRequestPtr createCreateTableRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DROP TABLE request.
     * @param node Parse tree node with SQL statement.
     * @return DROP TABLE request.
     */
    requests::DBEngineRequestPtr createDropTableRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE RENAME TO request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE RENAME TO request.
     */
    requests::DBEngineRequestPtr createRenameTableRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE SET attributes request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE SET attributes request.
     */
    requests::DBEngineRequestPtr createSetTableAttributesRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates CREATE INDEX request.
     * @param node Parse tree node with SQL statement.
     * @return CREATE INDEX request.
     */
    requests::DBEngineRequestPtr createCreateIndexRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DROP INDEX request.
     * @param node Parse tree node with SQL statement.
     * @return DROP INDEX request.
     */
    requests::DBEngineRequestPtr createDropIndexRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE ADD COLUMN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE ADD COLUMN request.
     */
    requests::DBEngineRequestPtr createAddColumnRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE DROP COLUMN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE DROP COLUMN request.
     */
    requests::DBEngineRequestPtr createDropColumnRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE ALTER COLUMN RENAME TO request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE ALTER COLUMN RENAME TO request.
     */
    requests::DBEngineRequestPtr createRenameColumnRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE ALTER COLUMN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE ALTER COLUMN request.
     */
    requests::DBEngineRequestPtr createRedefineColumnRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates CREATE USER request.
     * @param node Parse tree node with SQL statement.
     * @return CREATE USER request.
     */
    requests::DBEngineRequestPtr createCreateUserRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DROP USER request.
     * @param node Parse tree node with SQL statement.
     * @return DROP USER request.
     */
    requests::DBEngineRequestPtr createDropUserRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER SET attributes request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER SET attributes request.
     */
    requests::DBEngineRequestPtr createSetUserAttributesRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ADD ACCESS KEY request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ADD ACCESS KEY request.
     */
    requests::DBEngineRequestPtr createAddUserAccessKeyRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER DROP ACCESS KEY request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER DROP ACCESS KEY request.
     */
    requests::DBEngineRequestPtr createDropUserAccessKeyRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ALTER ACCESS KEY SET ATTR_LIST request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ALTER ACCESS KEY SET ATTR_LIST request.
     */
    requests::DBEngineRequestPtr createSetUserAccessKeyAttributesRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ALTER ACCES KEY RENAME IF EXISTS request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ALTER ACCESS KEY IF EXISTS request.
     */
    requests::DBEngineRequestPtr createRenameUserAccessKeyRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ADD TOKEN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ADD TOKEN request.
     */
    requests::DBEngineRequestPtr createAddUserTokenRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER DROP TOKEN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER DROP TOKEN request.
     */
    requests::DBEngineRequestPtr createDropUserTokenRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ALTER TOKEN SET attributes request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ALTER TOKEN SET attributes request.
     */
    requests::DBEngineRequestPtr createSetUserTokenAttributesRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ALTER TOKEN RENAME IF EXISTS request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ALTER TOKEN RENAME IF EXISTS request.
     */
    requests::DBEngineRequestPtr createRenameUserTokenRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates CHECK TOKEN request.
     * @param node Parse tree node with SQL statement.
     * @return CHECK TOKEN request.
     */
    requests::DBEngineRequestPtr createCheckUserTokenRequest(antlr4::tree::ParseTree* node);

    /**
     * Parses SelectCore.
     * @param node Parse tree node with SelectCore node.
     * @param[out] database Database name.
     * @param[out] tables List of tables.
     * @param[out] columns List of columns.
     * @param[out] where WHERE condition.
     */
    void parseSelectCore(antlr4::tree::ParseTree* node, std::string& database,
            std::vector<requests::SourceTable>& tables,
            std::vector<requests::ResultExpression>& columns, requests::ConstExpressionPtr& where);

    /**
     * Converts given type name into Siodb column data type.
     * @param typeName Type name.
     * @return Siodb column data type.
     * @throw std::invalid_argument if given data type name doesn't represent a valid type name.
     */
    siodb::ColumnDataType getColumnDataType(const std::string& typeName);

    /**
     * Creates a requests::ResultExpression from a selected node.
     * @param node Parse tree node with result_column statement.
     * @return requests::ResultExpression object.
     */
    requests::ResultExpression createResultExpression(antlr4::tree::ParseTree* node);

private:
    /** SQL parser object */
    SqlParser& m_parser;

    /** Siodb data type map. */
    static const std::unordered_map<std::string, siodb::ColumnDataType> m_siodbDataTypeMap;
};

}  // namespace siodb::iomgr::dbengine::parser
