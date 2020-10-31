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

std::vector<DatabaseInfo> readDatabases(
        io::InputOutputStream& connection, protobuf::StreamInputStream& input);

void dumpCreateDatabase(const DatabaseInfo& dbInfo, std::ostream& os);

void dumpCreateDatabase(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, std::ostream& os);

void dumpData(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::string& tableName, std::ostream& os);

void dumpTables(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::vector<TableInfo>& tableInfos,
        std::ostream& os);

void dumpTableDefinition(const std::string& databaseName, const TableInfo& table, std::ostream& os);

void dumpTableData(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const TableInfo& table, std::ostream& os);

std::vector<TableInfo> readTables(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName);

std::vector<ColumnInfo> readColumns(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName,
        std::int64_t currentColumnSetId);

std::vector<ColumnConstraint> readColumnConstraints(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName,
        std::int64_t columnSetId);

client_protocol::ServerResponse sendCommand(std::string&& command,
        io::InputOutputStream& connection, protobuf::StreamInputStream& input);

void checkResponse(const client_protocol::ServerResponse& response);

bool readValue(protobuf::ExtendedCodedInputStream& codedInput, ColumnDataType columnDataType,
        std::string& result);

std::string buildSelectStatementCore(const std::string& databaseName, const std::string& tableName,
        const std::vector<std::string>& ColumnNames);

std::string buildSelectStatementCore(const char* databaseName, const char* tableName,
        const std::vector<std::string>& columnNames);

std::string buildSelectAllStatement(const std::string& databaseName, const std::string& tableName);

std::string buildCreateDatabaseStatement(const DatabaseInfo& dbInfo);

std::string buildCreateTableStatement(const std::string& databaseName, const TableInfo& tableInfo);

std::string buildAlterTableSetNextTridStatement(
        const std::string& databaseName, const std::string& tableName, std::uint64_t nextTrid);

}  // namespace siodb::siocli::detail
