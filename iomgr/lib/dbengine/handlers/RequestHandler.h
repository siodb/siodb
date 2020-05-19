// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../DatabaseError.h"
#include "../Instance.h"
#include "../MasterColumnRecord.h"
#include "../TableDataSet.h"
#include "../Variant.h"
#include "../parser/DBEngineRequest.h"
#include "../parser/DatabaseContext.h"
#include "../parser/expr/Expression.h"
#include "../reg/ColumnRecord.h"

// Common project headers
#include <siodb/common/io/IoBase.h>

// Protobuf message headers
#include <siodb/common/proto/IOManagerProtocol.pb.h>

namespace siodb::iomgr::dbengine {

/** Handles SQL based requests */
class RequestHandler final {
public:
    /**
     * Initializes object of class RequestHandler.
     * @param instance DBMS instance.
     * @param connectionIo Connection with server file descriptor.
     * @param userId Current user ID.
     */
    RequestHandler(Instance& instance, siodb::io::IoBase& connectionIo, std::uint32_t userId);

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

private:
    // DDL queries

    /**
     * Executes SQL create database request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeCreateDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CreateDatabaseRequest& request);

    /**
     * Executes SQL create table request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeCreateTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CreateTableRequest& request);

    /**
     * Executes SQL add column request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAddColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AddColumnRequest& request);

    /**
     * Executes SQL create index request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeCreateIndexRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CreateIndexRequest& request);

    /**
     *
     * Executes drop database request.
     * @param response Response object.
     * @param request Request.
     */
    void executeDropDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropDatabaseRequest& request);

    /**
     * Executes use database request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeUseDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::UseDatabaseRequest& request);

    /**
     * Executes SQL drop table request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropTableRequest& request);

    /**
     * Executes SQL drop column request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropColumnRequest& request);

    /**
     * Executes SQL drop index request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropIndexRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropIndexRequest& request);

    /**
     * Executes SQL ALTER COLUMN request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAlterColumnRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AlterColumnRequest& request);

    /**
     * Executes SQL attach database request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAttachDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AttachDatabaseRequest& request);

    /**
     * Executes SQL detach database request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDetachDatabaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DetachDatabaseRequest& request);

    /**
     * Executes SQL rename table request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeRenameTableRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::RenameTableRequest& request);

    //** DML queries */

    /**
     * Executes SQL select request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeSelectRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::SelectRequest& request);

    /**
     * Executes SQL update request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeUpdateRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::UpdateRequest& request);

    /**
     * Executes SQL delete request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDeleteRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DeleteRequest& request);

    /**
     * Executes SQL insert request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeInsertRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::InsertRequest& request);

    //** TCL queries */

    /**
     * Executes SQL begin transaction request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeBeginTransactionRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::BeginTransactionRequest& request);

    /**
     * Executes SQL commit transaction request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeCommitTransactionRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CommitTransactionRequest& request);

    /**
     * Executes SQL rollback transaction request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeRollbackTransactionRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::RollbackTransactionRequest& request);

    /**
     * Executes SQL savepoint request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeSavepointRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::SavepointRequest& request);

    /**
     * Executes SQL release request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeReleaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::ReleaseRequest& request);

    /**
     * Executes SQL create user request
     * @param response Response object.
     * @param request Request object.
     */
    void executeCreateUserRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::CreateUserRequest& request);

    /**
     * Executes SQL drop user request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropUserRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropUserRequest& request);

    /**
     * Executes SQL alter user set state request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAlterUserRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AlterUserRequest& request);

    /**
     * Executes SQL alter user add access key request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAddUserAccessKeyRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AddUserAccessKeyRequest& request);

    /**
     * Executes SQL alter user drop access key request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeDropUserAccessKeyRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::DropUserAccessKeyRequest& request);

    /**
     * Executes SQL alter user alter access key request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeAlterUserAccessKeyRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::AlterUserAccessKey& request);

    /**
     * Executes SQL SHOW DATABASES request.
     * @param response Response object.
     * @param request Request object.
     */
    void executeShowDatabasesRequest(iomgr_protocol::DatabaseEngineResponse& response,
            const requests::ShowDatabasesRequest& request);

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
     * Writes variant value into coded output stream.
     * @param codedOutput Output stream.
     * @param value A value.
     */
    static void writeVariant(
            google::protobuf::io::CodedOutputStream& codedOutput, const Variant& value);

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
     * @throw DatabaseError in case of invalid where exception.
     */
    void checkWhereExpression(const requests::ConstExpressionPtr& whereExpression,
            requests::DatabaseContext& context);

private:
    /** DBMS instance */
    Instance& m_instance;

    /** Connection IO descriptor */
    siodb::io::IoBase& m_connectionIo;

    /** Current user ID */
    const std::uint32_t m_userId;

    /** Current database */
    std::string m_currentDatabaseName;

    /** Log context name */
    static constexpr const char* kLogContext = "RequestHandler: ";

    /** Row size that indicates "no more rows" */
    static constexpr std::uint64_t kNoMoreRows = 0;

    /** "Not implemented" error code */
    static constexpr int kFeatureNotImplementedErrorCode = 6;

    /** LOB chunk size */
    static constexpr std::uint32_t kLobChunkSize = 4096;

    /** Reserved expressions count */
    static constexpr std::size_t kReservedExpressionCount = 32;
};

}  // namespace siodb::iomgr::dbengine
