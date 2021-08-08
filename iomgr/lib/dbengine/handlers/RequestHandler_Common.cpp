// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "SqlClientProtocolRowsetWriterFactory.h"
#include "../Column.h"
#include "../DatabaseError.h"
#include "../Index.h"
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/JsonWriter.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/Uuid.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/BinaryOperator.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/ConstantExpression.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/InOperator.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/SingleColumnExpression.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/TernaryOperator.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/UnaryOperator.h>

// STL headers
#include <functional>

namespace siodb::iomgr::dbengine {

RequestHandler::RequestHandler(
        Instance& instance, siodb::io::OutputStream& connection, std::uint32_t userId)
    : m_instance(instance)
    , m_connection(connection)
    , m_currentUserId(userId)
    , m_currentDatabaseName(kSystemDatabaseName)
    , m_suppressSuperUserRights(false)
{
    m_instance.findDatabaseChecked(m_currentDatabaseName)->use();
}

RequestHandler::~RequestHandler()
{
    try {
        m_instance.findDatabaseChecked(m_currentDatabaseName)->release();
    } catch (std::exception& ex) {
        try {
            LOG_ERROR << "RequestHandler: " << ex.what();
        } catch (...) {
            // Ignore any exceptions
        }
    }
}

void RequestHandler::executeRequest(const requests::DBEngineRequest& request,
        std::uint64_t requestId, std::uint32_t responseId, std::uint32_t responseCount)
{
    iomgr_protocol::DatabaseEngineResponse response;
    try {
        response.set_request_id(requestId);
        response.set_response_id(responseId);
        response.set_response_count(responseCount);
        switch (request.m_requestType) {
            case requests::DBEngineRequestType::kSelect: {
                SqlClientProtocolRowsetWriterFactory rowsetWriterFactory;
                executeSelectRequest(response,
                        dynamic_cast<const requests::SelectRequest&>(request), rowsetWriterFactory);
                break;
            }

            case requests::DBEngineRequestType::kShowDatabases: {
                executeShowDatabasesRequest(
                        response, dynamic_cast<const requests::ShowDatabasesRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kShowTables: {
                executeShowTablesRequest(response);
                break;
            }

            case requests::DBEngineRequestType::kDescribeTable: {
                executeDescribeTableRequest(
                        response, dynamic_cast<const requests::DescribeTableRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kInsert: {
                executeInsertRequest(
                        response, dynamic_cast<const requests::InsertRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kUpdate: {
                executeUpdateRequest(
                        response, dynamic_cast<const requests::UpdateRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kDelete: {
                executeDeleteRequest(
                        response, dynamic_cast<const requests::DeleteRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kBeginTransaction: {
                executeBeginTransactionRequest(
                        response, dynamic_cast<const requests::BeginTransactionRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kCommitTransaction: {
                executeCommitTransactionRequest(
                        response, dynamic_cast<const requests::CommitTransactionRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRollbackTransaction: {
                executeRollbackTransactionRequest(response,
                        dynamic_cast<const requests::RollbackTransactionRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kSavepoint: {
                executeSavepointRequest(
                        response, dynamic_cast<const requests::SavepointRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRelease: {
                executeReleaseRequest(
                        response, dynamic_cast<const requests::ReleaseRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kAttachDatabase: {
                executeAttachDatabaseRequest(
                        response, dynamic_cast<const requests::AttachDatabaseRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kDetachDatabase: {
                executeDetachDatabaseRequest(
                        response, dynamic_cast<const requests::DetachDatabaseRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kCreateDatabase: {
                executeCreateDatabaseRequest(
                        response, dynamic_cast<const requests::CreateDatabaseRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kDropDatabase: {
                executeDropDatabaseRequest(
                        response, dynamic_cast<const requests::DropDatabaseRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRenameDatabase: {
                executeRenameDatabaseRequest(
                        response, dynamic_cast<const requests::RenameDatabaseRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kSetDatabaseAttributes: {
                executeSetDatabaseAttributesRequest(response,
                        dynamic_cast<const requests::SetDatabaseAttributesRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kUseDatabase: {
                executeUseDatabaseRequest(
                        response, dynamic_cast<const requests::UseDatabaseRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kCreateTable: {
                executeCreateTableRequest(
                        response, dynamic_cast<const requests::CreateTableRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kDropTable: {
                executeDropTableRequest(
                        response, dynamic_cast<const requests::DropTableRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRenameTable: {
                executeRenameTableRequest(
                        response, dynamic_cast<const requests::RenameTableRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kSetTableAttributes: {
                executeSetTableAttributesRequest(response,
                        dynamic_cast<const requests::SetTableAttributesRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kAddColumn: {
                executeAddColumnRequest(
                        response, dynamic_cast<const requests::AddColumnRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kDropColumn: {
                executeDropColumnRequest(
                        response, dynamic_cast<const requests::DropColumnRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRenameColumn: {
                executeRenameColumnRequest(
                        response, dynamic_cast<const requests::RenameColumnRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRedefineColumn: {
                executeRedefineColumnRequest(
                        response, dynamic_cast<const requests::RedefineColumnRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kCreateIndex: {
                executeCreateIndexRequest(
                        response, dynamic_cast<const requests::CreateIndexRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kDropIndex: {
                executeDropIndexRequest(
                        response, dynamic_cast<const requests::DropIndexRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kCreateUser: {
                executeCreateUserRequest(
                        response, dynamic_cast<const requests::CreateUserRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kDropUser: {
                executeDropUserRequest(
                        response, dynamic_cast<const requests::DropUserRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kSetUserAttributes: {
                executeSetUserAttributesRequest(
                        response, dynamic_cast<const requests::SetUserAttributesRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kAddUserAccessKey: {
                executeAddUserAccessKeyRequest(
                        response, dynamic_cast<const requests::AddUserAccessKeyRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kDropUserAccessKey: {
                executeDropUserAccessKeyRequest(
                        response, dynamic_cast<const requests::DropUserAccessKeyRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kSetUserAccessKeyAttributes: {
                executeSetUserAccessKeyAttributesRequest(response,
                        dynamic_cast<const requests::SetUserAccessKeyAttributesRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRenameUserAccessKey: {
                executeRenameUserAccessKeyRequest(response,
                        dynamic_cast<const requests::RenameUserAccessKeyRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kAddUserToken: {
                executeAddUserTokenRequest(
                        response, dynamic_cast<const requests::AddUserTokenRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kDropUserToken: {
                executeDropUserTokenRequest(
                        response, dynamic_cast<const requests::DropUserTokenRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kSetUserTokenAttributes: {
                executeSetUserTokenAttributesRequest(response,
                        dynamic_cast<const requests::SetUserTokenAttributesRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRenameUserToken: {
                executeRenameUserTokenRequest(
                        response, dynamic_cast<const requests::RenameUserTokenRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kCheckUserToken: {
                executeCheckUserTokenRequest(
                        response, dynamic_cast<const requests::CheckUserTokenRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kGrantPermissionsForTable: {
                executeGrantPermissionsForTableRequest(response,
                        dynamic_cast<const requests::GrantPermissionsForTableRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRevokePermissionsForTable: {
                executeRevokePermissionsForTableRequest(response,
                        dynamic_cast<const requests::RevokePermissionsForTableRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRestGetDatabases: {
                executeGetDatabasesRestRequest(
                        response, dynamic_cast<const requests::GetDatabasesRestRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRestGetTables: {
                executeGetTablesRestRequest(
                        response, dynamic_cast<const requests::GetTablesRestRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRestGetAllRows: {
                executeGetAllRowsRestRequest(
                        response, dynamic_cast<const requests::GetAllRowsRestRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRestGetSingleRow: {
                executeGetSingleRowRestRequest(
                        response, dynamic_cast<const requests::GetSingleRowRestRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRestGetSqlQueryRows: {
                executeGetSqlQueryRowsRestRequest(response,
                        dynamic_cast<const requests::GetSqlQueryRowsRestRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRestPostRows: {
                executePostRowsRestRequest(
                        response, dynamic_cast<const requests::PostRowsRestRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRestDeleteRow: {
                executeDeleteRowRestRequest(
                        response, dynamic_cast<const requests::DeleteRowRestRequest&>(request));
                break;
            }

            case requests::DBEngineRequestType::kRestPatchRow: {
                executePatchRowRestRequest(
                        response, dynamic_cast<const requests::PatchRowRestRequest&>(request));
                break;
            }

            default: throw std::invalid_argument("Unknown request type");
        }
    } catch (const UserVisibleDatabaseError& ex) {
        addUserVisibleDatabaseErrorToResponse(response, ex.getErrorCode(), ex.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
    } catch (const InternalDatabaseError& ex) {
        addInternalDatabaseErrorToResponse(response, ex.getErrorCode(), ex.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
    } catch (const DatabaseIOError& ex) {
        addIoErrorToResponse(response, ex.getErrorCode(), ex.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
    } catch (const CompoundDatabaseError& ex) {
        for (const auto& err : ex.getErrors()) {
            if (DatabaseError::isMessageIdInRange(
                        err.m_errorCode, DatabaseError::kIOErrorCodeRange)) {
                addIoErrorToResponse(response, err.m_errorCode, err.m_message.c_str());
            } else if (DatabaseError::isMessageIdInRange(
                               err.m_errorCode, DatabaseError::kInternalErrorCodeRange)) {
                addInternalDatabaseErrorToResponse(
                        response, err.m_errorCode, err.m_message.c_str());
            } else {
                addUserVisibleDatabaseErrorToResponse(
                        response, err.m_errorCode, err.m_message.c_str());
            }
        }
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
    } catch (fs::filesystem_error& ex) {
        addIoErrorToResponse(response, ex.code().value(), ex.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
    } catch (boost::system::system_error& ex) {
        addInternalDatabaseErrorToResponse(response, ex.code().value(), ex.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
    } catch (std::system_error& ex) {
        addInternalDatabaseErrorToResponse(response, ex.code().value(), ex.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
    } catch (std::exception& ex) {
        addInternalDatabaseErrorToResponse(response, -1, ex.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
    }
}

void RequestHandler::addUserVisibleDatabaseErrorToResponse(
        iomgr_protocol::DatabaseEngineResponse& response, int errorCode, const char* errorMessage)
{
    LOG_ERROR << kLogContext << '[' << errorCode << "] " << errorMessage;
    const auto msg = response.add_message();
    msg->set_status_code(errorCode);
    msg->set_text(errorMessage);
}

void RequestHandler::addInternalDatabaseErrorToResponse(
        iomgr_protocol::DatabaseEngineResponse& response, int errorCode, const char* errorMessage)
{
    const auto uuid = boost::uuids::random_generator()();
    LOG_ERROR << kLogContext << '[' << errorCode << "] " << errorMessage << " (MSG_UUID " << uuid
              << ')';
    const auto msg = response.add_message();
    msg->set_status_code(1);
    msg->set_text("Internal error, see Siodb server log for details, message UUID "
                  + boost::uuids::to_string(uuid));
}

void RequestHandler::addIoErrorToResponse(
        iomgr_protocol::DatabaseEngineResponse& response, int errorCode, const char* errorMessage)
{
    const auto uuid = boost::uuids::random_generator()();
    LOG_ERROR << kLogContext << '[' << errorCode << "] " << errorMessage << " (MSG_UUID " << uuid
              << ')';
    const auto msg = response.add_message();
    msg->set_status_code(1);
    msg->set_text("IO error, see Siodb server log for details, message UUID "
                  + boost::uuids::to_string(uuid));
}

void RequestHandler::addColumnToResponse(iomgr_protocol::DatabaseEngineResponse& response,
        const Column& column, const std::string& alias)
{
    const auto columnDescription = response.add_column_description();
    columnDescription->set_name(alias.empty() ? column.getName() : alias);
    columnDescription->set_is_null(!column.isNotNull());
    columnDescription->set_type(column.getDataType());
}

void RequestHandler::sendNotImplementedYet(iomgr_protocol::DatabaseEngineResponse& response)
{
    const auto msg = response.add_message();
    msg->set_status_code(kFeatureNotImplementedErrorCode);
    msg->set_text("Not implemented yet");
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

ColumnSpecification RequestHandler::convertTableColumnDefinition(
        const requests::ColumnDefinition& columnDefinition)
{
    std::vector<ColumnConstraintSpecification> constraintSpecs;
    for (const auto& constraint : columnDefinition.m_constraints) {
        switch (constraint->m_type) {
            case ConstraintType::kNotNull: {
                const auto& src = dynamic_cast<const requests::NotNullConstraint&>(*constraint);
                constraintSpecs.emplace_back(std::string(src.m_name), ConstraintType::kNotNull,
                        std::make_unique<requests::ConstantExpression>(Variant(src.m_notNull)),
                        std::nullopt);
                break;
            }
            case ConstraintType::kDefaultValue: {
                const auto& src =
                        dynamic_cast<const requests::DefaultValueConstraint&>(*constraint);
                constraintSpecs.emplace_back(std::string(src.m_name), ConstraintType::kDefaultValue,
                        requests::ExpressionPtr(src.m_value->clone()), std::nullopt);
                break;
            }
            default: {
                throwDatabaseError(IOManagerMessageId::kErrorConstraintNotSupported2,
                        static_cast<unsigned>(constraint->m_type));
            }
        }
    }
    return ColumnSpecification(std::string(columnDefinition.m_name), columnDefinition.m_dataType,
            columnDefinition.m_dataBlockDataAreaSize, std::move(constraintSpecs));
}

void RequestHandler::updateColumnsFromExpression(const std::vector<DataSetPtr>& dataSets,
        const requests::ConstExpressionPtr& expression,
        std::vector<CompoundDatabaseError::ErrorRecord>& errors) const
{
    if (!expression) {
        // Normally should never happen
        throw std::runtime_error(
                "RequestHandler::updateColumnsFromExpression: expression is nullptr");
    }

    std::vector<const requests::Expression*> expressions;
    // Reserve some space for expressions to avoid memory allocations/reallocations
    constexpr std::size_t kReservedExpressionCount = 32;
    expressions.reserve(kReservedExpressionCount);

    std::vector<std::unordered_map<std::reference_wrapper<const std::string>, int,
            std::hash<std::string>, std::equal_to<std::string>>>
            columns(dataSets.size());

    for (std::size_t i = 0; i < dataSets.size(); ++i) {
        for (std::size_t j = 0; j < dataSets[i]->getColumnCount(); ++j)
            columns[i].insert(std::make_pair(std::cref(dataSets[i]->getColumnName(j)), j));
    }

    expressions.push_back(expression.get());

    while (!expressions.empty()) {
        const auto expression = expressions.back();
        expressions.pop_back();

        if (expression->getType() == requests::ExpressionType::kSingleColumnReference) {
            const auto columnExpression =
                    dynamic_cast<const requests::SingleColumnExpression*>(expression);
            if (columnExpression == nullptr) {
                // Normally should never happen
                throw std::runtime_error("SingleColumnExpression expression type cast failed");
            }

            const auto& tableName = columnExpression->getTableName();
            const auto& columnName = columnExpression->getColumnName();

            std::size_t tableIndex = 0;
            if (!tableName.empty()) {
                for (; tableIndex < dataSets.size(); ++tableIndex) {
                    if (tableName == dataSets[tableIndex]->getName()
                            || tableName == dataSets[tableIndex]->getAlias())
                        break;
                }

                if (tableIndex == dataSets.size()) {
                    errors.push_back(makeDatabaseError(
                            IOManagerMessageId::kErrorTableNotSpecified, tableName));
                    continue;
                }
            }

            // -1 indicates column was not added before
            auto insertIter = columns[tableIndex].insert(std::make_pair(std::cref(columnName), -1));

            if (insertIter.second) {
                // Column inserted
                const auto columnPos =
                        dataSets[tableIndex]->getDataSourceColumnPosition(columnName);
                if (columnPos) {
                    // Required to set column and table indices
                    auto nonConstColumnExpression = stdext::as_mutable_ptr(columnExpression);
                    nonConstColumnExpression->setSingleDatasetTableIndex(tableIndex);
                    const auto newColumnIndex = dataSets[tableIndex]->getColumnCount();
                    nonConstColumnExpression->setDatasetColumnIndex(newColumnIndex);
                    insertIter.first->second = newColumnIndex;
                    dataSets[tableIndex]->emplaceColumnInfo(*columnPos, columnName, std::string());
                } else {
                    errors.push_back(makeDatabaseError(
                            IOManagerMessageId::kErrorColumnIsUnknown, tableName, columnName));
                }
            } else if (insertIter.first->second != -1) {
                // Column already exists
                const auto nonConstColumnExpression = stdext::as_mutable_ptr(columnExpression);
                nonConstColumnExpression->setSingleDatasetTableIndex(tableIndex);
                nonConstColumnExpression->setDatasetColumnIndex(insertIter.first->second);
            }
        } else if (expression->isUnaryOperator()) {
            const auto unaryOperator = dynamic_cast<const requests::UnaryOperator*>(expression);
            if (unaryOperator == nullptr) {
                // Normally should never happen
                throw std::runtime_error("UnaryOperator type cast failed");
            }
            expressions.push_back(&unaryOperator->getOperand());
        } else if (expression->isBinaryOperator()) {
            const auto binaryOperator = dynamic_cast<const requests::BinaryOperator*>(expression);
            if (binaryOperator == nullptr) {
                // Normally should never happen
                throw std::runtime_error("BinaryOperator type cast failed");
            }
            expressions.push_back(&binaryOperator->getLeftOperand());
            expressions.push_back(&binaryOperator->getRightOperand());
        } else if (expression->isTernaryOperator()) {
            const auto ternaryOperator = dynamic_cast<const requests::TernaryOperator*>(expression);
            if (ternaryOperator == nullptr) {
                // Normally should never happen
                throw std::runtime_error("TernaryOperator type cast failed");
            }
            expressions.push_back(&ternaryOperator->getLeftOperand());
            expressions.push_back(&ternaryOperator->getMiddleOperand());
            expressions.push_back(&ternaryOperator->getRightOperand());
        } else if (expression->getType() == requests::ExpressionType::kInPredicate) {
            const auto inOperator = dynamic_cast<const requests::InOperator*>(expression);
            if (inOperator == nullptr) {
                // Normally should never happen
                throw std::runtime_error("InOperator type cast failed");
            }
            expressions.push_back(&inOperator->getValue());
            for (const auto& variant : inOperator->getVariants())
                expressions.push_back(variant.get());
        }
    }
}

void RequestHandler::checkWhereExpression(const requests::ConstExpressionPtr& whereExpression,
        requests::DBExpressionEvaluationContext& context)
{
    if (!whereExpression) return;
    try {
        whereExpression->validate(context);
    } catch (std::exception& e) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidWhereCondition, e.what());
    }
    if (!isBoolType(whereExpression->getResultValueType(context))) {
        throwDatabaseError(
                IOManagerMessageId::kErrorInvalidWhereCondition, "Result is not boolean value");
    }
}

}  // namespace siodb::iomgr::dbengine
