// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../DatabaseError.h"
#include "../Instance.h"
#include "../MasterColumnRecord.h"
#include "../TableDataSet.h"
#include "../parser/DBEngineRestRequest.h"
#include "../parser/DBEngineSqlRequest.h"
#include "../parser/DatabaseContext.h"
#include "../parser/expr/Expression.h"
#include "../reg/ColumnRecord.h"

// Common project headers
#include <siodb/common/io/OutputStream.h>

// Protobuf message headers
#include <siodb/common/proto/IOManagerProtocol.pb.h>

namespace siodb::io {
class JsonWriter;
}  // namespace siodb::io

namespace siodb::iomgr::dbengine {

/** 
 * Handles SQL based requests.
 * Request handler automaticall marks current database as used.
 */
class RequestHandler final {
public:
    /**
     * Initializes object of class RequestHandler.
     * @param instance DBMS instance.
     * @param connection Connection stream.
     * @param userId Current user ID.
     */
    RequestHandler(Instance& instance, siodb::io::OutputStream& connection, std::uint32_t userId);

    /** De-initializes object of class RequestHandler */
    ~RequestHandler();

    /**
     * Executes request.
     * @param request Request message.
     * @param requestId Id of a request.
     * @param responseId Id of a response.
     * @param responseCount Number of responses.
     */
    void executeRequest(const requests::DBEngineRequest& request, std::uint64_t requestId,
            std::uint32_t responseId, std::uint32_t responseCount);

    /**
     * Suppresses super-user effect on some operations.
     */
    void suppressSuperUserRights() noexcept
    {
        m_suppressSuperUserRights = true;
    }

private:
    /**
     * Returns indication that this RequestHandler acts under the super user rights.
     * @return true if this RequestHandler acts under the super user rights, false otherwise.
     */
    bool isSuperUser() const noexcept
    {
        return m_userId == User::kSuperUserId && !m_suppressSuperUserRights;
    }

    // DDL requests

    /**
     * Executes SQL CREATE DATABASE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeCreateDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CreateDatabaseRequest& request);

    /**
     * Executes SQL CREATE TABLE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeCreateTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CreateTableRequest& request);

    /**
     * Executes SQL ALTER TABLE ADD COLUMN request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAddColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AddColumnRequest& request);

    /**
     * Executes SQL CREATE INDEX request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeCreateIndexRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CreateIndexRequest& request);

    /**
     * Executes DROP DATABASE request.
     * @param response Response object.
     * @param request Request.
     */
    void executeDropDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropDatabaseRequest& request);

    /**
     * Executes ALTER DATABASE RENAME TO request.
     * @param response Response object.
     * @param request Request.
     */
    void executeRenameDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::RenameDatabaseRequest& request);

    /**
     * Executes ALTER DATABASE SET attributes request.
     * @param response Response object.
     * @param request Request.
     */
    void executeSetDatabaseAttributesRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::SetDatabaseAttributesRequest& request);

    /**
     * Executes USE DATABASE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeUseDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::UseDatabaseRequest& request);

    /**
     * Executes SQL DROP TABLE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropTableRequest& request);

    /**
     * Executes SQL ALTER TABLE DROP COLUMN request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropColumnRequest& request);

    /**
     * Executes SQL ALTER TABLE ALTER COLUMN RENAME TO request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeRenameColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::RenameColumnRequest& request);

    /**
     * Executes SQL DROP INDEX request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropIndexRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropIndexRequest& request);

    /**
     * Executes SQL ALTER TABLE ALTER COLUMN request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeRedefineColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::RedefineColumnRequest& request);

    /**
     * Executes SQL ATTACH DATABASE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAttachDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AttachDatabaseRequest& request);

    /**
     * Executes SQL DETACH DATABASE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDetachDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DetachDatabaseRequest& request);

    /**
     * Executes SQL RENAME TABLE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeRenameTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::RenameTableRequest& request);

    /**
     * Executes SQL ALTER TABLE SET attributes request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeSetTableAttributesRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::SetTableAttributesRequest& request);

    // DML requests

    /**
     * Executes SQL SELECT request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeSelectRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::SelectRequest& request);

    /**
     * Executes SQL UPDATE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeUpdateRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::UpdateRequest& request);

    /**
     * Executes SQL DELETE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDeleteRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DeleteRequest& request);

    /**
     * Executes SQL INSERT request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeInsertRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::InsertRequest& request);

    // TC requests

    /**
     * Executes SQL BEGIN TRANSACTION request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeBeginTransactionRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::BeginTransactionRequest& request);

    /**
     * Executes SQL COMMIT TRANSACTION request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeCommitTransactionRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CommitTransactionRequest& request);

    /**
     * Executes SQL ROLLBACK TRANSACTION request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeRollbackTransactionRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::RollbackTransactionRequest& request);

    /**
     * Executes SQL SAVEPOINT request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeSavepointRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::SavepointRequest& request);

    /**
     * Executes SQL RELEASE request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeReleaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::ReleaseRequest& request);

    // UM requests

    /**
     * Executes SQL CREATE USER request
     * @param response Response object.
     * @param request Request object.
     */
    void executeCreateUserRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CreateUserRequest& request);

    /**
     * Executes SQL DROP USER request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropUserRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropUserRequest& request);

    /**
     * Executes SQL ALTER USER SET attributes request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeSetUserAttributesRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::SetUserAttributesRequest& request);

    /**
     * Executes SQL ALTER USER ADD ACCESS KEY request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAddUserAccessKeyRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AddUserAccessKeyRequest& request);

    /**
     * Executes SQL ALTER USER DROP ACCESS KEY request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropUserAccessKeyRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropUserAccessKeyRequest& request);

    /**
     * Executes SQL ALTER USER ALTER ACCESS KEY SET attributes request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeSetUserAccessKeyAttributesRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::SetUserAccessKeyAttributesRequest& request);

    /**
     * Executes SQL ALTER USER ALTER ACCESS KEY RENAME TO request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeRenameUserAccessKeyRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::RenameUserAccessKeyRequest& request);

    /**
     * Executes SQL ALTER USER ADD TOKEN request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAddUserTokenRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AddUserTokenRequest& request);

    /**
     * Executes SQL ALTER USER DROP TOKEN request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropUserTokenRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropUserTokenRequest& request);

    /**
     * Executes SQL ALTER USER ALTER TOKEN SET attributes request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeSetUserTokenAttributesRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::SetUserTokenAttributesRequest& request);

    /**
     * Executes SQL ALTER USER ALTER TOKEN RENAME TO request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeRenameUserTokenRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::RenameUserTokenRequest& request);

    /**
     * Executes CHECK TOKEN request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeCheckUserTokenRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CheckUserTokenRequest& request);

    /**
     * Executes SQL SHOW DATABASES request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeShowDatabasesRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::ShowDatabasesRequest& request);

    // REST request handlers

    /**
     * Executes REST GET DATABASES request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeGetDatabasesRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::GetDatabasesRestRequest& request);

    /**
     * Executes REST GET TABLES request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeGetTablesRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::GetTablesRestRequest& request);

    /**
     * Executes REST GET ALL ROWS request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeGetAllRowsRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::GetAllRowsRestRequest& request);

    /**
     * Executes REST GET SINGLE ROW request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeGetSingleRowRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::GetSingleRowRestRequest& request);

    /**
     * Executes REST POST ROWS request.
     * @param response Response object.
     * @param request Request object.
     */
    void executePostRowsRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::PostRowsRestRequest& request);

    // Helpers

    /**
     * Adds user visible database error to the response.
     * @param response Response object.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    void addInternalDatabaseErrorToResponse(iomgr_protocol::DatabaseEngineResponse& response,
            int errorCode, const char* errorMessage);

    /**
     * Adds internal database error to the response.
     * @param response Response object.
     * @param errorCode Error code.
     * @param errorMessage Error message.
     */
    void addUserVisibleDatabaseErrorToResponse(iomgr_protocol::DatabaseEngineResponse& response,
            int errorCode, const char* errorMessage);

    /**
     * Adds user IO database error to response.
     * @param response Response.
     * @param errorCode Error code.
     * @param errorMessage Error message.
    */
    void addIoErrorToResponse(iomgr_protocol::DatabaseEngineResponse& response, int errorCode,
            const char* errorMessage);

    /**
     * Adds column description with alias name to the response.
     * @param response Response object.
     * @param column Column object.
     * @param alias Column alias.
     */
    static void addColumnToResponse(iomgr_protocol::DatabaseEngineResponse& response,
            const Column& column, const std::string& alias = std::string());

    /**
     * Replies with 'Not implemented yet' error.
     * @param response Response object.
     */
    void sendNotImplementedYet(iomgr_protocol::DatabaseEngineResponse& response);

    /**
     * Return variant value size for protocol value data.
     * @param value A value.
     * @return Variant value size for protocol value data.
     */
    static std::size_t getVariantSize(const Variant& value);

    /**
     * Writes variant value into coded output stream in the binary format.
     * @param value A value.
     * @param codedOutput Output stream.
     */
    static void writeVariant(
            const Variant& value, google::protobuf::io::CodedOutputStream& codedOutput);

    /**
     * Writes variant value as JSON.
     * @param fieldName Field name.
     * @param value A value.
     * @param jsonWriter JSON writer object.
     */
    static void writeVariantAsJson(const Variant& value, siodb::io::JsonWriter& jsonWriter);

    /**
     * Converts request column definition to database engine column info.
     * @param columnDefinition Protocol column definition.
     * @return Column info.
     */
    static ColumnSpecification convertTableColumnDefinition(
            const requests::ColumnDefinition& columnDefinition);

    /**
     * Updates table and column indices in expressions, and fills columnSpecs.
     * @param dataSets Data sets for expression.
     * @param expression Expression.
     * @param errors Vector with errors.
     */
    void updateColumnsFromExpression(const std::vector<DataSetPtr>& dataSets,
            const requests::ConstExpressionPtr& expression,
            std::vector<CompoundDatabaseError::ErrorRecord>& errors) const;

    /**
     * Checks where expression.
     * @param whereExpression WHERE clause expression.
     * @param context A context.
     * @throw DatabaseError if where expression is invalid.
     */
    void checkWhereExpression(const requests::ConstExpressionPtr& whereExpression,
            requests::DatabaseContext& context);

    /**
     * Writes JSON prolog.
     * @param jsonWriter JSON writer object.
     * @throw std::system_error on write error.
     */
    void writeJsonProlog(siodb::io::JsonWriter& jsonWriter);

    /**
     * Writes JSON epilog.
     * @param jsonWriter JSON writer object.
     * @throw std::system_error on write error.
     */
    void writeJsonEpilog(siodb::io::JsonWriter& jsonWriter);

private:
    /** DBMS instance */
    Instance& m_instance;

    /** Connection stream */
    siodb::io::OutputStream& m_connection;

    /** Current user ID */
    const std::uint32_t m_userId;

    /** Current database */
    std::string m_currentDatabaseName;

    /** Inidication that super-user access rights should be suppressed */
    bool m_suppressSuperUserRights;

    /** Log context name */
    static constexpr const char* kLogContext = "RequestHandler: ";

    /** Row size that indicates "no more rows" */
    static constexpr std::uint64_t kNoMoreRows = 0;

    /** "Not implemented" error code */
    static constexpr int kFeatureNotImplementedErrorCode = 6;

    /** LOB chunk size */
    static constexpr std::uint32_t kLobChunkSize = 4096;

    /** JSON chunk size */
    static constexpr std::size_t kJsonChunkSize = 65536;

    /** Token prefix in the freetext message */
    static constexpr const char* kTokenResponsePrefix = "token: ";

    /** REST status field name */
    static constexpr const char* kRestStatusFieldName = "status";

    /** REST status OK */
    static constexpr int kRestStatusOk = 200;

    /** REST rows field name */
    static constexpr const char* kRestRowsFieldName = "rows";

    /** Database name field name */
    static constexpr const char* kDatabaseNameFieldName = "name";

    /** Table name field name */
    static constexpr const char* kTableNameFieldName = "name";
};

}  // namespace siodb::iomgr::dbengine
