// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../Column.h"
#include "../DatabaseError.h"
#include "../Index.h"
#include "../ThrowDatabaseError.h"
#include "../parser/expr/BinaryOperator.h"
#include "../parser/expr/ConstantExpression.h"
#include "../parser/expr/InOperator.h"
#include "../parser/expr/SingleColumnExpression.h"
#include "../parser/expr/TernaryOperator.h"
#include "../parser/expr/UnaryOperator.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>
#include <siodb/common/utils/Uuid.h>

// STL headers
#include <functional>

// Boost headers
#include <boost/endian/conversion.hpp>

namespace siodb::iomgr::dbengine {

RequestHandler::RequestHandler(
        Instance& instance, siodb::io::IoBase& connectionIo, std::uint32_t userId)
    : m_instance(instance)
    , m_connectionIo(connectionIo)
    , m_userId(userId)
    , m_currentDatabaseName(Database::kSystemDatabaseName)
{
    m_instance.getDatabaseChecked(m_currentDatabaseName)->use();
}

RequestHandler::~RequestHandler()
{
    try {
        m_instance.getDatabaseChecked(m_currentDatabaseName)->release();
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
                executeSelectRequest(
                        response, dynamic_cast<const requests::SelectRequest&>(request));
                break;
            }
            case requests::DBEngineRequestType::kShowDatabases: {
                executeShowDatabasesRequest(
                        response, dynamic_cast<const requests::ShowDatabasesRequest&>(request));
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
            case requests::DBEngineRequestType::kAlterColumn: {
                executeAlterColumnRequest(
                        response, dynamic_cast<const requests::AlterColumnRequest&>(request));
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
            case requests::DBEngineRequestType::kAlterUser: {
                executeAlterUserRequest(
                        response, dynamic_cast<const requests::AlterUserRequest&>(request));
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
            case requests::DBEngineRequestType::kAlterUserAccessKey: {
                executeAlterUserAccessKeyRequest(
                        response, dynamic_cast<const requests::AlterUserAccessKey&>(request));
                break;
            }
            default: {
                throw std::invalid_argument("Unknown request type");
            }
        }
    } catch (const UserVisibleDatabaseError& userDbErrorEx) {
        addUserVisibleDatabaseErrorToResponse(
                response, userDbErrorEx.getErrorCode(), userDbErrorEx.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
    } catch (const InternalError& internalErrorEx) {
        addInternalDatabaseErrorToResponse(
                response, internalErrorEx.getErrorCode(), internalErrorEx.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
    } catch (const IOError& ioErrorEx) {
        addIoErrorToResponse(response, ioErrorEx.getErrorCode(), ioErrorEx.what());
        protobuf::writeMessage(
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
    } catch (const CompoundDatabaseError& compoundError) {
        for (const auto& err : compoundError.getErrors()) {
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
                protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
    }
}

void RequestHandler::addUserVisibleDatabaseErrorToResponse(
        iomgr_protocol::DatabaseEngineResponse& response, int errorCode, const char* errorMessage)
{
    LOG_ERROR << kLogContext << '[' << errorCode << "] " << errorMessage;

    auto msg = response.add_message();
    msg->set_status_code(errorCode);
    msg->set_text(errorMessage);
}

void RequestHandler::addInternalDatabaseErrorToResponse(
        iomgr_protocol::DatabaseEngineResponse& response, int errorCode, const char* errorMessage)
{
    const auto uuid = boost::uuids::random_generator()();
    LOG_ERROR << kLogContext << '[' << errorCode << "] " << errorMessage << " (" << uuid << ')';

    auto msg = response.add_message();
    msg->set_status_code(1);
    msg->set_text(
            "Internal error, see log for details, message UUID " + boost::uuids::to_string(uuid));
}

void RequestHandler::addIoErrorToResponse(
        iomgr_protocol::DatabaseEngineResponse& response, int errorCode, const char* errorMessage)
{
    const auto uuid = boost::uuids::random_generator()();
    LOG_ERROR << kLogContext << '[' << errorCode << "] " << errorMessage << " (" << uuid << ')';

    auto msg = response.add_message();
    msg->set_status_code(1);
    msg->set_text("IO error, see log for details, message UUID " + boost::uuids::to_string(uuid));
}

void RequestHandler::addColumnToResponse(iomgr_protocol::DatabaseEngineResponse& response,
        const Column& column, const std::string& alias)
{
    auto columnDescription = response.add_column_description();
    columnDescription->set_name(alias.empty() ? column.getName() : alias);
    columnDescription->set_is_null(!column.isNotNull());
    columnDescription->set_type(column.getDataType());
}

void RequestHandler::sendNotImplementedYet(iomgr_protocol::DatabaseEngineResponse& response)
{
    auto msg = response.add_message();
    msg->set_status_code(kFeatureNotImplementedErrorCode);
    msg->set_text("Not implemented yet");
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connectionIo);
}

std::size_t RequestHandler::getVariantSize(const Variant& value)
{
    switch (value.getValueType()) {
        case VariantType::kNull: return 0;
        case VariantType::kBool:
        case VariantType::kInt8:
        case VariantType::kUInt8: return 1;
        case VariantType::kInt16:
        case VariantType::kUInt16: return 2;
        case VariantType::kInt32: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getInt32());
        }
        case VariantType::kUInt32: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getUInt32());
        }
        case VariantType::kInt64: {
            return google::protobuf::io::CodedOutputStream::VarintSize64(value.getInt64());
        }
        case VariantType::kUInt64: {
            return google::protobuf::io::CodedOutputStream::VarintSize64(value.getUInt64());
        }
        case VariantType::kFloat: return 4;
        case VariantType::kDouble: return 8;
        case VariantType::kDateTime: return value.getDateTime().getSerializedSize();
        case VariantType::kString: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getString().size())
                   + value.getString().size();
        }
        case VariantType::kBinary: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getBinary().size())
                   + value.getBinary().size();
        }
        case VariantType::kClob: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getClob().getSize())
                   + value.getClob().getSize();
        }
        case VariantType::kBlob: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getBlob().getSize())
                   + value.getBlob().getSize();
        }
        default: {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidValueType,
                    static_cast<int>(value.getValueType()));
        }
    }
}

void RequestHandler::writeVariant(
        google::protobuf::io::CodedOutputStream& codedOutput, const Variant& value)
{
    switch (value.getValueType()) {
        case VariantType::kNull: break;
        case VariantType::kBool: {
            const std::uint8_t val = value.getBool() ? 1 : 0;
            codedOutput.WriteRaw(&val, sizeof(val));
            break;
        }
        case VariantType::kInt8: {
            const auto val = value.getInt8();
            codedOutput.WriteRaw(&val, sizeof(val));
            break;
        }
        case VariantType::kUInt8: {
            const auto val = value.getUInt8();
            codedOutput.WriteRaw(&val, sizeof(val));
            break;
        }
        case VariantType::kInt16: {
            const auto val = boost::endian::native_to_little(value.getInt16());
            codedOutput.WriteRaw(&val, sizeof(val));
            break;
        }
        case VariantType::kUInt16: {
            const auto val = boost::endian::native_to_little(value.getUInt16());
            codedOutput.WriteRaw(&val, sizeof(val));
            break;
        }
        case VariantType::kInt32: {
            codedOutput.WriteVarint32(value.getInt32());
            break;
        }
        case VariantType::kUInt32: {
            codedOutput.WriteVarint32(value.getUInt32());
            break;
        }
        case VariantType::kInt64: {
            codedOutput.WriteVarint64(value.getInt64());
            break;
        }
        case VariantType::kUInt64: {
            codedOutput.WriteVarint64(value.getUInt64());
            break;
        }
        case VariantType::kFloat: {
            // Make GCC strict aliasing rules happy
            union {
                float m_floatValue;
                std::uint32_t m_uint32Value;
            } v;
            v.m_floatValue = value.getFloat();
            codedOutput.WriteLittleEndian32(v.m_uint32Value);
            break;
        }
        case VariantType::kDouble: {
            // Make GCC strict aliasing rules happy
            union {
                double m_doubleValue;
                std::uint64_t m_uint64Value;
            } v;
            v.m_doubleValue = value.getDouble();
            codedOutput.WriteLittleEndian64(v.m_uint64Value);
            break;
        }
        case VariantType::kDateTime: {
            protobuf::writeRawDateTime(codedOutput, value.getDateTime());
            break;
        }
        case VariantType::kString: {
            codedOutput.WriteVarint32(value.getString().size());
            codedOutput.WriteRaw(value.getString().data(), value.getString().size());
            break;
        }
        case VariantType::kBinary: {
            codedOutput.WriteVarint32(value.getBinary().size());
            codedOutput.WriteRaw(value.getBinary().data(), value.getBinary().size());
            break;
        }
        case VariantType::kClob: {
            auto clob = value.getClob().clone();
            if (clob == nullptr) {
                throw std::runtime_error("Could not clone CLOB stream");
            }
            auto size = clob->getRemainingSize();
            BinaryValue buffer(std::min(size, kLobChunkSize));
            codedOutput.WriteVarint32(size);
            while (size > 0) {
                auto curChunkSize = std::min(size, kLobChunkSize);
                curChunkSize = clob->read(buffer.data(), curChunkSize);
                size -= curChunkSize;
                codedOutput.WriteRaw(buffer.data(), curChunkSize);
            }
            break;
        }
        case VariantType::kBlob: {
            auto blob = value.getBlob().clone();
            if (blob == nullptr) {
                throw std::runtime_error("Could not clone BLOB stream");
            }
            auto size = blob->getRemainingSize();
            BinaryValue buffer(std::min(size, kLobChunkSize));
            codedOutput.WriteVarint32(size);
            while (size > 0) {
                auto curChunkSize = std::min(size, kLobChunkSize);
                curChunkSize = blob->read(buffer.data(), curChunkSize);
                size -= curChunkSize;
                codedOutput.WriteRaw(buffer.data(), curChunkSize);
            }
            break;
        }
        default: {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidValueType,
                    static_cast<int>(value.getValueType()));
        }
    }
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
                        std::make_unique<requests::ConstantExpression>(Variant(src.m_notNull)));
                break;
            }
            case ConstraintType::kDefaultValue: {
                const auto& src =
                        dynamic_cast<const requests::DefaultValueConstraint&>(*constraint);
                constraintSpecs.emplace_back(std::string(src.m_name), ConstraintType::kNotNull,
                        requests::ExpressionPtr(src.m_value->clone()));
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
    expressions.reserve(kReservedExpressionCount);

    std::vector<std::unordered_map<std::reference_wrapper<const std::string>, int,
            std::hash<std::string>, std::equal_to<std::string>>>
            addedColumns(dataSets.size());

    for (std::size_t i = 0; i < dataSets.size(); ++i) {
        for (std::size_t j = 0; j < dataSets[i]->getColumnCount(); ++j) {
            addedColumns[i].insert(std::make_pair(std::cref(dataSets[i]->getColumnName(j)), j));
        }
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
            auto insertIter =
                    addedColumns[tableIndex].insert(std::make_pair(std::cref(columnName), -1));

            // If column was not added into the dataset
            if (insertIter.second) {
                const auto columnPos =
                        dataSets[tableIndex]->getDataSourceColumnPosition(columnName);
                if (columnPos) {
                    // Required to set column and table indices
                    auto nonConstColumnExpression = stdext::as_mutable_ptr(columnExpression);
                    nonConstColumnExpression->setDatasetTableIndex(tableIndex);
                    const auto newColumnIndex = dataSets[tableIndex]->getColumnCount();
                    nonConstColumnExpression->setDatasetColumnIndex(newColumnIndex);
                    insertIter.first->second = newColumnIndex;
                    dataSets[tableIndex]->emplaceColumnInfo(*columnPos, columnName, std::string());
                } else {
                    errors.push_back(makeDatabaseError(
                            IOManagerMessageId::kErrorColumnIsUnknown, tableName, columnName));
                }
            } else if (insertIter.first->second != -1) {
                const auto nonConstColumnExpression = stdext::as_mutable_ptr(columnExpression);
                nonConstColumnExpression->setDatasetTableIndex(tableIndex);
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

void RequestHandler::checkWhereExpression(
        const requests::ConstExpressionPtr& whereExpression, requests::DatabaseContext& context)
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
