// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "SqlDump.h"

// Common project headers
#include "siodb/iomgr/shared/dbengine/SystemObjectNames.h"
#include <siodb/common/data/RawDateTime.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>
#include <siodb/common/stl_ext/bitmask.h>
#include <siodb/common/utils/BinaryValue.h>
#include <siodb/iomgr/shared/dbengine/ColumnDataType.h>
#include <siodb/iomgr/shared/dbengine/ConstraintType.h>

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

struct DatabaseInfo {
    std::string m_name;
    std::string m_cipherId;
    BinaryValue m_cipherKey;
};

struct ColumnConstaint {
    ConstraintType m_type;
    std::string m_name;
    std::uint64_t m_constraintDefinitionId;
    BinaryValue m_expression;
};

struct ColumnInfo {
    std::uint64_t m_trid;
    std::string m_name;
    ColumnDataType m_dataType;
    std::uint64_t m_columnDefinitionId;

    std::vector<ColumnConstaint> m_constraints;
};

struct ColumnSetInfo {
    std::uint64_t m_trid;
    std::uint64_t m_columnDefinitionId;
};

struct ColumnDefInfo {
    std::uint64_t m_trid;
    std::uint64_t m_columnId;
};

struct TableInfo {
    std::uint64_t m_trid;
    std::uint64_t m_currentColumnSetId;

    std::string m_name;
    std::vector<ColumnInfo> m_columns;
};

std::string constraintToString(const ColumnConstaint& constraint)
{
    // Only NOT NULL constraint is supported for now
    if (constraint.m_type != ConstraintType::kNotNull) return std::string();

    return getConstaintTypeName(constraint.m_type);
}

std::string formSelectCoreBody(const char* databaseName, const char* tableName,
        const std::vector<std::string>& columnNames)
{
    std::stringstream ss;
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
    std::stringstream ss;
    ss << "SELECT * FROM " << databaseName << '.' << tableName;
    return ss.str();
}

std::string formCreateDatabaseQuery(const DatabaseInfo& dbInfo)
{
    std::stringstream ss;
    ss << "CREATE DATABASE " << dbInfo.m_name;

    if (!dbInfo.m_cipherId.empty())
        ss << " WITH " << kSysDatabases_CipherId_ColumnName << " = '" << dbInfo.m_cipherId << '\'';

    // We can't create a seed value from database key
    // ss << ", " << kSysDatabases_CipherKey_ColumnName << " = " << dbInfo.m_cipherKey;

    return ss.str();
}

std::string formCreateTableQuery(const std::string& databaseName, const TableInfo& tableInfo)
{
    std::stringstream ss;

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

BinaryValue readBinaryValue(google::protobuf::io::CodedInputStream& input)
{
    std::uint32_t blobLength = 0;
    // Read length
    if (!input.ReadVarint32(&blobLength))
        throw std::runtime_error("Can't read binary value length");

    BinaryValue value;
    value.resize(blobLength);

    if (!input.ReadRaw(value.data(), blobLength))
        throw std::runtime_error("Binary data read failed");

    return value;
}

std::string readString(google::protobuf::io::CodedInputStream& input)
{
    std::uint32_t clobLength = 0;
    // Read length
    if (!input.ReadVarint32(&clobLength))
        throw std::runtime_error("Can't read string value length");

    // Read sample
    std::string str;
    str.resize(clobLength);

    if (!input.ReadRaw(str.data(), clobLength)) throw std::runtime_error("String data read failed");

    return str;
}

std::int8_t readInt8(google::protobuf::io::CodedInputStream& input)
{
    std::int8_t data = 0;
    if (!input.ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read int8 failed");

    return data;
}

std::int8_t readInt16(google::protobuf::io::CodedInputStream& input)
{
    std::int16_t data = 0;
    if (!input.ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read int16 failed");

    return data;
}

std::int32_t readInt32(google::protobuf::io::CodedInputStream& input)
{
    std::int32_t data = 0;
    if (!input.ReadVarint32(reinterpret_cast<std::uint32_t*>(&data)))
        throw std::runtime_error("Read int32 failed");

    return data;
}

std::int64_t readInt64(google::protobuf::io::CodedInputStream& input)
{
    std::int64_t data = 0;
    if (!input.ReadVarint64(reinterpret_cast<std::uint64_t*>(&data)))
        throw std::runtime_error("Read int64 failed");

    return data;
}

std::uint8_t readUInt8(google::protobuf::io::CodedInputStream& input)
{
    std::int8_t data = 0;
    if (!input.ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read int8 failed");

    return data;
}

std::uint8_t readUInt16(google::protobuf::io::CodedInputStream& input)
{
    std::uint16_t data = 0;
    if (!input.ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read int16 failed");

    return data;
}

std::uint32_t readUInt32(google::protobuf::io::CodedInputStream& input)
{
    std::uint32_t data = 0;
    if (!input.ReadVarint32(&data)) throw std::runtime_error("Read int32 failed");

    return data;
}

std::uint64_t readUInt64(google::protobuf::io::CodedInputStream& input)
{
    std::uint64_t data = 0;
    if (!input.ReadVarint64(&data)) throw std::runtime_error("Read int64 failed");

    return data;
}

float readFloat(google::protobuf::io::CodedInputStream& input)
{
    float data = 0;
    if (!input.ReadLittleEndian32(reinterpret_cast<std::uint32_t*>(&data)))
        throw std::runtime_error("Read float failed");

    return data;
}

double readDouble(google::protobuf::io::CodedInputStream& input)
{
    double data = 0;
    if (!input.ReadLittleEndian64(reinterpret_cast<std::uint64_t*>(&data)))
        throw std::runtime_error("Read double failed");

    return data;
}

bool readBool(google::protobuf::io::CodedInputStream& input)
{
    std::uint8_t data = 0;
    if (!input.ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read bool failed");

    return data > 0;
}

std::string readValue(google::protobuf::io::CodedInputStream& input, ColumnDataType columnDataType)
{
    switch (columnDataType) {
        case ColumnDataType::COLUMN_DATA_TYPE_BOOL: {
            return readBool(input) ? "true" : "false";
        }
        case ColumnDataType::COLUMN_DATA_TYPE_INT8: {
            return std::to_string(readInt8(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_UINT8: {
            return std::to_string(readUInt8(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_INT16: {
            return std::to_string(readInt16(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_UINT16: {
            return std::to_string(readUInt16(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_INT32: {
            return std::to_string(readInt32(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_UINT32: {
            return std::to_string(readUInt32(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_INT64: {
            return std::to_string(readInt64(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_UINT64: {
            return std::to_string(readUInt64(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_FLOAT: {
            return std::to_string(readFloat(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_DOUBLE: {
            return std::to_string(readDouble(input));
        }
        case ColumnDataType::COLUMN_DATA_TYPE_TEXT: {
            std::stringstream ss;

            const std::string str = readString(input);
            ss << '\'';
            for (const auto symbol : str) {
                if (symbol == '\'')
                    ss << "''";
                else
                    ss << symbol;
            }

            ss << '\'';

            return ss.str();
        }
        case ColumnDataType::COLUMN_DATA_TYPE_BINARY: {
            const auto binaryValue = readBinaryValue(input);

            std::stringstream ss;
            ss << "X'" << std::setfill('0') << std::hex;
            for (std::size_t i = 0; i < binaryValue.size(); ++i) {
                const std::uint16_t v = binaryValue[i];
                ss << std::setw(2) << v;
            }
            ss << '\'';

            return ss.str();
        }
        case ColumnDataType::COLUMN_DATA_TYPE_TIMESTAMP: {
            std::stringstream ss;
            siodb::RawDateTime dateTime;
            if (!siodb::protobuf::readRawDateTime(input, dateTime))
                throw std::runtime_error("Read timestamp value failed");

            ss << '\'' << dateTime.formatDefault() << '\'';
            return ss.str();
        }
        default: throw std::runtime_error("Unknown data type");
    }
}

std::string formAlterTableSetNextTrid(
        const std::string& databaseName, const std::string& tableName, std::uint64_t nextTrid)
{
    std::stringstream ss;
    ss << "ALTER TABLE " << databaseName << '.' << tableName << " SET NEXT_TRID = " << nextTrid;
    return ss.str();
}

void checkResponse(const client_protocol::ServerResponse& response)
{
    const auto messageCount = response.message_size();
    if (messageCount > 0) {
        bool sqlErrorOccurred = false;
        for (int i = 0; i < messageCount; ++i) {
            const auto& message = response.message(i);
            sqlErrorOccurred |= message.status_code() != 0;
        }

        if (sqlErrorOccurred) throw std::runtime_error("SQL error");
    }
}

client_protocol::ServerResponse sendCommand(
        std::string&& command, io::IoBase& connectionIo, protobuf::CustomProtobufInputStream& input)
{
    static std::int64_t requestId = 0;
    client_protocol::Command clientCommand;
    clientCommand.set_request_id(requestId);
    clientCommand.set_text(std::move(command));
    protobuf::writeMessage(protobuf::ProtocolMessageType::kCommand, clientCommand, connectionIo);

    client_protocol::ServerResponse response;
    protobuf::readMessage(protobuf::ProtocolMessageType::kServerResponse, response, input);

    checkResponse(response);
    ++requestId;

    return response;
}

std::vector<DatabaseInfo> dumpDatabasesList(
        io::IoBase& connectionIo, std::ostream& os, protobuf::CustomProtobufInputStream& input)
{
    auto query = formSelectCoreBody(kSystemDatabaseName, kSysDatabasesTableName,
            {kSysDatabases_Name_ColumnName, kSysDatabases_CipherId_ColumnName,
                    kSysDatabases_CipherKey_ColumnName});
    const auto response = sendCommand(std::move(query), connectionIo, input);

    std::vector<DatabaseInfo> databases;
    databases.reserve(16);

    const int columnCount = response.column_description_size();
    if (columnCount != 3) std::runtime_error("Unexpected SYS DATABASES column count");

    // Create CodedInputStream only if row data is available to read
    // otherwise codedInput constructor will be stucked on waiting on buffering data
    google::protobuf::io::CodedInputStream codedInput(&input);

    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength)) std::runtime_error("Can't read row length");

        if (rowLength == 0) break;

        // nulls are dissalowed, read values
        DatabaseInfo database;
        database.m_name = readString(codedInput);
        database.m_cipherId = readString(codedInput);
        database.m_cipherKey = readBinaryValue(codedInput);

        if (database.m_name != kSystemDatabaseName) {
            os << formCreateDatabaseQuery(database) << ';' << '\n';
            databases.push_back(std::move(database));
        }
    }

    return databases;
}

std::vector<ColumnConstaint> receiveColumnConstraintsList(io::IoBase& connectionIo,
        protobuf::CustomProtobufInputStream& input, const std::string& databaseName,
        std::int64_t columnSetId)
{
    // SELECT CONSTRAINT_ID FROM <database-name>SYS_COLUMN_DEF_CONSTRAINTS
    // WHERE COLUMN_DEF_ID = value of the earlier selected SYS_COLUMN_SET_COLUMNS.COLUMN_DEF_ID
    std::stringstream ss;
    ss << formSelectCoreBody(databaseName, kSysColumnDefConstraintsTableName,
            {kSysColumnDefinitionConstraintList_ConstraintId_ColumnName})
       << " WHERE " << kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnName << " = "
       << columnSetId;

    std::string query = ss.str();

    auto response = sendCommand(std::move(query), connectionIo, input);

    google::protobuf::io::CodedInputStream codedInput(&input);

    std::vector<std::uint64_t> constaintIds;
    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength)) std::runtime_error("Can't read row length");

        if (rowLength == 0) break;

        const auto constaintId = readUInt64(codedInput);
        constaintIds.push_back(constaintId);
    }

    if (constaintIds.empty()) return std::vector<ColumnConstaint>();

    //  SELECT NAME, CONSTRAINT_DEF_ID FROM .SYS_CONSTRAINTS
    //  WHERE TRID IN (list of selected above CONSTRAINT_ID)
    ss.str("");
    ss << formSelectCoreBody(databaseName, kSysConstraintsTableName,
            {kSysConstraints_Name_ColumnName, kSysConstraints_DefinitionId_ColumnName})
       << " WHERE " << kMasterColumnName << " IN (";

    for (auto i = 0u; i < constaintIds.size() - 1; ++i)
        ss << constaintIds[i] << ',';

    ss << constaintIds.back() << ')';

    query = ss.str();
    response = sendCommand(std::move(query), connectionIo, input);
    std::vector<ColumnConstaint> constraints;
    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength)) std::runtime_error("Can't read row length");

        if (rowLength == 0) break;

        ColumnConstaint constraint;
        constraint.m_name = readString(codedInput);
        constraint.m_constraintDefinitionId = readUInt64(codedInput);
        constraints.push_back(std::move(constraint));
    }

    for (auto& constraint : constraints) {
        // SELECT TYPE, EXPR FROM <database-name>.SYS_CONSTRAINT_DEFS WHERE TRID= selected above CONSTRAINT_DEF_ID
        ss.str("");
        ss << formSelectCoreBody(databaseName, kSysConstraintDefsTableName,
                {kSysConstraintDefs_Type_ColumnName, kSysConstraintDefs_Expr_ColumnName})
           << " WHERE " << kMasterColumnName << " = " << constraint.m_constraintDefinitionId;

        query = ss.str();
        response = sendCommand(std::move(query), connectionIo, input);

        const int columnCount = response.column_description_size();

        while (true) {
            std::uint64_t rowLength = 0;
            if (!codedInput.ReadVarint64(&rowLength)) std::runtime_error("Can't read row length");

            if (rowLength == 0) break;

            stdext::bitmask nullBitmask;

            // EXPR is allowed to be null
            nullBitmask.resize(columnCount, false);
            if (!codedInput.ReadRaw(nullBitmask.data(), nullBitmask.size())) {
                std::ostringstream err;
                err << "Can't read null bitmask from server: " << std::strerror(input.GetErrno());
                throw std::system_error(input.GetErrno(), std::generic_category(), err.str());
            }

            constraint.m_type = static_cast<ConstraintType>(readUInt8(codedInput));
            if (!nullBitmask.get(1)) constraint.m_expression = readBinaryValue(codedInput);
        }
    }

    return constraints;
}

std::vector<ColumnInfo> receiveColumnList(io::IoBase& connectionIo,
        protobuf::CustomProtobufInputStream& input, const std::string& databaseName,
        std::int64_t currentColumnSetId)
{
    std::stringstream ss;
    ss.str("");
    // SELECT TRID, COLUMN_ID <database-name>.SYS_COLUMN_SET_COLUMNS
    // WHERE COLUMN_SET_ID = value of <CURRENT_COLUMN_SET> in the SYS_TABLES
    ss << formSelectCoreBody(databaseName, kSysColumnSetColumnsTableName,
            {kMasterColumnName, kSysColumnSetColumns_ColumnDefinitionId_ColumnName})
       << " WHERE " << kSysColumnSetColumns_ColumnSetId_ColumnName << " = " << currentColumnSetId;

    auto query = ss.str();

    auto response = sendCommand(std::move(query), connectionIo, input);
    google::protobuf::io::CodedInputStream codedInput(&input);

    std::vector<ColumnSetInfo> columnSetInfos;
    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength)) std::runtime_error("Can't read row length");

        if (rowLength == 0) break;

        // nulls are dissalowed, read values
        ColumnSetInfo columnSetInfo;
        columnSetInfo.m_trid = readUInt64(codedInput);
        columnSetInfo.m_columnDefinitionId = readUInt64(codedInput);

        columnSetInfos.push_back(std::move(columnSetInfo));
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
    query = ss.str();

    response = sendCommand(std::move(query), connectionIo, input);

    std::unordered_map<std::uint64_t, std::uint64_t> columnIdToColumnDefIdMap;
    std::vector<ColumnDefInfo> columnDefInfos;
    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength)) std::runtime_error("Can't read row length");

        if (rowLength == 0) break;

        // nulls are dissalowed, read values
        ColumnDefInfo columnDefInfo;
        columnDefInfo.m_trid = readUInt64(codedInput);
        columnDefInfo.m_columnId = readUInt64(codedInput);

        columnIdToColumnDefIdMap[columnDefInfo.m_columnId] = columnDefInfo.m_trid;
        columnDefInfos.push_back(std::move(columnDefInfo));
    }

    // SELECT TRID, DATA_TYPE, NAME FROM <database-name>.SYS_COLUMNS WHERE TRID IN (... list of retrieved above COLUMN_IDs ... )
    ss.str("");
    ss << formSelectCoreBody(databaseName, kSysColumnsTableName,
            {kMasterColumnName, kSysColumns_DataType_ColumnName, kSysColumns_Name_ColumnName})
       << " WHERE " << kMasterColumnName << " IN (";

    for (auto i = 0u; i < columnDefInfos.size() - 1; ++i)
        ss << columnDefInfos[i].m_columnId << ',';

    ss << columnDefInfos.back().m_columnId << ')';

    query = ss.str();
    response = sendCommand(std::move(query), connectionIo, input);

    std::vector<ColumnInfo> columns;
    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength)) std::runtime_error("Can't read row length");

        if (rowLength == 0) break;

        // nulls are dissalowed, read values
        ColumnInfo columnInfo;
        columnInfo.m_trid = readUInt64(codedInput);
        columnInfo.m_dataType = static_cast<ColumnDataType>(readInt8(codedInput));
        columnInfo.m_name = readString(codedInput);
        if (columnInfo.m_name != kMasterColumnName) {
            columnInfo.m_columnDefinitionId = columnIdToColumnDefIdMap[columnInfo.m_trid];
            columns.push_back(std::move(columnInfo));
        }
    }

    std::sort(
            columns.begin(), columns.end(), [](const auto& left, const auto& right) noexcept {
                return left.m_columnDefinitionId < right.m_columnDefinitionId;
            });

    for (auto& column : columns) {
        column.m_constraints = receiveColumnConstraintsList(
                connectionIo, input, databaseName, column.m_columnDefinitionId);
    }

    return columns;
}

std::vector<TableInfo> dumpTablesList(io::IoBase& connectionIo, std::ostream& os,
        protobuf::CustomProtobufInputStream& input, const std::string& databaseName)
{
    std::stringstream ss;

    // SELECT TRID,NAME,CURRENT_COLUMN_SET_ID FROM <DATABASE>.SYS_TABLES WHERE NAME NOT LIKE 'SYS_%'"
    ss << formSelectCoreBody(databaseName, kSysTablesTableName,
            {kMasterColumnName, kSysTables_Name_ColumnName,
                    kSysTables_CurrentColumnSetId_ColumnName})
       << " WHERE " << kSysTables_Name_ColumnName << " NOT LIKE 'SYS_%' AND "
       << kSysTables_Type_ColumnName << " = 1";

    auto query = ss.str();
    auto response = sendCommand(std::move(query), connectionIo, input);

    std::vector<TableInfo> tableInfos;
    tableInfos.reserve(32);

    const int columnCount = response.column_description_size();
    if (columnCount != 3) std::runtime_error("Unexpected SYS_TABLES column count");

    // Create CodedInputStream only if row data is available to read
    // otherwise codedInput constructor will be stucked on waiting on buffering data
    google::protobuf::io::CodedInputStream codedInput(&input);

    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength)) std::runtime_error("Can't read row length");

        if (rowLength == 0) break;
        // nulls are dissalowed, read values
        TableInfo tableInfo;
        tableInfo.m_trid = readUInt64(codedInput);
        tableInfo.m_name = readString(codedInput);
        tableInfo.m_currentColumnSetId = readUInt64(codedInput);

        tableInfos.push_back(std::move(tableInfo));
    }

    for (auto& tableInfo : tableInfos) {
        tableInfo.m_columns = receiveColumnList(
                connectionIo, input, databaseName, tableInfo.m_currentColumnSetId);

        os << formCreateTableQuery(databaseName, tableInfo) << ';' << '\n';
    }

    return tableInfos;
}

void dumpTableData(io::IoBase& connectionIo, std::ostream& os,
        protobuf::CustomProtobufInputStream& input, const std::string& databaseName,
        const TableInfo& table)
{
    std::stringstream ss;

    // SELECT TRID,TYPE,NAME,CURRENT_COLUMN_SET_ID FROM <DATABASE>.SYS_TABLES WHERE NAME NOT LIKE 'SYS_%'"
    ss << formSelectAllQuery(databaseName, table.m_name);

    auto query = ss.str();
    auto response = sendCommand(std::move(query), connectionIo, input);

    std::vector<TableInfo> tableInfos;
    tableInfos.reserve(32);

    // + TRID
    if (response.column_description_size() != static_cast<int>(table.m_columns.size() + 1))
        std::runtime_error("Unexpected user table column count");

    bool nullAllowed = false;
    for (int i = 0; i < response.column_description_size(); ++i) {
        const auto& column = response.column_description(i);
        nullAllowed |= column.is_null();
    }

    google::protobuf::io::CodedInputStream codedInput(&input);

    std::uint64_t expectedTrid = 1;
    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength)) std::runtime_error("Can't read row length");

        if (rowLength == 0) break;

        stdext::bitmask nullBitmask;

        if (nullAllowed) {
            nullBitmask.resize(response.column_description_size(), false);

            if (!codedInput.ReadRaw(nullBitmask.data(), nullBitmask.size())) {
                std::ostringstream err;
                err << "Can't read null bitmask from server: " << std::strerror(input.GetErrno());
                throw std::system_error(input.GetErrno(), std::generic_category(), err.str());
            }
        }

        const auto trid = readUInt64(codedInput);
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
            else
                os << readValue(codedInput, table.m_columns[i].m_dataType) << ", ";
        }

        if (nullBitmask.get(table.m_columns.size() - 1))
            os << "NULL";
        else
            os << readValue(codedInput, table.m_columns.back().m_dataType);

        if (!table.m_columns.empty()) os << ')';

        os << ';' << '\n';
    }
}

}  // namespace

void sqlDumpAllDatabases(io::IoBase& connectionIo, std::ostream& os)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;

    client_protocol::ServerResponse response;
    protobuf::CustomProtobufInputStream input(connectionIo, errorCodeChecker);

    auto databases = dumpDatabasesList(connectionIo, os, input);
    for (const auto& database : databases)
        sqlDumpDatabase(connectionIo, os, database.m_name);

    os << std::flush;
}

void sqlDumpDatabase(io::IoBase& connectionIo, std::ostream& os, const std::string& databaseName)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;

    client_protocol::ServerResponse response;
    protobuf::CustomProtobufInputStream input(connectionIo, errorCodeChecker);

    auto tables = dumpTablesList(connectionIo, os, input, databaseName);
    for (const auto& table : tables)
        dumpTableData(connectionIo, os, input, databaseName, table);

    os << std::flush;
}

}  // namespace siodb::siocli
