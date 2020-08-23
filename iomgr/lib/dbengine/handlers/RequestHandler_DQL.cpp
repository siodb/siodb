// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../Column.h"
#include "../ColumnSet.h"
#include "../Index.h"
#include "../SystemDatabase.h"
#include "../Table.h"
#include "../TableDataSet.h"
#include "../ThrowDatabaseError.h"
#include "../parser/DatabaseContext.h"
#include "../parser/EmptyContext.h"
#include "../parser/expr/AllColumnsExpression.h"
#include "../parser/expr/SingleColumnExpression.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/bitmask.h>
#include <siodb/common/utils/EmptyString.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

namespace siodb::iomgr::dbengine {

namespace {

bool moveToNextRow(const std::vector<DataSetPtr>& tableDataSets)
{
    const auto firstElement = --tableDataSets.rend();
    for (auto it = tableDataSets.rbegin(); it != tableDataSets.rend() && !(*it)->moveToNextRow();
            ++it) {
        // Reset dataset pointer to the beggining
        if (it != firstElement) (*it)->resetCursor();
    }
    return tableDataSets.front()->hasCurrentRow();
}

}  // namespace

void RequestHandler::executeSelectRequest(
        iomgr_protocol::DatabaseEngineResponse& response, const requests::SelectRequest& request)
{
    response.set_has_affected_row_count(false);

    const std::string& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    const auto database = m_instance.findDatabaseChecked(databaseName);

    std::vector<CompoundDatabaseError::ErrorRecord> errors;

    if (request.m_tables.empty())
        errors.push_back(makeDatabaseError(IOManagerMessageId::kErrorSelectWithoutTables));

    for (const auto& table : request.m_tables) {
        if (!isValidDatabaseObjectName(table.m_name)) {
            errors.push_back(
                    makeDatabaseError(IOManagerMessageId::kErrorInvalidTableName, table.m_name));
        }

        if (!table.m_alias.empty() && !isValidDatabaseObjectName(table.m_alias)) {
            errors.push_back(
                    makeDatabaseError(IOManagerMessageId::kErrorInvalidTableAlias, table.m_alias));
        }
    }

    if (!errors.empty()) throw CompoundDatabaseError(std::move(errors));

    std::unique_ptr<requests::DatabaseContext> dbContext;
    {
        std::vector<DataSetPtr> tableDataSets;
        tableDataSets.reserve(request.m_tables.size());
        for (const auto& table : request.m_tables) {
            tableDataSets.push_back(std::make_shared<TableDataSet>(
                    database->findTableChecked(table.m_name), table.m_alias));
        }
        dbContext = std::make_unique<requests::DatabaseContext>(std::move(tableDataSets));
    }
    const auto& dataSets = dbContext->getDataSets();

    std::vector<std::vector<TableColumn>> tableColumnRecordLists;
    tableColumnRecordLists.reserve(request.m_tables.size());

    // Number of columns to be sent to the server
    std::size_t columnCountToSend = 0;

    for (const auto& tableDataSet : dataSets) {
        const auto table = database->findTableChecked(tableDataSet->getDataSourceId());
        const auto columnSetId = table->getCurrentColumnSetId();
        const auto tableColumns = table->getColumnsOrderedByPosition();
        const auto n = tableColumns.size();
        auto& tableColumnRecords = tableColumnRecordLists.emplace_back();
        tableColumnRecords.reserve(n);
        for (std::size_t i = 0; i < n; ++i)
            tableColumnRecords.emplace_back(tableColumns[i], columnSetId, i);
    }

    std::unordered_set<std::string> knownAliases;

    bool hasNullableColumns = false;
    for (const auto& resultExpr : request.m_resultExpressions) {
        const auto resultExprType = resultExpr.m_expression->getType();

        if (resultExprType == requests::ExpressionType::kAllColumnsReference) {
            if (!resultExpr.m_alias.empty()) {
                errors.push_back(makeDatabaseError(
                        IOManagerMessageId::kErrorCannotUseAllColumnsAlias, resultExpr.m_alias));
                continue;
            }

            auto allColumnsExpression =
                    stdext::as_mutable_ptr(dynamic_cast<const requests::AllColumnsExpression*>(
                            resultExpr.m_expression.get()));
            if (!allColumnsExpression) {
                // Normally should never happen
                throw std::runtime_error(
                        "RequestHandler::executeSelectRequest: allColumnsExpression is nullptr");
            }

            std::size_t tableIndex = 0;
            const auto& tableName = allColumnsExpression->getTableName();
            if (!tableName.empty()) {
                const auto idx = dbContext->getDataSetIndex(tableName);
                if (idx)
                    tableIndex = *idx;
                else {
                    errors.push_back(
                            makeDatabaseError(IOManagerMessageId::kErrorTableDoesNotExistInContext,
                                    databaseName, tableName));
                }
            }

            allColumnsExpression->setDatasetTableIndex(tableIndex);
            for (const auto& tableColumn : tableColumnRecordLists[tableIndex]) {
                addColumnToResponse(response, *tableColumn.m_column);
                dataSets[tableIndex]->emplaceColumnInfo(tableColumn.m_position,
                        tableColumn.m_column->getName(), utils::g_emptyString);
                hasNullableColumns |= !tableColumn.m_column->isNotNull();
            }

            columnCountToSend += tableColumnRecordLists[tableIndex].size();
        } else if (resultExprType == requests::ExpressionType::kSingleColumnReference) {
            auto columnExpression =
                    stdext::as_mutable_ptr(dynamic_cast<const requests::SingleColumnExpression*>(
                            resultExpr.m_expression.get()));

            if (!columnExpression) {
                // Normally should never happen
                throw std::runtime_error(
                        "RequestHandler::executeSelectRequest: columnExpression is nullptr");
            }

            const auto& columnName = columnExpression->getColumnName();
            const auto& tableName = columnExpression->getTableName();
            std::size_t tableIndex = 0;

            if (!isValidDatabaseObjectName(columnName)) {
                errors.push_back(
                        makeDatabaseError(IOManagerMessageId::kErrorInvalidColumnName, columnName));
            }

            // TODO: Support search of duplicated column names in tables when name is not empty.
            if (tableName.empty() && !knownAliases.insert(columnName).second) {
                errors.push_back(makeDatabaseError(
                        IOManagerMessageId::kErrorSelectDuplicateColumnName, columnName));
            }

            if (!resultExpr.m_alias.empty()) {
                if (!isValidDatabaseObjectName(resultExpr.m_alias)) {
                    errors.push_back(makeDatabaseError(
                            IOManagerMessageId::kErrorInvalidColumnAlias, resultExpr.m_alias));
                }

                if (!knownAliases.insert(resultExpr.m_alias).second) {
                    errors.push_back(
                            makeDatabaseError(IOManagerMessageId::kErrorSelectDuplicateColumnAlias,
                                    resultExpr.m_alias));
                }
            }

            if (!tableName.empty()) {
                const auto idx = dbContext->getDataSetIndex(tableName);
                if (!idx) {
                    errors.push_back(
                            makeDatabaseError(IOManagerMessageId::kErrorTableDoesNotExistInContext,
                                    databaseName, tableName));
                    continue;
                }
                tableIndex = *idx;
            }

            // Search for columns
            const auto& tableColumnRecords = tableColumnRecordLists[tableIndex];
            const auto it = std::find_if(tableColumnRecords.begin(), tableColumnRecords.end(),
                    [&columnName](const auto& tableColumn) noexcept {
                        return tableColumn.m_column->getName() == columnName;
                    });
            if (it == tableColumnRecords.end()) {
                errors.push_back(makeDatabaseError(IOManagerMessageId::kErrorColumnDoesNotExist,
                        databaseName, dataSets[tableIndex]->getName(), columnName));
            } else {
                addColumnToResponse(response, *it->m_column, resultExpr.m_alias);
                columnExpression->setDatasetTableIndex(tableIndex);
                columnExpression->setDatasetColumnIndex(dataSets[tableIndex]->getColumnCount());
                dataSets[tableIndex]->emplaceColumnInfo(
                        it->m_position, it->m_column->getName(), resultExpr.m_alias);
                hasNullableColumns |= !it->m_column->isNotNull();
                ++columnCountToSend;
            }
        } else {
            // Expression case
            updateColumnsFromExpression(dataSets, resultExpr.m_expression, errors);

            // getExpectedResultType() does not require column value to be read
            const auto dataType = resultExpr.m_expression->getColumnDataType(*dbContext);
            const auto columnDescription = response.add_column_description();
            columnDescription->set_name(resultExpr.m_alias);

            columnDescription->set_is_null(true);
            columnDescription->set_type(dataType);
            hasNullableColumns = true;
            ++columnCountToSend;
        }
    }

    // Add remaining columns used in the WHERE clause
    if (request.m_where != nullptr) updateColumnsFromExpression(dataSets, request.m_where, errors);
    if (!errors.empty()) throw CompoundDatabaseError(std::move(errors));

    for (auto& tableDataSet : dataSets)
        tableDataSet->resetCursor();

    checkWhereExpression(request.m_where, *dbContext);

    std::optional<std::uint64_t> limit;
    std::optional<std::uint64_t> offset;

    if (request.m_limit != nullptr) {
        requests::EmptyContext emptyContext;
        request.m_limit->validate(emptyContext);

        const auto limitValue = request.m_limit->evaluate(emptyContext);
        if (!limitValue.isInteger())
            throwDatabaseError(IOManagerMessageId::kErrorLimitValueTypeNotInteger);

        if (limitValue.isNegative())
            throwDatabaseError(IOManagerMessageId::kErrorLimitValueIsNegative);

        limit = limitValue.asUInt64();
        if (request.m_offset != nullptr) {
            request.m_offset->validate(emptyContext);

            const auto offsetValue = request.m_offset->evaluate(emptyContext);
            if (!offsetValue.isInteger())
                throwDatabaseError(IOManagerMessageId::kErrorOffsetValueTypeNotInteger);

            if (offsetValue.isNegative())
                throwDatabaseError(IOManagerMessageId::kErrorOffsetValueIsNegative);

            offset = offsetValue.asUInt64();
        }
    }

    utils::DefaultErrorCodeChecker errorChecker;
    protobuf::StreamOutputStream rawOutput(m_connection, errorChecker);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);

    google::protobuf::io::CodedOutputStream codedOutput(&rawOutput);

    try {
        bool rowDataAvailable = true;
        for (auto& tableDataSet : dataSets) {
            rowDataAvailable &= tableDataSet->hasCurrentRow();
            if (!rowDataAvailable) break;
        }

        stdext::bitmask nullMask;
        if (hasNullableColumns) nullMask.resize(columnCountToSend);

        std::vector<Variant> values(columnCountToSend);

        //std::uint64_t rowNumber = 0;
        while (rowDataAvailable && (!limit.has_value() || *limit > 0)) {
            std::size_t rowLength = 0;
            if (request.m_where) {
                try {
                    if (isNullType(request.m_where->getResultValueType(*dbContext))) {
                        rowDataAvailable = moveToNextRow(dataSets);
                        continue;
                    }

                    const auto rowFits = request.m_where->evaluate(*dbContext);
                    if (!rowFits.getBool()) {
                        rowDataAvailable = moveToNextRow(dataSets);
                        continue;
                    }
                } catch (const std::runtime_error& e) {
                    // Catch exception from WHERE expression evaluation
                    throwDatabaseError(IOManagerMessageId::kErrorInvalidWhereCondition, e.what());
                } catch (const VariantLogicError& error) {
                    throwDatabaseError(
                            IOManagerMessageId::kErrorInvalidWhereCondition, error.what());
                }
            }

            if (offset && *offset > 0) {
                --(*offset);
                rowDataAvailable = moveToNextRow(dataSets);
                continue;
            }

            std::size_t valueIndex = 0;
            for (const auto& expr : request.m_resultExpressions) {
                const auto exprType = expr.m_expression->getType();
                if (exprType == requests::ExpressionType::kAllColumnsReference) {
                    const auto allColumnsExpression =
                            dynamic_cast<const requests::AllColumnsExpression*>(
                                    expr.m_expression.get());
                    const auto tableIndex = *allColumnsExpression->getDatasetTableIndex();
                    for (const auto& rowValue : dataSets[tableIndex]->getCurrentRow()) {
                        auto& value = values[valueIndex];
                        value = rowValue;
                        rowLength += getVariantSize(value);
                        if (hasNullableColumns) nullMask.set(valueIndex, value.isNull());
                        ++valueIndex;
                    }
                } else {
                    auto& value = values[valueIndex];
                    value = expr.m_expression->evaluate(*dbContext);
                    rowLength += getVariantSize(value);
                    if (hasNullableColumns) nullMask.set(valueIndex, value.isNull());
                    ++valueIndex;
                }
            }

            rowLength += nullMask.size();

            codedOutput.WriteVarint64(rowLength);
            rawOutput.CheckNoError();

            //DBG_LOG_DEBUG(">>> OUTPUT: Row# " << rowNumber << ": Length=" << rowLength);

            if (hasNullableColumns) {
                codedOutput.WriteRaw(nullMask.data(), nullMask.size());
                rawOutput.CheckNoError();
            }

            for (std::size_t i = 0; i < columnCountToSend; ++i) {
                writeVariant(codedOutput, values[i]);
                rawOutput.CheckNoError();
            }

            if (limit) --(*limit);
            rowDataAvailable = moveToNextRow(dataSets);
            //++rowNumber;
        }
    } catch (DatabaseError& ex) {
        LOG_ERROR << kLogContext << ex.what();
        // DatabaseError exception is only possible before data serialization and writing,
        // so data shouldn't be sent to the server at that moment.
        // All other exceptions are caught on the upper level.
        codedOutput.WriteVarint64(kNoMoreRows);
        rawOutput.CheckNoError();

        // NOTE: Do not re-throw here to prevent double response.
    }

    codedOutput.WriteVarint64(kNoMoreRows);
    rawOutput.CheckNoError();
}

void RequestHandler::executeShowDatabasesRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::ShowDatabasesRequest& request)
{
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);

    auto& systemDatabase = m_instance.getSystemDatabase();
    const auto sysDbTable = systemDatabase.findTableChecked(kSysDatabasesTableName);
    const auto nameColumn = sysDbTable->findColumnChecked(kSysDatabases_Name_ColumnName);
    const auto uuidColumn = sysDbTable->findColumnChecked(kSysDatabases_Uuid_ColumnName);

    addColumnToResponse(response, *nameColumn, "");
    addColumnToResponse(response, *uuidColumn, "");

    const bool nullNotAllowed = nameColumn->isNotNull() && uuidColumn->isNotNull();

    const auto databaseRecords = m_instance.getDatabaseRecordsOrderedByName();

    utils::DefaultErrorCodeChecker errorChecker;
    protobuf::StreamOutputStream rawOutput(m_connection, errorChecker);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);

    std::vector<Variant> values(2);
    stdext::bitmask nullMask;
    if (!nullNotAllowed) nullMask.resize(values.size(), false);

    google::protobuf::io::CodedOutputStream codedOutput(&rawOutput);
    for (const auto& dbRecord : databaseRecords) {
        values[0] = dbRecord.m_name;
        values[1] = boost::uuids::to_string(dbRecord.m_uuid);

        if (!nullNotAllowed) {
            nullMask.set(0, values[0].isNull());
            nullMask.set(1, values[1].isNull());
        }

        const std::size_t rowSize =
                getVariantSize(values[0]) + getVariantSize(values[1]) + nullMask.size();

        codedOutput.WriteVarint64(rowSize);

        if (!nullNotAllowed) {
            codedOutput.WriteRaw(nullMask.data(), nullMask.size());
            rawOutput.CheckNoError();
        }

        rawOutput.CheckNoError();

        for (std::size_t i = 0; i < values.size(); ++i) {
            writeVariant(codedOutput, values[i]);
            rawOutput.CheckNoError();
        }
    }

    codedOutput.WriteVarint64(kNoMoreRows);
    rawOutput.CheckNoError();
}

}  // namespace siodb::iomgr::dbengine
