// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SqlDump.h"
#include "internal/SqlDumpInternal.h"

// Project headers
#include "SqlQueryException.h"

// Common project headers
#include <siodb/common/crt_ext/compiler_defs.h>
#include <siodb/common/data/RawDateTime.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>
#include <siodb/common/stl_ext/bitmask.h>

// STL headers
#include <iomanip>
#include <sstream>

// Protobuf headers
#include <google/protobuf/io/zero_copy_stream_impl.h>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>

// utf8cpp headers
#include <utf8cpp/utf8.h>

namespace siodb::siocli {

void dumpAllDatabases(io::InputOutputStream& connection, std::ostream& os)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;
    protobuf::StreamInputStream input(connection, errorCodeChecker);
    const auto databases = detail::readDatabases(connection, input);
    bool addLeadingNewline = false;
    for (const auto& dbInfo : databases) {
        if (addLeadingNewline) os << '\n';
        addLeadingNewline = true;
        detail::dumpCreateDatabase(dbInfo, os);
        detail::dumpData(connection, input, dbInfo.m_name, std::string(), os);
    }
    os << std::flush;
}

void dumpSingleDatabase(
        io::InputOutputStream& connection, const std::string& databaseName, std::ostream& os)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;
    protobuf::StreamInputStream input(connection, errorCodeChecker);
    detail::dumpCreateDatabase(connection, input, databaseName, os);
    detail::dumpData(connection, input, databaseName, std::string(), os);
    os << std::flush;
}

void dumpSingleTable(io::InputOutputStream& connection, const std::string& databaseName,
        const std::string& tableName, std::ostream& os)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;
    protobuf::StreamInputStream input(connection, errorCodeChecker);
    detail::dumpData(connection, input, databaseName, tableName, os);
    os << std::flush;
}

namespace detail {

void dumpData(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::string& tableName, std::ostream& os)
{
    auto tables = readTables(connection, input, databaseName);
    if (!tableName.empty()) {
        const auto upperTableName = boost::to_upper_copy(tableName);
        tables.erase(std::remove_if(tables.begin(), tables.end(),
                             [&upperTableName](const auto& tableInfo) noexcept {
                                 return tableInfo.m_name != upperTableName;
                             }),
                tables.end());
        if (tables.empty()) {
            std::cerr << "Table " << databaseName << '.' << tableName << " doesn't exist."
                      << std::endl;
            return;
        }
    }

    dumpTables(connection, input, databaseName, tables, os);
}

void dumpTables(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::vector<TableInfo>& tableInfos, std::ostream& os)
{
    for (auto& table : tableInfos) {
        dumpTableDefinition(databaseName, table, os);
        os << '\n';
        dumpTableData(connection, input, databaseName, table, os);
    }
}

void dumpTableDefinition(const std::string& databaseName, const TableInfo& table, std::ostream& os)
{
    os << '\n' << buildCreateTableStatement(databaseName, table) << '\n';
}

void dumpTableData(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const TableInfo& table, std::ostream& os)
{
    std::ostringstream ss;
    ss << buildSelectAllStatement(databaseName, table.m_name);
    auto response = sendCommand(ss.str(), connection, input);

    std::vector<TableInfo> tableInfos;
    tableInfos.reserve(32);

    bool nullAllowed = false;
    const auto columnCount = response.column_description_size();
    for (int i = 0; i < columnCount; ++i) {
        const auto& column = response.column_description(i);
        nullAllowed |= column.is_null();
    }

    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::uint64_t expectedTrid = 1;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error("dumpTableData: Read row length failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        stdext::bitmask nullBitmask;

        if (nullAllowed) {
            nullBitmask.resize(columnCount, false);
            if (SIODB_UNLIKELY(!codedInput.ReadRaw(nullBitmask.data(), nullBitmask.size()))) {
                std::ostringstream err;
                err << "dumpTableData: Read null bitmask from server failed: "
                    << std::strerror(input.GetErrno());
                throw std::system_error(input.GetErrno(), std::generic_category(), err.str());
            }
        }

        std::uint64_t trid = 0;
        if (SIODB_UNLIKELY(!codedInput.Read(&trid)))
            throw std::runtime_error("dumpTableData: Read TRID failed");

        if (expectedTrid != trid) {
            os << buildAlterTableSetNextTridStatement(databaseName, table.m_name, trid) << ';'
               << '\n';
            expectedTrid = trid + 1;
        } else {
            ++expectedTrid;
        }

        os << "INSERT INTO " << databaseName << '.' << table.m_name << " (";
        bool insertComma = false;
        for (const auto& column : table.m_columns) {
            if (insertComma) os << ", ";
            insertComma = true;
            os << column.m_name;
        }
        os << ")\nVALUES (";

        for (std::size_t i = 0; i < table.m_columns.size(); ++i) {
            if (i > 0) os << ", ";
            if (nullAllowed && nullBitmask.get(i + 1))  // Skip TRID
                os << "NULL";
            else {
                std::string value;
                if (SIODB_UNLIKELY(!readValue(codedInput, table.m_columns.at(i).m_dataType, value)))
                    throw std::runtime_error("Can't read value");
                os << value;
            }
        }

        os << ");\n";
    }
}

std::vector<DatabaseInfo> readDatabases(
        io::InputOutputStream& connection, protobuf::StreamInputStream& input)
{
    auto query = buildSelectStatementCore(kSystemDatabaseName, kSysDatabasesTableName,
            {kSysDatabases_Name_ColumnName, kSysDatabases_CipherId_ColumnName});
    const auto response = sendCommand(std::move(query), connection, input);

    std::vector<DatabaseInfo> databases;
    databases.reserve(16);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength))
            std::runtime_error("readDatabases: Read row length failed");

        if (rowLength == 0) break;

        DatabaseInfo database;
        if (SIODB_LIKELY(
                    codedInput.Read(&database.m_name) && codedInput.Read(&database.m_cipherId))) {
            if (database.m_name != kSystemDatabaseName) databases.push_back(std::move(database));
        } else
            std::runtime_error("readDatabases: Read database record failed");
    }

    return databases;
}

void dumpCreateDatabase(const DatabaseInfo& dbInfo, std::ostream& os)
{
    os << "\n-- Database: " << dbInfo.m_name << '\n'
       << buildCreateDatabaseStatement(dbInfo) << '\n';
}

void dumpCreateDatabase(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, std::ostream& os)
{
    std::ostringstream ss;
    ss << buildSelectStatementCore(kSystemDatabaseName, kSysDatabasesTableName,
            {kSysDatabases_Name_ColumnName, kSysDatabases_CipherId_ColumnName})
       << " WHERE " << kSysDatabases_Name_ColumnName << "='" << databaseName << '\'';

    const auto response = sendCommand(ss.str(), connection, input);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::uint64_t rowLength = 0;
    if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
        std::runtime_error("dumpCreateDatabase: Read row length failed");

    if (SIODB_UNLIKELY(rowLength == 0))
        std::runtime_error("dumpCreateDatabase: Database doesn't exist");

    // nulls are dissalowed, read values
    DatabaseInfo database;
    if (SIODB_LIKELY(codedInput.Read(&database.m_name) && codedInput.Read(&database.m_cipherId))) {
        if (database.m_name != kSystemDatabaseName) dumpCreateDatabase(database, os);
    } else
        std::runtime_error("dumpCreateDatabase: Read database record failed");

    if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
        std::runtime_error("dumpCreateDatabase: Read row length failed");
    if (SIODB_UNLIKELY(rowLength != 0))
        std::runtime_error("dumpCreateDatabase: Invalid row length");
}

std::vector<TableInfo> readTables(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName)
{
    // SELECT TRID,NAME,CURRENT_COLUMN_SET_ID FROM <DATABASE>.SYS_TABLES
    // WHERE NAME NOT LIKE 'SYS_%'"
    std::ostringstream ss;
    ss << buildSelectStatementCore(databaseName, kSysTablesTableName,
            {kMasterColumnName, kSysTables_Name_ColumnName,
                    kSysTables_CurrentColumnSetId_ColumnName})
       << " WHERE " << kSysTables_Name_ColumnName << " NOT LIKE 'SYS_%' AND "
       << kSysTables_Type_ColumnName << "=1";
    auto response = sendCommand(ss.str(), connection, input);

    std::vector<TableInfo> tableInfos;
    tableInfos.reserve(32);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error("getTablesList: Read row length failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        TableInfo tableInfo;
        if (SIODB_LIKELY(codedInput.Read(&tableInfo.m_trid) && codedInput.Read(&tableInfo.m_name)
                         && codedInput.Read(&tableInfo.m_currentColumnSetId))) {
            tableInfos.push_back(std::move(tableInfo));
        } else
            throw std::runtime_error("readTables: Read table record failed");
    }

    for (auto& tableInfo : tableInfos) {
        tableInfo.m_columns =
                readColumns(connection, input, databaseName, tableInfo.m_currentColumnSetId);
    }

    return tableInfos;
}

std::vector<ColumnInfo> readColumns(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName,
        std::int64_t currentColumnSetId)
{
    // SELECT TRID, COLUMN_ID <database-name>.SYS_COLUMN_SET_COLUMNS
    // WHERE COLUMN_SET_ID = value of <CURRENT_COLUMN_SET> in the SYS_TABLES
    std::ostringstream ss;
    ss << buildSelectStatementCore(databaseName, kSysColumnSetColumnsTableName,
            {kMasterColumnName, kSysColumnSetColumns_ColumnDefinitionId_ColumnName})
       << " WHERE " << kSysColumnSetColumns_ColumnSetId_ColumnName << " = " << currentColumnSetId;

    auto response = sendCommand(ss.str(), connection, input);
    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::vector<ColumnSetInfo> columnSetInfos;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error("readColumns: Read SYS_COLUMN_SET_COLUMNS row failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        // Nulls are not allowed, read values
        ColumnSetInfo columnSetInfo;
        if (SIODB_LIKELY(codedInput.Read(&columnSetInfo.m_trid)
                         && codedInput.Read(&columnSetInfo.m_columnDefinitionId)))
            columnSetInfos.push_back(std::move(columnSetInfo));
        else
            throw std::runtime_error("readColumns: Read column set record failed");
    }

    if (columnSetInfos.empty()) return std::vector<ColumnInfo>();

    // SELECT TRID, COLUMN_ID from <database-name>.SYS_COLUMN_DEFS
    // WHERE TRID IN (list of selected above m_columnDefinitionId);
    ss.str("");
    ss << buildSelectStatementCore(databaseName, kSysColumnDefsTableName,
            {kMasterColumnName, kSysColumnDefs_ColumnId_ColumnName})
       << " WHERE " << kMasterColumnName << " IN (";
    for (auto i = 0u; i < columnSetInfos.size() - 1; ++i)
        ss << columnSetInfos[i].m_columnDefinitionId << ',';
    ss << columnSetInfos.back().m_columnDefinitionId << ')';
    response = sendCommand(ss.str(), connection, input);

    std::unordered_map<std::uint64_t, std::uint64_t> columnIdToColumnDefIdMap;
    std::vector<ColumnDefinitionInfo> columnDefInfos;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error("readColumns: Read SYS_COLUMN_DEFS row length failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        // Nulls are not allowed, read values
        ColumnDefinitionInfo columnDefInfo;
        if (SIODB_LIKELY(codedInput.Read(&columnDefInfo.m_trid)
                         && codedInput.Read(&columnDefInfo.m_columnId))) {
            columnIdToColumnDefIdMap[columnDefInfo.m_columnId] = columnDefInfo.m_trid;
            columnDefInfos.push_back(std::move(columnDefInfo));
        } else
            throw std::runtime_error("readColumns: Read column definition record failed");
    }

    // SELECT TRID, DATA_TYPE, NAME FROM <database-name>.SYS_COLUMNS
    // WHERE TRID IN (... list of retrieved above COLUMN_IDs ... )
    ss.str("");
    ss << buildSelectStatementCore(databaseName, kSysColumnsTableName,
            {kMasterColumnName, kSysColumns_DataType_ColumnName, kSysColumns_Name_ColumnName})
       << " WHERE " << kMasterColumnName << " IN (";
    for (auto i = 0u; i < columnDefInfos.size() - 1; ++i)
        ss << columnDefInfos[i].m_columnId << ',';
    ss << columnDefInfos.back().m_columnId << ')';
    response = sendCommand(ss.str(), connection, input);

    std::vector<ColumnInfo> columns;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error("readColumns: Read SYS_COLUMNS row length failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        // Nulls are not allowed, read values
        ColumnInfo columnInfo;
        std::uint32_t n = 0;
        if (SIODB_LIKELY(codedInput.Read(&columnInfo.m_trid) && codedInput.Read(&n)
                         && codedInput.Read(&columnInfo.m_name))) {
            columnInfo.m_dataType = static_cast<ColumnDataType>(n);
            if (columnInfo.m_name != kMasterColumnName) {
                columnInfo.m_columnDefinitionId = columnIdToColumnDefIdMap[columnInfo.m_trid];
                columns.push_back(std::move(columnInfo));
            }
        } else
            throw std::runtime_error("readColumns: Read column record failed");
    }

    std::sort(columns.begin(), columns.end(), [](const auto& left, const auto& right) noexcept {
        return left.m_columnDefinitionId < right.m_columnDefinitionId;
    });

    for (auto& column : columns) {
        column.m_constraints =
                readColumnConstraints(connection, input, databaseName, column.m_columnDefinitionId);
    }

    return columns;
}

std::vector<ColumnConstraint> readColumnConstraints(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName,
        std::int64_t columnSetId)
{
    // SELECT CONSTRAINT_ID FROM <database-name>SYS_COLUMN_DEF_CONSTRAINTS
    // WHERE COLUMN_DEF_ID = value of the earlier selected SYS_COLUMN_SET_COLUMNS.COLUMN_DEF_ID
    std::ostringstream ss;
    ss << buildSelectStatementCore(databaseName, kSysColumnDefConstraintsTableName,
            {kSysColumnDefinitionConstraintList_ConstraintId_ColumnName})
       << " WHERE " << kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnName << '='
       << columnSetId;

    auto response = sendCommand(ss.str(), connection, input);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::vector<std::uint64_t> constaintIds;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error(
                    "readColumnConstraints: Read SYS_COLUMN_SET_COLUMNS row length failed");

        if (rowLength == 0) break;

        std::uint64_t constaintId = 0;
        if (SIODB_LIKELY(codedInput.Read(&constaintId)))
            constaintIds.push_back(constaintId);
        else
            throw std::runtime_error("Read constraint ID failed");
    }

    if (constaintIds.empty()) return std::vector<ColumnConstraint>();

    // SELECT NAME, CONSTRAINT_DEF_ID FROM .SYS_CONSTRAINTS
    // WHERE TRID IN (list of selected above CONSTRAINT_ID)
    ss.str("");
    ss << buildSelectStatementCore(databaseName, kSysConstraintsTableName,
            {kSysConstraints_Name_ColumnName, kSysConstraints_DefinitionId_ColumnName})
       << " WHERE " << kMasterColumnName << " IN (";
    for (auto i = 0u; i < constaintIds.size() - 1; ++i)
        ss << constaintIds[i] << ',';
    ss << constaintIds.back() << ')';

    response = sendCommand(ss.str(), connection, input);
    std::vector<ColumnConstraint> constraints;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error("readColumnConstraints: Read SYS_CONSTRAINTS row length failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        ColumnConstraint constraint;
        if (SIODB_LIKELY(codedInput.Read(&constraint.m_name)
                         && codedInput.Read(&constraint.m_constraintDefinitionId)))
            constraints.push_back(std::move(constraint));
        else
            throw std::runtime_error("readColumnConstraints: Read constraint record failed");
    }

    for (auto& constraint : constraints) {
        // SELECT TYPE, EXPR FROM <database-name>.SYS_CONSTRAINT_DEFS
        // WHERE TRID = selected above CONSTRAINT_DEF_ID
        ss.str("");
        ss << buildSelectStatementCore(databaseName, kSysConstraintDefsTableName,
                {kSysConstraintDefs_Type_ColumnName, kSysConstraintDefs_Expr_ColumnName})
           << " WHERE " << kMasterColumnName << '=' << constraint.m_constraintDefinitionId;
        response = sendCommand(ss.str(), connection, input);

        const int columnCount = response.column_description_size();

        while (true) {
            std::uint64_t rowLength = 0;
            if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
                std::runtime_error(
                        "readColumnConstraints: Read SYS_CONSTRAINT_DEFS row length failed");

            if (SIODB_UNLIKELY(rowLength == 0)) break;

            stdext::bitmask nullBitmask;

            // EXPR is allowed to be null
            nullBitmask.resize(columnCount, false);
            if (SIODB_LIKELY(codedInput.ReadRaw(nullBitmask.data(), nullBitmask.size()))) {
                std::uint32_t n = 0;
                if (SIODB_LIKELY(codedInput.Read(&n)))
                    constraint.m_type = static_cast<ConstraintType>(n);
                else {
                    throw std::runtime_error("readColumnConstraints: Read constraint type failed");
                }
                if (!nullBitmask.get(1)) {
                    if (SIODB_UNLIKELY(!codedInput.Read(&constraint.m_expression))) {
                        throw std::runtime_error(
                                "readColumnConstraints: Read constraint expression failed");
                    }
                }
            } else {
                std::ostringstream err;
                err << "Read read null bitmask failed: " << std::strerror(input.GetErrno());
                throw std::system_error(input.GetErrno(), std::generic_category(), err.str());
            }
        }
    }

    return constraints;
}

client_protocol::ServerResponse sendCommand(std::string&& command,
        io::InputOutputStream& connection, protobuf::StreamInputStream& input)
{
    static std::int64_t requestId = 0;
    client_protocol::Command clientCommand;
    clientCommand.set_request_id(requestId);
    clientCommand.set_text(std::move(command));
    protobuf::writeMessage(protobuf::ProtocolMessageType::kCommand, clientCommand, connection);

    client_protocol::ServerResponse response;
    protobuf::readMessage(protobuf::ProtocolMessageType::kServerResponse, response, input);
    checkResponse(response);
    ++requestId;

    return response;
}

void checkResponse(const client_protocol::ServerResponse& response)
{
    const auto messageCount = response.message_size();
    if (messageCount > 0) {
        std::vector<siodb::StatusMessage> errors;
        errors.reserve(messageCount);
        for (int i = 0; i < messageCount; ++i) {
            const auto& message = response.message(i);
            if (message.status_code() != 0) errors.push_back(message);
        }
        if (!errors.empty()) throw SqlQueryException(std::move(errors));
    }
}

bool readValue(protobuf::ExtendedCodedInputStream& codedInput, ColumnDataType columnDataType,
        std::string& result)
{
    switch (columnDataType) {
        case ColumnDataType::COLUMN_DATA_TYPE_BOOL: {
            bool value = false;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = value ? "true" : "false";
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_INT8: {
            std::int8_t value = 0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(static_cast<int>(value));
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_UINT8: {
            std::uint8_t value = 0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(static_cast<unsigned>(value));
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_INT16: {
            std::int16_t value = 0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(value);
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_UINT16: {
            std::uint16_t value = 0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(value);
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_INT32: {
            std::int32_t value = 0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(value);
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_UINT32: {
            std::uint8_t value = 0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(value);
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_INT64: {
            std::int64_t value = 0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(value);
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_UINT64: {
            std::uint64_t value = 0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(value);
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_FLOAT: {
            float value = 0.0f;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(value);
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_DOUBLE: {
            double value = 0.0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            result = std::to_string(value);
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_TEXT: {
            std::string s;
            if (SIODB_UNLIKELY(!codedInput.Read(&s))) return false;
            std::ostringstream oss;
            oss << '\'';
            for (const auto ch : s) {
                oss << ch;
                if (ch == '\'') oss << ch;
            }
            oss << '\'';
            result = oss.str();
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_BINARY: {
            siodb::BinaryValue bv;
            if (SIODB_UNLIKELY(!codedInput.Read(&bv))) return false;
            std::ostringstream oss;
            oss << "X'" << std::setfill('0') << std::hex;
            for (std::size_t i = 0; i < bv.size(); ++i) {
                const std::uint16_t v = bv[i];
                oss << std::setw(2) << v;
            }
            oss << '\'';
            result = oss.str();
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_TIMESTAMP: {
            siodb::RawDateTime dateTime;
            if (!siodb::protobuf::readRawDateTime(codedInput, dateTime)) return false;
            std::ostringstream oss;
            oss << '\'' << dateTime.formatDefault() << '\'';
            result = oss.str();
            return true;
        }

        default: return false;
    }
}

std::string buildConstraintDefinition(const ColumnConstraint& constraint)
{
    // Only NOT NULL constraint is supported for now
    if (constraint.m_type != ConstraintType::kNotNull) return std::string();
    return getConstraintTypeName(constraint.m_type);
}

std::string buildSelectStatementCore(const std::string& databaseName, const std::string& tableName,
        const std::vector<std::string>& ColumnNames)
{
    return buildSelectStatementCore(databaseName.c_str(), tableName.c_str(), ColumnNames);
}

std::string buildSelectStatementCore(const char* databaseName, const char* tableName,
        const std::vector<std::string>& columnNames)
{
    std::ostringstream oss;
    oss << "SELECT ";
    for (auto i = 0u; i < columnNames.size() - 1; ++i)
        oss << columnNames[i] << ',';
    if (!columnNames.empty()) oss << columnNames.back();
    oss << " FROM " << databaseName << '.' << tableName;
    return oss.str();
}

std::string buildSelectAllStatement(const std::string& databaseName, const std::string& tableName)
{
    std::ostringstream oss;
    oss << "SELECT * FROM " << databaseName << '.' << tableName;
    return oss.str();
}

std::string buildCreateDatabaseStatement(const DatabaseInfo& dbInfo)
{
    std::ostringstream oss;
    oss << "CREATE DATABASE " << dbInfo.m_name;
    if (!dbInfo.m_cipherId.empty()) {
        oss << "\nWITH " << kSysDatabases_CipherId_ColumnName << "='" << dbInfo.m_cipherId << '\'';
    }
    oss << ';';
    return oss.str();
}

std::string buildCreateTableStatement(const std::string& databaseName, const TableInfo& tableInfo)
{
    std::ostringstream oss;
    oss << "-- Table: " << databaseName << '.' << tableInfo.m_name << "\nCREATE TABLE "
        << databaseName << '.' << tableInfo.m_name << " (\n";
    if (!tableInfo.m_columns.empty()) {
        bool insertComma = false;
        for (const auto& column : tableInfo.m_columns) {
            if (insertComma) oss << ",\n";
            insertComma = true;
            oss << "  " << column.m_name << ' ' << getColumnDataTypeName(column.m_dataType);
            for (const auto& constraint : column.m_constraints)
                oss << ' ' << buildConstraintDefinition(constraint);
        }
    }
    oss << "\n)";
    oss << ';';
    return oss.str();
}

std::string buildAlterTableSetNextTridStatement(
        const std::string& databaseName, const std::string& tableName, std::uint64_t nextTrid)
{
    std::ostringstream oss;
    oss << "ALTER TABLE " << databaseName << '.' << tableName << " SET NEXT_TRID=" << nextTrid;
    return oss.str();
}

}  // namespace detail

}  // namespace siodb::siocli
