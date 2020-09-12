// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "SqlDump.h"
#include "SqlQueryException.h"

// Common project headers
#include <siodb/common/crt_ext/compiler_defs.h>
#include <siodb/common/data/RawDateTime.h>
#include <siodb/common/protobuf/ExtendedCodedInputStream.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>
#include <siodb/common/stl_ext/bitmask.h>
#include <siodb/common/utils/BinaryValue.h>
#include <siodb/iomgr/shared/dbengine/ColumnDataType.h>
#include <siodb/iomgr/shared/dbengine/ConstraintType.h>
#include <siodb/iomgr/shared/dbengine/SystemObjectNames.h>

// STL headers
#include <iomanip>
#include <sstream>

// Protobuf message headers
#include <siodb/common/proto/ClientProtocol.pb.h>

// Protobuf headers
#include <google/protobuf/io/zero_copy_stream_impl.h>

// utf8cpp headers
#include <utf8cpp/utf8.h>

namespace siodb::siocli {

namespace {

using namespace siodb::iomgr::dbengine;

/**
 * Database information
 */
struct DatabaseInfo {
    /** Initializes structure DatabaseInfo. */
    DatabaseInfo() noexcept
    {
        // Make GCC-8 happy
    }

    /** Database name */
    std::string m_name;

    /** Cipher ID of the database */
    std::string m_cipherId;

    /** Cipher key of the database */
    siodb::BinaryValue m_cipherKey;
};

/**
 * Column constraint information
 */
struct ColumnConstraint {
    /** Initializes structure ColumnConstraint. */
    ColumnConstraint() noexcept
        : m_type(ConstraintType::kNotNull)
        , m_constraintDefinitionId(0)
    {
    }

    /** Constraint type */
    ConstraintType m_type;

    /** Constraint name */
    std::string m_name;

    /** Constraint definition id */
    std::uint64_t m_constraintDefinitionId;

    /** Constraint expression */
    siodb::BinaryValue m_expression;
};

/**
 * Table column information
 */
struct ColumnInfo {
    /** Initializes structure ColumnInfo. */
    ColumnInfo() noexcept
        : m_trid(0)
        , m_dataType(ColumnDataType::COLUMN_DATA_TYPE_BOOL)
        , m_columnDefinitionId(0)
    {
    }

    /** Column TRID */
    std::uint64_t m_trid;

    /** Column name */
    std::string m_name;

    /** Column data type */
    ColumnDataType m_dataType;

    /** Column definition ID */
    std::uint64_t m_columnDefinitionId;

    /** Constraints of this column */
    std::vector<ColumnConstraint> m_constraints;
};

/**
 * Column set information
 */
struct ColumnSetInfo {
    /** Initializes structure ColumnInfo. */
    ColumnSetInfo() noexcept
        : m_trid(0)
        , m_columnDefinitionId(0)
    {
    }

    /** Column set TRID */
    std::uint64_t m_trid;

    /** Column definition ID */
    std::uint64_t m_columnDefinitionId;
};

/**
 * Column definition information
 */
struct ColumnDefinitionInfo {
    /** Initializes structure ColumnDefinitionInfo. */
    ColumnDefinitionInfo() noexcept
        : m_trid(0)
        , m_columnId(0)
    {
    }

    /** Column definition TRID */
    std::uint64_t m_trid;

    /** Column ID */
    std::uint64_t m_columnId;
};

/**
 * Table information
 */
struct TableInfo {
    /** Initializes structure TableInfo. */
    TableInfo() noexcept
        : m_trid(0)
        , m_currentColumnSetId(0)
    {
    }

    /** Table TRID */
    std::uint64_t m_trid;

    /** Current column set ID */
    std::uint64_t m_currentColumnSetId;

    /** Table name */
    std::string m_name;

    /** Columns related to this table */
    std::vector<ColumnInfo> m_columns;
};

std::string constraintToString(const ColumnConstraint& constraint)
{
    // Only NOT NULL constraint is supported for now
    if (constraint.m_type != ConstraintType::kNotNull) return std::string();
    return getConstraintTypeName(constraint.m_type);
}

std::string formSelectCoreBody(const char* databaseName, const char* tableName,
        const std::vector<std::string>& columnNames)
{
    std::ostringstream ss;
    ss << "SELECT ";
    for (auto i = 0u; i < columnNames.size() - 1; ++i)
        ss << columnNames[i] << ',';
    if (!columnNames.empty()) ss << columnNames.back();
    ss << " FROM " << databaseName << '.' << tableName;
    return ss.str();
}

std::string formSelectCoreBody(const std::string& databaseName, const std::string& tableName,
        const std::vector<std::string>& ColumnNames)
{
    return formSelectCoreBody(databaseName.c_str(), tableName.c_str(), ColumnNames);
}

std::string formSelectAllQuery(const std::string& databaseName, const std::string& tableName)
{
    std::ostringstream ss;
    ss << "SELECT * FROM " << databaseName << '.' << tableName;
    return ss.str();
}

std::string formCreateDatabaseQuery(const DatabaseInfo& dbInfo)
{
    std::ostringstream ss;
    ss << "CREATE DATABASE " << dbInfo.m_name;
    if (!dbInfo.m_cipherId.empty()) {
        ss << " WITH " << kSysDatabases_CipherId_ColumnName << " = '" << dbInfo.m_cipherId << '\'';
    }
    return ss.str();
}

std::string formCreateTableQuery(const std::string& databaseName, const TableInfo& tableInfo)
{
    std::ostringstream ss;
    ss << "CREATE TABLE " << databaseName << '.' << tableInfo.m_name;
    if (!tableInfo.m_columns.empty()) {
        ss << " (";
        for (auto i = 0u; i < tableInfo.m_columns.size() - 1; ++i) {
            const auto& column = tableInfo.m_columns[i];
            ss << " " << column.m_name << " " << getColumnDataTypeName(column.m_dataType);
            for (const auto& constraint : column.m_constraints)
                ss << " " << constraintToString(constraint);
            ss << ',';
        }
        const auto& column = tableInfo.m_columns.back();
        ss << " " << column.m_name << " " << getColumnDataTypeName(column.m_dataType);
        for (const auto& constraint : column.m_constraints)
            ss << " " << constraintToString(constraint);
        ss << " )";
    }
    return ss.str();
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
            std::ostringstream ss;
            ss << '\'';
            for (const auto ch : s) {
                if (ch == '\'')
                    ss << "''";
                else
                    ss << ch;
            }
            ss << '\'';
            result = ss.str();
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_BINARY: {
            siodb::BinaryValue bv;
            if (SIODB_UNLIKELY(!codedInput.Read(&bv))) return false;
            std::ostringstream ss;
            ss << "X'" << std::setfill('0') << std::hex;
            for (std::size_t i = 0; i < bv.size(); ++i) {
                const std::uint16_t v = bv[i];
                ss << std::setw(2) << v;
            }
            ss << '\'';
            result = ss.str();
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_TIMESTAMP: {
            siodb::RawDateTime dateTime;
            if (!siodb::protobuf::readRawDateTime(codedInput, dateTime)) return false;
            std::ostringstream ss;
            ss << '\'' << dateTime.formatDefault() << '\'';
            result = ss.str();
            return true;
        }

        default: return false;
    }
}

std::string formAlterTableSetNextTrid(
        const std::string& databaseName, const std::string& tableName, std::uint64_t nextTrid)
{
    std::ostringstream ss;
    ss << "ALTER TABLE " << databaseName << '.' << tableName << " SET NEXT_TRID = " << nextTrid;
    return ss.str();
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

std::vector<DatabaseInfo> dumpDatabasesList(
        io::InputOutputStream& connection, std::ostream& os, protobuf::StreamInputStream& input)
{
    auto query = formSelectCoreBody(kSystemDatabaseName, kSysDatabasesTableName,
            {kSysDatabases_Name_ColumnName, kSysDatabases_CipherId_ColumnName,
                    kSysDatabases_CipherKey_ColumnName});
    const auto response = sendCommand(std::move(query), connection, input);

    std::vector<DatabaseInfo> databases;
    databases.reserve(16);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength))
            std::runtime_error("dumpDatabasesList: Read row length failed");

        if (rowLength == 0) break;

        DatabaseInfo database;
        if (SIODB_LIKELY(codedInput.Read(&database.m_name) && codedInput.Read(&database.m_cipherId)
                         && codedInput.Read(&database.m_cipherKey))) {
            if (database.m_name != kSystemDatabaseName) {
                os << formCreateDatabaseQuery(database) << ';' << '\n';
                databases.push_back(std::move(database));
            }
        } else
            std::runtime_error("dumpDatabasesList: Read database record failed");
    }

    return databases;
}

void dumpSpecificDatabase(io::InputOutputStream& connection, std::ostream& os,
        protobuf::StreamInputStream& input, const std::string& databaseName)
{
    std::ostringstream ss;
    ss << formSelectCoreBody(kSystemDatabaseName, kSysDatabasesTableName,
            {kSysDatabases_Name_ColumnName, kSysDatabases_CipherId_ColumnName,
                    kSysDatabases_CipherKey_ColumnName})
       << " WHERE " << kSysDatabases_Name_ColumnName << " = '" << databaseName << '\'';

    const auto response = sendCommand(ss.str(), connection, input);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::uint64_t rowLength = 0;
    if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
        std::runtime_error("dumpSpecificDatabase: Read row length failed");

    if (SIODB_UNLIKELY(rowLength == 0))
        std::runtime_error("dumpSpecificDatabase: Database doesn't exist");

    // nulls are dissalowed, read values
    DatabaseInfo database;
    if (SIODB_LIKELY(codedInput.Read(&database.m_name) && codedInput.Read(&database.m_cipherId)
                     && codedInput.Read(&database.m_cipherKey))) {
        if (database.m_name != kSystemDatabaseName) {
            os << formCreateDatabaseQuery(database) << ';' << '\n';
        }
    } else
        std::runtime_error("dumpSpecificDatabase: Read database record failed");

    if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
        std::runtime_error("dumpSpecificDatabase: Read row length failed");
    if (SIODB_UNLIKELY(rowLength != 0))
        std::runtime_error("dumpSpecificDatabase: Invalid row length");
}

std::vector<ColumnConstraint> getColumnConstraintsList(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName,
        std::int64_t columnSetId)
{
    // SELECT CONSTRAINT_ID FROM <database-name>SYS_COLUMN_DEF_CONSTRAINTS
    // WHERE COLUMN_DEF_ID = value of the earlier selected SYS_COLUMN_SET_COLUMNS.COLUMN_DEF_ID
    std::ostringstream ss;
    ss << formSelectCoreBody(databaseName, kSysColumnDefConstraintsTableName,
            {kSysColumnDefinitionConstraintList_ConstraintId_ColumnName})
       << " WHERE " << kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnName << " = "
       << columnSetId;

    auto response = sendCommand(ss.str(), connection, input);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::vector<std::uint64_t> constaintIds;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error(
                    "getColumnConstraintsList: Read SYS_COLUMN_SET_COLUMNS row length failed");

        if (rowLength == 0) break;

        std::uint64_t constaintId = 0;
        if (SIODB_LIKELY(codedInput.Read(&constaintId)))
            constaintIds.push_back(constaintId);
        else
            throw std::runtime_error("Read constraint ID failed");
    }

    if (constaintIds.empty()) return std::vector<ColumnConstraint>();

    //  SELECT NAME, CONSTRAINT_DEF_ID FROM .SYS_CONSTRAINTS
    //  WHERE TRID IN (list of selected above CONSTRAINT_ID)
    ss.str("");
    ss << formSelectCoreBody(databaseName, kSysConstraintsTableName,
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
            std::runtime_error("getColumnConstraintsList: Read SYS_CONSTRAINTS row length failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        ColumnConstraint constraint;
        if (SIODB_LIKELY(codedInput.Read(&constraint.m_name)
                         && codedInput.Read(&constraint.m_constraintDefinitionId)))
            constraints.push_back(std::move(constraint));
        else
            throw std::runtime_error("getColumnConstraintsList: Read constraint record failed");
    }

    for (auto& constraint : constraints) {
        // SELECT TYPE, EXPR FROM <database-name>.SYS_CONSTRAINT_DEFS WHERE TRID= selected above CONSTRAINT_DEF_ID
        ss.str("");
        ss << formSelectCoreBody(databaseName, kSysConstraintDefsTableName,
                {kSysConstraintDefs_Type_ColumnName, kSysConstraintDefs_Expr_ColumnName})
           << " WHERE " << kMasterColumnName << " = " << constraint.m_constraintDefinitionId;
        response = sendCommand(ss.str(), connection, input);

        const int columnCount = response.column_description_size();

        while (true) {
            std::uint64_t rowLength = 0;
            if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
                std::runtime_error(
                        "getColumnConstraintsList: Read SYS_CONSTRAINT_DEFS row length failed");

            if (SIODB_UNLIKELY(rowLength == 0)) break;

            stdext::bitmask nullBitmask;

            // EXPR is allowed to be null
            nullBitmask.resize(columnCount, false);
            if (SIODB_LIKELY(codedInput.ReadRaw(nullBitmask.data(), nullBitmask.size()))) {
                std::uint32_t n = 0;
                if (SIODB_LIKELY(codedInput.Read(&n)))
                    constraint.m_type = static_cast<ConstraintType>(n);
                else {
                    throw std::runtime_error(
                            "getColumnConstraintsList: Read constraint type failed");
                }
                if (!nullBitmask.get(1)) {
                    if (SIODB_UNLIKELY(!codedInput.Read(&constraint.m_expression))) {
                        throw std::runtime_error(
                                "getColumnConstraintsList: Read constraint expression failed");
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

std::vector<ColumnInfo> getColumnList(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName,
        std::int64_t currentColumnSetId)
{
    std::ostringstream ss;
    ss.str("");
    // SELECT TRID, COLUMN_ID <database-name>.SYS_COLUMN_SET_COLUMNS
    // WHERE COLUMN_SET_ID = value of <CURRENT_COLUMN_SET> in the SYS_TABLES
    ss << formSelectCoreBody(databaseName, kSysColumnSetColumnsTableName,
            {kMasterColumnName, kSysColumnSetColumns_ColumnDefinitionId_ColumnName})
       << " WHERE " << kSysColumnSetColumns_ColumnSetId_ColumnName << " = " << currentColumnSetId;

    auto response = sendCommand(ss.str(), connection, input);
    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::vector<ColumnSetInfo> columnSetInfos;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error("getColumnList: Read SYS_COLUMN_SET_COLUMNS row failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        // nulls are dissalowed, read values
        ColumnSetInfo columnSetInfo;
        if (SIODB_LIKELY(codedInput.Read(&columnSetInfo.m_trid)
                         && codedInput.Read(&columnSetInfo.m_columnDefinitionId)))
            columnSetInfos.push_back(std::move(columnSetInfo));
        else
            throw std::runtime_error("getColumnList: Read column set record failed");
    }

    if (columnSetInfos.empty()) return std::vector<ColumnInfo>();

    // SELECT TRID, COLUMN_ID from <database-name>.SYS_COLUMN_DEFS WHERE TRID IN (list of selected above m_columnDefinitionId);
    ss.str("");
    ss << formSelectCoreBody(databaseName, kSysColumnDefsTableName,
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
            std::runtime_error("getColumnList: Read SYS_COLUMN_DEFS row length failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        // nulls are dissalowed, read values
        ColumnDefinitionInfo columnDefInfo;
        if (SIODB_LIKELY(codedInput.Read(&columnDefInfo.m_trid)
                         && codedInput.Read(&columnDefInfo.m_columnId))) {
            columnIdToColumnDefIdMap[columnDefInfo.m_columnId] = columnDefInfo.m_trid;
            columnDefInfos.push_back(std::move(columnDefInfo));
        } else
            throw std::runtime_error("getColumnList: Read column definition record failed");
    }

    // SELECT TRID, DATA_TYPE, NAME FROM <database-name>.SYS_COLUMNS WHERE TRID IN (... list of retrieved above COLUMN_IDs ... )
    ss.str("");
    ss << formSelectCoreBody(databaseName, kSysColumnsTableName,
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
            std::runtime_error("getColumnList: Read SYS_COLUMNS row length failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        // nulls are dissalowed, read values
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
            throw std::runtime_error("getColumnList: Read column record failed");
    }

    std::sort(columns.begin(), columns.end(), [](const auto& left, const auto& right) noexcept {
        return left.m_columnDefinitionId < right.m_columnDefinitionId;
    });

    for (auto& column : columns) {
        column.m_constraints = getColumnConstraintsList(
                connection, input, databaseName, column.m_columnDefinitionId);
    }

    return columns;
}

std::vector<TableInfo> dumpTablesList(io::InputOutputStream& connection, std::ostream& os,
        protobuf::StreamInputStream& input, const std::string& databaseName)
{
    std::ostringstream ss;
    // SELECT TRID,NAME,CURRENT_COLUMN_SET_ID FROM <DATABASE>.SYS_TABLES WHERE NAME NOT LIKE 'SYS_%'"
    ss << formSelectCoreBody(databaseName, kSysTablesTableName,
            {kMasterColumnName, kSysTables_Name_ColumnName,
                    kSysTables_CurrentColumnSetId_ColumnName})
       << " WHERE " << kSysTables_Name_ColumnName << " NOT LIKE 'SYS_%' AND "
       << kSysTables_Type_ColumnName << " = 1";
    auto response = sendCommand(ss.str(), connection, input);

    std::vector<TableInfo> tableInfos;
    tableInfos.reserve(32);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            std::runtime_error("dumpTablesList: Read row length failed");

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        TableInfo tableInfo;
        if (SIODB_LIKELY(codedInput.Read(&tableInfo.m_trid) && codedInput.Read(&tableInfo.m_name)
                         && codedInput.Read(&tableInfo.m_currentColumnSetId))) {
            tableInfos.push_back(std::move(tableInfo));
        } else
            throw std::runtime_error("dumpTablesList: Read table record failed");
    }

    for (auto& tableInfo : tableInfos) {
        tableInfo.m_columns =
                getColumnList(connection, input, databaseName, tableInfo.m_currentColumnSetId);
        os << formCreateTableQuery(databaseName, tableInfo) << ';' << '\n';
    }

    return tableInfos;
}

void dumpTableData(io::InputOutputStream& connection, std::ostream& os,
        protobuf::StreamInputStream& input, const std::string& databaseName, const TableInfo& table)
{
    std::ostringstream ss;
    ss << formSelectAllQuery(databaseName, table.m_name);
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
        if (SIODB_UNLIKELY(codedInput.Read(&trid)))
            throw std::runtime_error("dumpTableData: Read TRID failed");

        if (expectedTrid != trid) {
            os << formAlterTableSetNextTrid(databaseName, table.m_name, trid) << ';' << '\n';
            expectedTrid = trid + 1;
        } else {
            ++expectedTrid;
        }

        os << "INSERT INTO " << databaseName << '.' << table.m_name;
        if (!table.m_columns.empty()) os << " VALUES (";

        for (auto i = 0u; i < table.m_columns.size() - 1; ++i) {
            if (nullAllowed && nullBitmask.get(i + 1))  // Skip TRID
                os << "NULL, ";
            else {
                std::string value;
                if (SIODB_UNLIKELY(!readValue(codedInput, table.m_columns.at(i).m_dataType, value)))
                    throw std::runtime_error("Can't read value");
                os << value << ", ";
            }
        }

        if (nullAllowed && nullBitmask.get(columnCount - 1))
            os << "NULL";
        else {
            std::string value;
            if (SIODB_UNLIKELY(!readValue(
                        codedInput, table.m_columns.at(columnCount - 1).m_dataType, value)))
                throw std::runtime_error("Can't read value");
            os << value;
        }

        if (!table.m_columns.empty()) os << ')';

        os << ';' << '\n';
    }
}

void dumpDatabaseData(io::InputOutputStream& connection, std::ostream& os,
        const std::string& databaseName, protobuf::StreamInputStream& input)
{
    auto tables = dumpTablesList(connection, os, input, databaseName);
    for (const auto& table : tables)
        dumpTableData(connection, os, input, databaseName, table);
}

}  // namespace

void dumpAllDatabases(io::InputOutputStream& connection, std::ostream& os)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;
    protobuf::StreamInputStream input(connection, errorCodeChecker);

    auto databases = dumpDatabasesList(connection, os, input);
    for (const auto& database : databases)
        dumpDatabaseData(connection, os, database.m_name, input);

    os << std::flush;
}

void dumpDatabase(
        io::InputOutputStream& connection, std::ostream& os, const std::string& databaseName)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;
    protobuf::StreamInputStream input(connection, errorCodeChecker);

    dumpSpecificDatabase(connection, os, input, databaseName);
    dumpDatabaseData(connection, os, databaseName, input);

    os << std::flush;
}

}  // namespace siodb::siocli
