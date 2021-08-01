// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../Column.h"
#include "../Database.h"
#include "../MasterColumnRecord.h"
#include "../Table.h"
#include "../ThrowDatabaseError.h"
#include "../TransactionParameters.h"
#include "../parser/DBExpressionEvaluationContext.h"
#include "../parser/EmptyExpressionEvaluationContext.h"

// Common project headers
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

// STL headers
#include <functional>

namespace siodb::iomgr::dbengine {

void RequestHandler::executeUpdateRequest(
        iomgr_protocol::DatabaseEngineResponse& response, const requests::UpdateRequest& request)
{
    response.set_affected_row_count(0);
    response.set_has_affected_row_count(true);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    if (!isValidDatabaseObjectName(request.m_table.m_name))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table.m_name);

    if (request.m_columns.empty()) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnsListIsEmpty, request.m_database,
                request.m_table.m_name);
    }

    if (request.m_values.empty()) throwDatabaseError(IOManagerMessageId::kErrorValuesListIsEmpty);

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    if (!isValidDatabaseObjectName(request.m_table.m_name))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table.m_name);

    if (database->isSystemTable(request.m_table.m_name)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotUpdateSystemTable, databaseName,
                request.m_table.m_name);
    }

    const auto table = database->findTableChecked(request.m_table.m_name);
    table->checkOperationPermitted(m_userId, PermissionType::kUpdate);

    const auto tableColumns = table->getColumnsOrderedByPosition();

    // Positions of used columns
    std::vector<std::size_t> columnPositions;

    // Contains used columns in 'SET' expression
    // and used to determine duplicated columns
    std::vector<char> columnPresent(tableColumns.size());
    columnPositions.reserve(request.m_columns.size());
    std::vector<CompoundDatabaseError::ErrorRecord> errors;

    const auto tableDataSet = std::make_shared<TableDataSet>(table, request.m_table.m_alias);
    requests::DBExpressionEvaluationContext dbContext(std::vector<DataSetPtr> {tableDataSet});

    for (const auto& columnRef : request.m_columns) {
        if (columnRef.m_column == kMasterColumnName) {
            errors.push_back(makeDatabaseError(IOManagerMessageId::kErrorCannotUpdateMasterColumn,
                    database->getName(), table->getName()));
            continue;
        }

        if (!columnRef.m_table.empty() && columnRef.m_table != request.m_table.m_name
                && columnRef.m_table != request.m_table.m_alias) {
            errors.push_back(
                    makeDatabaseError(IOManagerMessageId::kErrorUpdateTableIsNotEqualToColumnTable,
                            request.m_table.m_name, columnRef.m_table, columnRef.m_column));
            continue;
        }

        const auto it = std::find_if(tableColumns.cbegin(), tableColumns.cend(),
                [&columnRef](const auto& tableColumn) noexcept {
                    return tableColumn->getName() == columnRef.m_column;
                });
        if (it == tableColumns.cend()) {
            errors.push_back(makeDatabaseError(IOManagerMessageId::kErrorColumnDoesNotExist,
                    databaseName, request.m_table.m_name, columnRef.m_column));
        } else {
            const auto& tableColumnRecord = **it;
            tableDataSet->emplaceColumnInfo(
                    tableColumnRecord.getCurrentPosition(), tableColumnRecord.getName(), "");
            const auto& columnPresentFlag =
                    columnPresent.at(tableColumnRecord.getCurrentPosition());
            if (columnPresentFlag) {
                errors.push_back(makeDatabaseError(
                        IOManagerMessageId::kErrorUpdateDuplicateColumnName, columnRef.m_column));
            } else
                columnPositions.push_back(std::distance(tableColumns.begin(), it));
        }
    }

    if (request.m_where)
        updateColumnsFromExpression(dbContext.getDataSets(), request.m_where, errors);

    for (const auto& expr : request.m_values)
        updateColumnsFromExpression(dbContext.getDataSets(), expr, errors);

    if (!errors.empty()) throw CompoundDatabaseError(std::move(errors));

    checkWhereExpression(request.m_where, dbContext);

    try {
        for (const auto& expr : request.m_values)
            expr->validate(dbContext);
    } catch (std::exception& e) {
        throwDatabaseError(IOManagerMessageId::kErrorUpdateInvalidValueExpression, e.what());
    }

    std::uint64_t updatedRowCount = 0;
    for (tableDataSet->resetCursor(); tableDataSet->hasCurrentRow();
            tableDataSet->moveToNextRow()) {
        // Read all columns required for where
        if (request.m_where) {
            try {
                const auto rowFits = request.m_where->evaluate(dbContext);
                if (!rowFits.getBool()) continue;
            } catch (const std::runtime_error& e) {
                // Catch exception from WHERE expression evaluation
                throwDatabaseError(IOManagerMessageId::kErrorInvalidWhereCondition, e.what());
            } catch (const VariantLogicError& error) {
                throwDatabaseError(IOManagerMessageId::kErrorInvalidWhereCondition, error.what());
            }
        }

        std::vector<Variant> values;
        values.reserve(request.m_values.size());
        for (const auto& value : request.m_values)
            values.push_back(value->evaluate(dbContext));

        tableDataSet->updateCurrentRow(std::move(values), columnPositions, m_userId);
        response.set_affected_row_count(++updatedRowCount);
    }

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeDeleteRequest(
        iomgr_protocol::DatabaseEngineResponse& response, const requests::DeleteRequest& request)
{
    response.set_affected_row_count(0);
    response.set_has_affected_row_count(true);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    if (!isValidDatabaseObjectName(request.m_table.m_name))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table.m_name);

    if (database->isSystemTable(request.m_table.m_name)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotDeleteFromSystemTable, databaseName,
                request.m_table.m_name);
    }

    const auto table = database->findTableChecked(request.m_table.m_name);
    table->checkOperationPermitted(m_userId, PermissionType::kDelete);

    const auto tableDataSet = std::make_shared<TableDataSet>(table, request.m_table.m_alias);
    requests::DBExpressionEvaluationContext dbContext(std::vector<DataSetPtr> {tableDataSet});

    std::vector<CompoundDatabaseError::ErrorRecord> errors;
    if (request.m_where)
        updateColumnsFromExpression(dbContext.getDataSets(), request.m_where, errors);
    if (!errors.empty()) throw CompoundDatabaseError(std::move(errors));

    checkWhereExpression(request.m_where, dbContext);

    std::uint64_t deletedRowCount = 0;
    for (tableDataSet->resetCursor(); tableDataSet->hasCurrentRow();
            tableDataSet->moveToNextRow()) {
        if (request.m_where) {
            try {
                const auto rowFits = request.m_where->evaluate(dbContext);
                if (!rowFits.getBool()) continue;
            } catch (const std::runtime_error& ex) {
                // Catch exception from WHERE expression evaluation
                throwDatabaseError(IOManagerMessageId::kErrorInvalidWhereCondition, ex.what());
            } catch (const VariantLogicError& error) {
                throwDatabaseError(IOManagerMessageId::kErrorInvalidWhereCondition, error.what());
            }
        }
        tableDataSet->deleteCurrentRow(m_userId);
        response.set_affected_row_count(++deletedRowCount);
    }

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeInsertRequest(
        iomgr_protocol::DatabaseEngineResponse& response, const requests::InsertRequest& request)
{
    response.set_affected_row_count(0);
    response.set_has_affected_row_count(true);

    const auto& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (!isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, databaseName);

    const auto database = m_instance.findDatabaseChecked(databaseName);
    UseDatabaseGuard databaseGuard(*database);

    if (!isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (database->isSystemTable(request.m_table)) {
        throwDatabaseError(
                IOManagerMessageId::kErrorCannotInsertToSystemTable, databaseName, request.m_table);
    }

    const auto table = database->findTableChecked(request.m_table);
    table->checkOperationPermitted(m_userId, PermissionType::kInsert);

    if (request.m_values.empty()) throwDatabaseError(IOManagerMessageId::kErrorValuesListIsEmpty);

    std::vector<CompoundDatabaseError::ErrorRecord> errors;
    requests::EmptyExpressionEvaluationContext context;

    const bool requestHasColumns = !request.m_columns.empty();

    // Includes TRID
    const auto tableColumns = table->getColumnsOrderedByPosition();
    std::size_t filledColumnsCount = 0;
    if (requestHasColumns)
        filledColumnsCount = request.m_columns.size();
    else {
        // In case of insert with multiple rows, all rows should have the same size
        filledColumnsCount = request.m_values.at(0).size();
        if (filledColumnsCount >= tableColumns.size()) {
            throwDatabaseError(IOManagerMessageId::kErrorTooManyColumnsToInsert, databaseName,
                    request.m_table, filledColumnsCount, tableColumns.size() - 1);
        }
    }

    std::vector<std::string> columnNames;

    if (requestHasColumns) {
        // NOTE: According to tests, this is at least 10% faster than linear search.
        std::unordered_map<std::reference_wrapper<const std::string>, ColumnPtr,
                std::hash<std::string>, std::equal_to<std::string>>
                tableColumnsByName;
        tableColumnsByName.reserve(tableColumns.size());
        for (const auto& tableColumn : tableColumns)
            tableColumnsByName.emplace(std::ref(tableColumn->getName()), tableColumn);

        columnNames.reserve(request.m_columns.size());

        for (const auto& columnName : request.m_columns) {
            if (columnName == kMasterColumnName) {
                errors.push_back(
                        makeDatabaseError(IOManagerMessageId::kErrorCannotInsertIntoMasterColumn));
                continue;
            }

            const auto it = tableColumnsByName.find(columnName);

            if (it == tableColumnsByName.cend()) {
                errors.push_back(makeDatabaseError(IOManagerMessageId::kErrorColumnDoesNotExist,
                        databaseName, request.m_table, columnName));
            } else
                columnNames.push_back(it->first);
        }
    }

    for (std::size_t i = 0; i < request.m_values.size(); ++i) {
        const auto& value = request.m_values[i];
        if (value.size() != filledColumnsCount) {
            const auto errorCode = requestHasColumns
                                           ? IOManagerMessageId::kErrorValuesListNotMatchColumns
                                           : IOManagerMessageId::kErrorValuesListLengthsNotSame;
            errors.push_back(makeDatabaseError(errorCode, request.m_database, request.m_table,
                    filledColumnsCount, i + 1, value.size()));
        }
    }

    if (!errors.empty()) throw CompoundDatabaseError(std::move(errors));

    const TransactionParameters transactionParams(m_userId, database->generateNextTransactionId());

    // Do not include TRID
    const auto requestColumnCount =
            requestHasColumns ? request.m_columns.size() : tableColumns.size() - 1;

    std::vector<Variant> rowValues;
    rowValues.reserve(requestColumnCount);

    std::uint64_t insertedRowCount = 0;
    for (const auto& row : request.m_values) {
        rowValues.clear();
        for (const auto& expression : row)
            rowValues.push_back(expression->evaluate(context));

        if (columnNames.empty())
            table->insertRow(std::move(rowValues), transactionParams);
        else
            table->insertRow(columnNames, std::move(rowValues), transactionParams);

        response.set_affected_row_count(++insertedRowCount);
    }

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

}  // namespace siodb::iomgr::dbengine
