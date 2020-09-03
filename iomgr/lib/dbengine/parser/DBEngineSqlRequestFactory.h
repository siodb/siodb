// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "DBEngineSqlRequest.h"
#include "antlr_wrappers/Antlr4RuntimeWrapper.h"

namespace siodb::iomgr::dbengine::parser {

/** DBEngineSqlRequestFactory produces DB Engine requests for the SQL statements. */
class DBEngineSqlRequestFactory {
public:
    /**
     * Creates database engine request from a statement.
     * @param node A statement node.
     * @return Database engine request object filled with parsed data.
     */
    static requests::DBEngineRequestPtr createSqlRequest(antlr4::tree::ParseTree* node);

private:
    /**
     * Creates SELECT request.
     * @param node Parse tree node with SQL statement.
     * @param tag Tag idicating general form of SELECT statement.
     * @return SELECT request.
     */
    static requests::DBEngineRequestPtr createSelectRequestForGeneralSelectStatement(
            antlr4::tree::ParseTree* node);

    /**
     * Creates SELECT request.
     * @param node Parse tree node with SQL statement.
     * @param tag Tag idicating simple form of SELECT statement.
     * @return SELECT request.
     */
    static requests::DBEngineRequestPtr createSelectRequestForSimpleSelectStatement(
            antlr4::tree::ParseTree* node);

    /**
     * Creates SELECT request.
     * @param node Parse tree node with SQL statement.
     * @param tag Tag idicating factored form of SELECT statement.
     * @return SELECT request.
     */
    static requests::DBEngineRequestPtr createSelectRequestForFactoredSelectStatement(
            antlr4::tree::ParseTree* node);

    /**
     * Creates INSERT request.
     * @param node Parse tree node with SQL statement.
     * @return INSERT request.
     */
    static requests::DBEngineRequestPtr createInsertRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates UPDATE request.
     * @param node Parse tree node with SQL statement.
     * @return UPDATE request.
     */
    static requests::DBEngineRequestPtr createUpdateRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DELETE request.
     * @param node Parse tree node with SQL statement.
     * @return DELETE request.
     */
    static requests::DBEngineRequestPtr createDeleteRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates BEGIN TRANSACTION request.
     * @param node Parse tree node with SQL statement.
     * @return BEGIN TRANSACTION request.
     */
    static requests::DBEngineRequestPtr createBeginTransactionRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates COMMIT TRANSACTION request.
     * @param node Parse tree node with SQL statement.
     * @return COMMIT TRANSACTION request.
     */
    static requests::DBEngineRequestPtr createCommitTransactionRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates ROLLBACK TRANSACTION request.
     * @param node Parse tree node with SQL statement.
     * @return ROLLBACK TRANSACTION request.
     */
    static requests::DBEngineRequestPtr createRollbackTransactionRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates SAVEPOINT request.
     * @param node Parse tree node with SQL statement.
     * @return SAVEPOINT request.
     */
    static requests::DBEngineRequestPtr createSavepointRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates RELEASE request.
     * @param node Parse tree node with SQL statement.
     * @return RELEASE request.
     */
    static requests::DBEngineRequestPtr createReleaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ATTACH DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return ATTACH DATABASE request.
     */
    static requests::DBEngineRequestPtr createAttachDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DETACH DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return DETACH DATABASE request.
     */
    static requests::DBEngineRequestPtr createDetachDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates CREATE DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return CREATE DATABASE request.
     */
    static requests::DBEngineRequestPtr createCreateDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DROP DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return DROP DATABASE request.
     */
    static requests::DBEngineRequestPtr createDropDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER DATABASE RENAME TO request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER DATABASE RENAME TO request.
     */
    static requests::DBEngineRequestPtr createRenameDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER DATABASE SET attributes request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER DATABASE SET attributes request.
     */
    static requests::DBEngineRequestPtr createSetDatabaseAttributesRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates USE DATABASE request.
     * @param node Parse tree node with SQL statement.
     * @return USE DATABASE request.
     */
    static requests::DBEngineRequestPtr createUseDatabaseRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates CREATE TABLE request.
     * @param node Parse tree node with SQL statement.
     * @return CREATE TABLE request.
     */
    static requests::DBEngineRequestPtr createCreateTableRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DROP TABLE request.
     * @param node Parse tree node with SQL statement.
     * @return DROP TABLE request.
     */
    static requests::DBEngineRequestPtr createDropTableRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE RENAME TO request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE RENAME TO request.
     */
    static requests::DBEngineRequestPtr createRenameTableRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE SET attributes request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE SET attributes request.
     */
    static requests::DBEngineRequestPtr createSetTableAttributesRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates CREATE INDEX request.
     * @param node Parse tree node with SQL statement.
     * @return CREATE INDEX request.
     */
    static requests::DBEngineRequestPtr createCreateIndexRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DROP INDEX request.
     * @param node Parse tree node with SQL statement.
     * @return DROP INDEX request.
     */
    static requests::DBEngineRequestPtr createDropIndexRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE ADD COLUMN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE ADD COLUMN request.
     */
    static requests::DBEngineRequestPtr createAddColumnRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE DROP COLUMN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE DROP COLUMN request.
     */
    static requests::DBEngineRequestPtr createDropColumnRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE ALTER COLUMN RENAME TO request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE ALTER COLUMN RENAME TO request.
     */
    static requests::DBEngineRequestPtr createRenameColumnRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER TABLE ALTER COLUMN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER TABLE ALTER COLUMN request.
     */
    static requests::DBEngineRequestPtr createRedefineColumnRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates CREATE USER request.
     * @param node Parse tree node with SQL statement.
     * @return CREATE USER request.
     */
    static requests::DBEngineRequestPtr createCreateUserRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates DROP USER request.
     * @param node Parse tree node with SQL statement.
     * @return DROP USER request.
     */
    static requests::DBEngineRequestPtr createDropUserRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER SET attributes request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER SET attributes request.
     */
    static requests::DBEngineRequestPtr createSetUserAttributesRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ADD ACCESS KEY request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ADD ACCESS KEY request.
     */
    static requests::DBEngineRequestPtr createAddUserAccessKeyRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER DROP ACCESS KEY request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER DROP ACCESS KEY request.
     */
    static requests::DBEngineRequestPtr createDropUserAccessKeyRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ALTER ACCESS KEY SET ATTR_LIST request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ALTER ACCESS KEY SET ATTR_LIST request.
     */
    static requests::DBEngineRequestPtr createSetUserAccessKeyAttributesRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ALTER ACCES KEY RENAME IF EXISTS request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ALTER ACCESS KEY IF EXISTS request.
     */
    static requests::DBEngineRequestPtr createRenameUserAccessKeyRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ADD TOKEN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ADD TOKEN request.
     */
    static requests::DBEngineRequestPtr createAddUserTokenRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER DROP TOKEN request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER DROP TOKEN request.
     */
    static requests::DBEngineRequestPtr createDropUserTokenRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ALTER TOKEN SET attributes request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ALTER TOKEN SET attributes request.
     */
    static requests::DBEngineRequestPtr createSetUserTokenAttributesRequest(
            antlr4::tree::ParseTree* node);

    /**
     * Creates ALTER USER ALTER TOKEN RENAME IF EXISTS request.
     * @param node Parse tree node with SQL statement.
     * @return ALTER USER ALTER TOKEN RENAME IF EXISTS request.
     */
    static requests::DBEngineRequestPtr createRenameUserTokenRequest(antlr4::tree::ParseTree* node);

    /**
     * Creates CHECK TOKEN request.
     * @param node Parse tree node with SQL statement.
     * @return CHECK TOKEN request.
     */
    static requests::DBEngineRequestPtr createCheckUserTokenRequest(antlr4::tree::ParseTree* node);

    /**
     * Parses SelectCore.
     * @param node Parse tree node with SelectCore node.
     * @param[out] database Database name.
     * @param[out] tables List of tables.
     * @param[out] columns List of columns.
     * @param[out] where WHERE condition.
     */
    static void parseSelectCore(antlr4::tree::ParseTree* node, std::string& database,
            std::vector<requests::SourceTable>& tables,
            std::vector<requests::ResultExpression>& columns, requests::ConstExpressionPtr& where);

    /**
     * Converts given type name into Siodb column data type.
     * @param typeName Type name.
     * @return Siodb column data type.
     * @throw std::invalid_argument if given data type name doesn't represent a valid type name.
     */
    static siodb::ColumnDataType getColumnDataType(const std::string& typeName);

    /**
     * Creates a requests::ResultExpression from a selected node.
     * @param node Parse tree node with result_column statement.
     * @return requests::ResultExpression object.
     */
    static requests::ResultExpression createResultExpression(antlr4::tree::ParseTree* node);

private:
    /** Siodb data type map. */
    static const std::unordered_map<std::string, siodb::ColumnDataType> m_siodbDataTypeMap;
};

}  // namespace siodb::iomgr::dbengine::parser
