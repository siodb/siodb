// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/io/InputOutputStream.h>
#include <siodb/common/protobuf/ExtendedCodedInputStream.h>
#include <siodb/common/protobuf/StreamInputStream.h>
#include <siodb/common/utils/BinaryValue.h>
#include <siodb/iomgr/shared/dbengine/ColumnDataType.h>
#include <siodb/iomgr/shared/dbengine/ConstraintType.h>
#include <siodb/iomgr/shared/dbengine/SystemObjectNames.h>

// Protobuf message headers
#include <siodb/common/proto/ClientProtocol.pb.h>

namespace siodb::siocli::detail {

using namespace siodb::iomgr::dbengine;

/** Database information */
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
};

/** Column constraint information */
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

/** Table column information */
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

/** Column set information */
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

/** Column definition information */
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

/** Table information */
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

/**
 * Dumps definition and data of a given database into a given output stream.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param dbInfo Database information.
 * @param os Output stream.
 * @param printDebugMessages Turns on or off printing debug messages.
 */
void dumpDatabase(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const DatabaseInfo& dbInfo, std::ostream& os, bool printDebugMessages);

/**
 * Dumps data of a given list of table into a given output stream.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param databaseName Database name.
 * @param tableInfos Table list.
 * @param os Output stream.
 * @param printDebugMessages Turns on or off printing debug messages.
 */
void dumpTables(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::vector<TableInfo>& tableInfos, std::ostream& os,
        bool printDebugMessages);

/**
 * Dumps data of a given table into a given output stream.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @param os Output stream.
 * @param printDebugMessages Turns on or off printing debug messages.
 */
void dumpTable(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::string& tableName, std::ostream& os,
        bool printDebugMessages);

/**
 * Dumps data of a given table into a given output stream.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @param os Output stream.
 * @param printDebugMessages Turns on or off printing debug messages.
 */
void dumpTable(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const TableInfo& table, std::ostream& os,
        bool printDebugMessages);

/**
 * Writes CREATE DATABASE statement fir the given database into a give output stream.
 * @param dbInfo Database information.
 * @param os Output stream.
 */
void dumpDatabaseDefinition(const DatabaseInfo& dbInfo, std::ostream& os);

/**
 * Writes CREATE TABLE statment for a given table into a given output stream.
 * @param databaseName Database name.
 * @param table Table information.
 * @param os Output stream.
 */
void dumpTableDefinition(const std::string& databaseName, const TableInfo& table, std::ostream& os);

/**
 * Reads list of databases.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param printDebugMessages Turns on or off printing debug messages to stderr.
 * @return List of databases.
 */
std::vector<DatabaseInfo> readDatabaseInfos(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, bool printDebugMessages);

/**
 * Reads information about a single database.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param dbInfo Database information.
 * @param printDebugMessages Turns on or off printing debug messages.
 * @return Database information.
 */
DatabaseInfo readDatabaseInfo(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, bool printDebugMessages);

/**
 * Reads information about tables in a given database.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param databaseName Database name.
 * @param printDebugMessages Turns on or off printing debug messages.
 * @return List of tables.
 */
std::vector<TableInfo> readTableInfos(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName,
        bool printDebugMessages);

/**
 * Reads information about a single table from a given database.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @param printDebugMessages Turns on or off printing debug messages.
 * @return List of tables.
 */
TableInfo readTableInfo(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::string& tableName, bool printDebugMessages);

/**
 * Reads information about columns of a table and puts it into table info.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @param columnSetId Column set identifier.
 * @param printDebugMessages Turns on or off printing debug messages.
 */
void readColumns(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, TableInfo& tableInfo, bool printDebugMessages);

/**
 * Reads information about constraint of a column of a table.
 * @param connection Connection with database server.
 * @param input Input stream.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @param columnInfo Column information.
 * @param printDebugMessages Turns on or off printing debug messages.
 */
void readColumnConstraints(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::string& tableName, ColumnInfo& columnInfo,
        bool printDebugMessages);

/**
 * Sends command to a server.
 * @param command  Command text.
 * @param connection Connection with server.
 * @param input Input stream.
 * @param printDebugMessages Turns on or off printing debug messages.
 */
client_protocol::ServerResponse sendCommand(std::string&& command,
        io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        bool printDebugMessages);

/**
 * Checks server reponse for errors.
 * @throw std::runtime_error if errors found in the reponse.
 */
void checkResponse(const client_protocol::ServerResponse& response);

/**
 * Reads data value from a stream.
 * @param codedInput Input stream.
 * @param dataType Data type.
 * @param result Resulting value converted to string.
 * @return true if read was successful, false otherwise.
 */
bool readValue(protobuf::ExtendedCodedInputStream& codedInput, ColumnDataType dataType,
        std::string& result);

/**
 * Builds core part of the select statement.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @param columnNames Column names.
 * @return SELECT statement text.
 */
std::string buildSelectStatementCore(const std::string& databaseName, const std::string& tableName,
        const std::vector<std::string>& columnNames);

/**
 * Builds core part of the select statement.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @param columnNames Column names.
 * @return SELECT statement text.
 */
std::string buildSelectStatementCore(const char* databaseName, const char* tableName,
        const std::vector<std::string>& columnNames);

/**
 * Builds "SELECT *" statement for a give table.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @return "SELECT *" statement text.
 */
std::string buildSelectAllStatement(const std::string& databaseName, const std::string& tableName);

/**
 * Builds CREATE DATABASE statement for a given database.
 * @param dbInfo Database information.
 * @return CREATE DATABASE statement text.
 */
std::string buildCreateDatabaseStatement(const DatabaseInfo& dbInfo);

/**
 * Builds CREATE TABLE statement for a given database.
 * @param dbInfo Database information.
 * @return CREATE TABLE statement text.
 */
std::string buildCreateTableStatement(const std::string& databaseName, const TableInfo& tableInfo);

/**
 * Builds ALTER TABLE SET NEXT_TRID statement for a given table.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @param nextTrid Next table row ID value.
 * @return ALTER TABLE SET NEXT_TRID statement text.
 */
std::string buildAlterTableSetNextTridStatement(
        const std::string& databaseName, const std::string& tableName, std::uint64_t nextTrid);

}  // namespace siodb::siocli::detail
