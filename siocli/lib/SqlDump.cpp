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

void dumpAllDatabases(io::InputOutputStream& connection, std::ostream& os, bool printDebugMessages)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;
    protobuf::StreamInputStream input(connection, errorCodeChecker);
    const auto databases = detail::readDatabaseInfos(connection, input, printDebugMessages);
    bool addLeadingNewline = false;
    for (const auto& dbInfo : databases) {
        if (addLeadingNewline) os << '\n';
        addLeadingNewline = true;
        detail::dumpDatabase(connection, input, dbInfo, os, printDebugMessages);
    }
    os << std::flush;
}

void dumpSingleDatabase(io::InputOutputStream& connection, const std::string& databaseName,
        std::ostream& os, bool printDebugMessages)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;
    protobuf::StreamInputStream input(connection, errorCodeChecker);
    const auto dbInfo =
            detail::readDatabaseInfo(connection, input, databaseName, printDebugMessages);
    detail::dumpDatabase(connection, input, dbInfo, os, printDebugMessages);
    os << std::flush;
}

void dumpSingleTable(io::InputOutputStream& connection, const std::string& databaseName,
        const std::string& tableName, std::ostream& os, bool printDebugMessages)
{
    const utils::DefaultErrorCodeChecker errorCodeChecker;
    protobuf::StreamInputStream input(connection, errorCodeChecker);
    detail::dumpTable(connection, input, databaseName, tableName, os, printDebugMessages);
    os << std::flush;
}

namespace detail {

void dumpDatabase(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const DatabaseInfo& dbInfo, std::ostream& os, bool printDebugMessages)
{
    if (printDebugMessages)
        std::clog << "progress: Dumping database '" << dbInfo.m_name << "'..." << std::endl;
    const auto tables = readTableInfos(connection, input, dbInfo.m_name, printDebugMessages);
    dumpDatabaseDefinition(dbInfo, os);
    dumpTables(connection, input, dbInfo.m_name, tables, os, printDebugMessages);
}

void dumpTables(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::vector<TableInfo>& tableInfos, std::ostream& os,
        bool printDebugMessages)
{
    for (auto& table : tableInfos)
        dumpTable(connection, input, databaseName, table, os, printDebugMessages);
}

void dumpTable(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::string& tableName, std::ostream& os,
        bool printDebugMessages)
{
    const auto tableInfo =
            readTableInfo(connection, input, databaseName, tableName, printDebugMessages);
    dumpTable(connection, input, databaseName, tableInfo, os, printDebugMessages);
}

void dumpTable(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const TableInfo& table, std::ostream& os,
        bool printDebugMessages)
{
    if (printDebugMessages) {
        std::clog << "progress: Dumping table '" << databaseName << '.' << table.m_name << "'..."
                  << std::endl;
        if (printDebugMessages) {
            std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
        }
    }

    dumpTableDefinition(databaseName, table, os);
    os << '\n';

    std::ostringstream oss;
    oss << buildSelectAllStatement(databaseName, table.m_name);
    auto response = sendCommand(oss.str(), connection, input, printDebugMessages);

    std::vector<TableInfo> tableInfos;
    tableInfos.reserve(32);

    bool nullsExpected = false;
    const auto columnCount = response.column_description_size();
    for (int i = 0; i < columnCount; ++i) {
        const auto& column = response.column_description(i);
        nullsExpected |= column.is_null();
    }

    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::uint64_t rowCount = 0;
    std::uint64_t expectedTrid = 1;
    while (true) {
        if (printDebugMessages) {
            std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
        }
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            throw std::runtime_error("dumpTable: Read row length failed");
        if (printDebugMessages) {
            std::clog << "debug: dumpTable: rowLength=" << rowLength << std::endl;
        }

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        stdext::bitmask nullsMask;

        if (nullsExpected) {
            nullsMask.resize(columnCount, false);
            if (printDebugMessages) std::clog << "debug: Reading nulls bitmask" << std::endl;
            if (SIODB_UNLIKELY(!codedInput.ReadRaw(nullsMask.data(), nullsMask.size()))) {
                std::ostringstream err;
                err << "dumpTable: Read nulls bitmask from server failed: "
                    << std::strerror(input.GetErrno());
                throw std::system_error(input.GetErrno(), std::generic_category(), err.str());
            }
            if (printDebugMessages) std::clog << "debug: Read nulls bitmask" << std::endl;
        }

        if (printDebugMessages) std::clog << "debug: Reading TRID" << std::endl;
        std::uint64_t trid = 0;
        if (SIODB_UNLIKELY(!codedInput.Read(&trid)))
            throw std::runtime_error("dumpTable: Read TRID failed");
        if (printDebugMessages) std::clog << "debug: Read TRID: " << trid << std::endl;

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
            if (nullsExpected && nullsMask.get(i + 1))  // Skip TRID
                os << "NULL";
            else {
                const auto& column = table.m_columns.at(i);
                if (printDebugMessages)
                    std::clog << "debug: Reading value #" << i << " '" << column.m_name << "' ("
                              << siodb::iomgr::dbengine::getColumnDataTypeName(column.m_dataType)
                              << ')' << std::endl;
                std::string value;
                if (SIODB_UNLIKELY(!readValue(codedInput, column.m_dataType, value)))
                    throw std::runtime_error("Can't read value");
                os << value;
                if (printDebugMessages)
                    std::clog << "debug: Read value #" << i << ": " << value << std::endl;
            }
        }

        os << ");\n";
        ++rowCount;
    }

    if (printDebugMessages) {
        std::clog << "progress: Dumping " << rowCount << " rows from the table '" << databaseName
                  << '.' << table.m_name << '.' << std::endl;
    }
}

void dumpDatabaseDefinition(const DatabaseInfo& dbInfo, std::ostream& os)
{
    os << "\n-- Database: " << dbInfo.m_name << '\n'
       << buildCreateDatabaseStatement(dbInfo) << '\n';
}

void dumpTableDefinition(const std::string& databaseName, const TableInfo& table, std::ostream& os)
{
    os << '\n' << buildCreateTableStatement(databaseName, table) << '\n';
}

std::vector<DatabaseInfo> readDatabaseInfos(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, bool printDebugMessages)
{
    if (printDebugMessages) {
        std::clog << "progress: Reading databases..." << std::endl;
        std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
    }
    auto query = buildSelectStatementCore(kSystemDatabaseName, kSysDatabasesTableName,
            {kSysDatabases_Name_ColumnName, kSysDatabases_CipherId_ColumnName});
    const auto response = sendCommand(std::move(query), connection, input, printDebugMessages);

    std::vector<DatabaseInfo> databases;
    databases.reserve(16);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength))
            throw std::runtime_error("readDatabaseInfos: Read row length failed");
        if (printDebugMessages) {
            std::clog << "debug: readDatabaseInfos: rowLength=" << rowLength << std::endl;
        }

        if (rowLength == 0) break;

        DatabaseInfo database;
        if (SIODB_LIKELY(
                    codedInput.Read(&database.m_name) && codedInput.Read(&database.m_cipherId))) {
            if (database.m_name != kSystemDatabaseName) databases.push_back(std::move(database));
        } else
            throw std::runtime_error("readDatabaseInfos: Read database record failed");
    }

    if (printDebugMessages)
        std::clog << "progress: Read " << databases.size() << " databases." << std::endl;

    return databases;
}

DatabaseInfo readDatabaseInfo(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, bool printDebugMessages)
{
    if (printDebugMessages) {
        std::clog << "progress: Reading database '" << databaseName << "'..." << std::endl;
        std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
    }

    std::ostringstream oss;
    oss << buildSelectStatementCore(kSystemDatabaseName, kSysDatabasesTableName,
            {kSysDatabases_Name_ColumnName, kSysDatabases_CipherId_ColumnName})
        << " WHERE " << kSysDatabases_Name_ColumnName << "='" << databaseName << '\'';

    const auto response = sendCommand(oss.str(), connection, input, printDebugMessages);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::uint64_t rowLength = 0;
    if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
        throw std::runtime_error("readDatabaseInfo: Read row length failed");
    if (printDebugMessages) {
        std::clog << "debug: readDatabaseInfo(1): rowLength=" << rowLength << std::endl;
    }

    if (SIODB_UNLIKELY(rowLength == 0))
        throw std::runtime_error("readDatabaseInfo: Database doesn't exist");

    // Nulls are not allowed, so read values
    DatabaseInfo dbInfo;
    if (SIODB_LIKELY(codedInput.Read(&dbInfo.m_name) && codedInput.Read(&dbInfo.m_cipherId))) {
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            throw std::runtime_error("readDatabaseInfo: Read row length failed");
        if (printDebugMessages) {
            std::clog << "debug: readDatabaseInfo(2): rowLength=" << rowLength << std::endl;
        }
        if (SIODB_UNLIKELY(rowLength != 0))
            throw std::runtime_error("readDatabaseInfo: Invalid row length");
        if (printDebugMessages) std::clog << "Read database '" << databaseName << "'." << std::endl;
        return dbInfo;
    } else
        throw std::runtime_error("readDatabaseInfo: Read database record failed");
}

std::vector<TableInfo> readTableInfos(io::InputOutputStream& connection,
        protobuf::StreamInputStream& input, const std::string& databaseName,
        bool printDebugMessages)
{
    if (printDebugMessages) {
        std::clog << "progress: Reading tables of the database '" << databaseName << "'..."
                  << std::endl;
        std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
    }

    // SELECT TRID,NAME,CURRENT_COLUMN_SET_ID FROM SYS_TABLES
    // WHERE NAME NOT LIKE 'SYS_%'"
    std::ostringstream oss;
    oss << buildSelectStatementCore(databaseName, kSysTablesTableName,
            {kMasterColumnName, kSysTables_Name_ColumnName,
                    kSysTables_CurrentColumnSetId_ColumnName})
        << " WHERE " << kSysTables_Name_ColumnName << " NOT LIKE 'SYS_%' AND "
        << kSysTables_Type_ColumnName << "=1";
    auto response = sendCommand(oss.str(), connection, input, printDebugMessages);

    std::vector<TableInfo> tableInfos;
    tableInfos.reserve(16);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            throw std::runtime_error("readTableInfos: Read row length failed");
        if (printDebugMessages) {
            std::clog << "debug: readTableInfos: rowLength=" << rowLength << std::endl;
        }

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        TableInfo tableInfo;
        if (SIODB_LIKELY(codedInput.Read(&tableInfo.m_trid) && codedInput.Read(&tableInfo.m_name)
                         && codedInput.Read(&tableInfo.m_currentColumnSetId))) {
            tableInfos.push_back(std::move(tableInfo));
        } else
            throw std::runtime_error("readTableInfos: Read table record failed");
    }

    for (auto& tableInfo : tableInfos)
        readColumns(connection, input, databaseName, tableInfo, printDebugMessages);

    if (printDebugMessages) {
        std::clog << "progress: Database '" << databaseName << "': read " << tableInfos.size()
                  << " tables." << std::endl;
    }

    return tableInfos;
}

TableInfo readTableInfo(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::string& tableName, bool printDebugMessages)
{
    if (printDebugMessages) {
        std::clog << "progress: Reading table '" << databaseName << '.' << tableName << "'..."
                  << std::endl;
        std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
    }

    // SELECT TRID,NAME,CURRENT_COLUMN_SET_ID FROM SYS_TABLES
    // WHERE NAME NOT LIKE 'SYS_%'"
    std::ostringstream oss;
    oss << buildSelectStatementCore(databaseName, kSysTablesTableName,
            {kMasterColumnName, kSysTables_Name_ColumnName,
                    kSysTables_CurrentColumnSetId_ColumnName})
        << " WHERE " << kSysTables_Name_ColumnName << "='" << tableName << "' AND "
        << kSysTables_Type_ColumnName << "=1";
    auto response = sendCommand(oss.str(), connection, input, printDebugMessages);

    TableInfo tableInfo;

    protobuf::ExtendedCodedInputStream codedInput(&input);

    bool tableInfoRead = false;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            throw std::runtime_error("readTableInfos: Read row length failed");
        if (printDebugMessages) {
            std::clog << "debug: readTableInfo: rowLength=" << rowLength << std::endl;
        }

        if (tableInfoRead) {
            if (SIODB_LIKELY(rowLength == 0))
                break;
            else {
                throw std::runtime_error(
                        "readTableInfos: Extra rows when expecting information about a single "
                        "table");
            }
        }

        if (SIODB_UNLIKELY(rowLength == 0))
            throw std::runtime_error("readTableInfos: Table doesn't exist");

        if (SIODB_LIKELY(codedInput.Read(&tableInfo.m_trid) && codedInput.Read(&tableInfo.m_name)
                         && codedInput.Read(&tableInfo.m_currentColumnSetId))) {
            tableInfoRead = true;
            if (printDebugMessages) {
                std::clog << "progress: Read table '" << databaseName << '.' << tableName << "'"
                          << std::endl;
            }
        } else
            throw std::runtime_error("readTableInfos: Read table record failed");
    }
    readColumns(connection, input, databaseName, tableInfo, printDebugMessages);
    return tableInfo;
}

void readColumns(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, TableInfo& tableInfo, bool printDebugMessages)
{
    if (printDebugMessages) {
        std::clog << "progress: Reading columns of the table '" << databaseName << '.'
                  << tableInfo.m_name << "'..." << std::endl;
        std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
    }

    // SELECT TRID, COLUMN_ID SYS_COLUMN_SET_COLUMNS
    // WHERE COLUMN_SET_ID = value of <CURRENT_COLUMN_SET> in the SYS_TABLES
    std::ostringstream oss;
    oss << buildSelectStatementCore(databaseName, kSysColumnSetColumnsTableName,
            {kMasterColumnName, kSysColumnSetColumns_ColumnDefinitionId_ColumnName})
        << " WHERE " << kSysColumnSetColumns_ColumnSetId_ColumnName << '='
        << tableInfo.m_currentColumnSetId;

    auto response = sendCommand(oss.str(), connection, input, printDebugMessages);
    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::vector<ColumnSetInfo> columnSetInfos;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            throw std::runtime_error("readColumns: Read SYS_COLUMN_SET_COLUMNS row failed");
        if (printDebugMessages) {
            std::clog << "debug: readColumns(1): rowLength=" << rowLength << std::endl;
        }

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        // Nulls are not allowed, read values
        ColumnSetInfo columnSetInfo;
        if (SIODB_LIKELY(codedInput.Read(&columnSetInfo.m_trid)
                         && codedInput.Read(&columnSetInfo.m_columnDefinitionId)))
            columnSetInfos.push_back(std::move(columnSetInfo));
        else
            throw std::runtime_error("readColumns: Read column set record failed");
    }

    if (columnSetInfos.empty()) {
        tableInfo.m_columns.clear();
        if (printDebugMessages) {
            std::clog << "debug: Table '" << databaseName << '.' << tableInfo.m_name
                      << "': column set #" << tableInfo.m_currentColumnSetId << ": no columns"
                      << std::endl;
        }
        return;
    }

    // SELECT TRID, COLUMN_ID from SYS_COLUMN_DEFS
    // WHERE TRID IN (list of selected above m_columnDefinitionId);
    oss.str("");
    oss << buildSelectStatementCore(databaseName, kSysColumnDefsTableName,
            {kMasterColumnName, kSysColumnDefs_ColumnId_ColumnName})
        << " WHERE " << kMasterColumnName << " IN (";
    for (std::size_t i = 0; i < columnSetInfos.size(); ++i) {
        if (i > 0) oss << ',';
        oss << columnSetInfos[i].m_columnDefinitionId;
    }
    oss << ')';
    response = sendCommand(oss.str(), connection, input, printDebugMessages);

    std::unordered_map<std::uint64_t, std::uint64_t> columnIdToColumnDefIdMap;
    std::vector<ColumnDefinitionInfo> columnDefInfos;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            throw std::runtime_error("readColumns: Read SYS_COLUMN_DEFS row length failed");
        if (printDebugMessages) {
            std::clog << "debug: readColumns(2): rowLength=" << rowLength << std::endl;
        }

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

    // SELECT TRID, DATA_TYPE, NAME FROM SYS_COLUMNS
    // WHERE TRID IN (... list of retrieved above COLUMN_IDs ... )
    oss.str("");
    oss << buildSelectStatementCore(databaseName, kSysColumnsTableName,
            {kMasterColumnName, kSysColumns_DataType_ColumnName, kSysColumns_Name_ColumnName})
        << " WHERE " << kMasterColumnName << " IN (";
    for (std::size_t i = 0; i < columnDefInfos.size(); ++i) {
        if (i > 0) oss << ',';
        oss << columnDefInfos[i].m_columnId;
    }
    oss << ')';

    response = sendCommand(oss.str(), connection, input, printDebugMessages);

    std::vector<ColumnInfo> columns;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength)))
            throw std::runtime_error("readColumns: Read SYS_COLUMNS row length failed");
        if (printDebugMessages) {
            std::clog << "debug: readColumns(3): rowLength=" << rowLength << std::endl;
        }

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

    for (auto& columnInfo : columns) {
        readColumnConstraints(
                connection, input, databaseName, tableInfo.m_name, columnInfo, printDebugMessages);
    }

    tableInfo.m_columns = std::move(columns);

    if (printDebugMessages) {
        std::clog << "progress: Table '" << databaseName << '.' << tableInfo.m_name
                  << "': column set #" << tableInfo.m_currentColumnSetId << ": read "
                  << tableInfo.m_columns.size() << " columns" << std::endl;
    }
}

void readColumnConstraints(io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        const std::string& databaseName, const std::string& tableName, ColumnInfo& columnInfo,
        bool printDebugMessages)
{
    if (printDebugMessages) {
        std::clog << "progress: Reading constraints of the table column '" << databaseName << '.'
                  << tableName << '.' << columnInfo.m_name << "'..." << std::endl;
        std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
    }

    // SELECT CONSTRAINT_ID FROM SYS_COLUMN_DEF_CONSTRAINTS
    // WHERE COLUMN_DEF_ID = value of the earlier selected SYS_COLUMN_SET_COLUMNS.COLUMN_DEF_ID
    std::ostringstream oss;
    oss << buildSelectStatementCore(databaseName, kSysColumnDefConstraintsTableName,
            {kSysColumnDefinitionConstraintList_ConstraintId_ColumnName})
        << " WHERE " << kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnName << '='
        << columnInfo.m_columnDefinitionId;

    auto response = sendCommand(oss.str(), connection, input, printDebugMessages);

    protobuf::ExtendedCodedInputStream codedInput(&input);

    std::vector<std::uint64_t> constaintIds;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength))) {
            throw std::runtime_error(
                    "readColumnConstraints: Read SYS_COLUMN_SET_COLUMNS row length failed");
        }
        if (printDebugMessages) {
            std::clog << "debug: readColumnConstraints(1): rowLength=" << rowLength << std::endl;
        }

        if (rowLength == 0) break;

        std::uint64_t constaintId = 0;
        if (SIODB_LIKELY(codedInput.Read(&constaintId)))
            constaintIds.push_back(constaintId);
        else
            throw std::runtime_error("Read constraint ID failed");
    }

    if (constaintIds.empty()) {
        if (printDebugMessages) {
            std::clog << "progress: Table column '" << databaseName << '.' << tableName << '.'
                      << columnInfo.m_name << "': column definition #"
                      << columnInfo.m_columnDefinitionId << ": there are no constraints."
                      << std::endl;
        }
        return;
    }

    // SELECT TRID, NAME, CONSTRAINT_DEF_ID FROM SYS_CONSTRAINTS
    // WHERE TRID IN (list of selected above CONSTRAINT_ID)
    oss.str("");
    oss << buildSelectStatementCore(databaseName, kSysConstraintsTableName,
            {kSysConstraints_Name_ColumnName, kSysConstraints_DefinitionId_ColumnName})
        << " WHERE " << kMasterColumnName << " IN (";
    for (std::size_t i = 0; i < constaintIds.size(); ++i) {
        if (i > 0) oss << ',';
        oss << constaintIds[i];
    }
    oss << ')';

    response = sendCommand(oss.str(), connection, input, printDebugMessages);
    std::vector<ColumnConstraint> constraints;
    while (true) {
        std::uint64_t rowLength = 0;
        if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength))) {
            throw std::runtime_error(
                    "readColumnConstraints: Read SYS_CONSTRAINTS row length failed");
        }
        if (printDebugMessages) {
            std::clog << "debug: readColumnConstraints(2): rowLength=" << rowLength << std::endl;
        }

        if (SIODB_UNLIKELY(rowLength == 0)) break;

        ColumnConstraint constraint;
        if (SIODB_LIKELY(codedInput.Read(&constraint.m_name)
                         && codedInput.Read(&constraint.m_constraintDefinitionId)))
            constraints.push_back(std::move(constraint));
        else
            throw std::runtime_error("readColumnConstraints: Read constraint record failed");
    }

    for (auto& constraint : constraints) {
        // SELECT TYPE, EXPR FROM SYS_CONSTRAINT_DEFS
        // WHERE TRID = selected above CONSTRAINT_DEF_ID
        if (printDebugMessages) {
            std::clog << "debug: Reading constraint definition #"
                      << constraint.m_constraintDefinitionId << std::endl;
            std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
        }
        oss.str("");
        oss << buildSelectStatementCore(databaseName, kSysConstraintDefsTableName,
                {kSysConstraintDefs_Type_ColumnName, kSysConstraintDefs_Expr_ColumnName})
            << " WHERE " << kMasterColumnName << '=' << constraint.m_constraintDefinitionId;
        response = sendCommand(oss.str(), connection, input, printDebugMessages);
        const int columnCount = response.column_description_size();

        while (true) {
            std::uint64_t rowLength = 0;
            if (SIODB_UNLIKELY(!codedInput.ReadVarint64(&rowLength))) {
                throw std::runtime_error(
                        "readColumnConstraints: Read SYS_CONSTRAINT_DEFS row length failed");
            }
            if (printDebugMessages) {
                std::clog << "debug: readColumnConstraints(3): rowLength=" << rowLength
                          << std::endl;
            }

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

    columnInfo.m_constraints = std::move(constraints);

    if (printDebugMessages) {
        std::clog << "progress: Table column '" << databaseName << '.' << tableName << '.'
                  << columnInfo.m_name << "': column definition #"
                  << columnInfo.m_columnDefinitionId << ": read " << columnInfo.m_constraints.size()
                  << " constraints." << std::endl;
    }
}

client_protocol::ServerResponse sendCommand(std::string&& command,
        io::InputOutputStream& connection, protobuf::StreamInputStream& input,
        bool printDebugMessages)
{
    if (printDebugMessages) {
        std::clog << "debug: Sending command: \n----------\n"
                  << command << "\n----------\n\n"
                  << std::flush;
        std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
    }

    static std::int64_t requestId = 0;
    client_protocol::Command clientCommand;
    clientCommand.set_request_id(requestId);
    clientCommand.set_text(std::move(command));
    protobuf::writeMessage(protobuf::ProtocolMessageType::kCommand, clientCommand, connection);

    client_protocol::ServerResponse response;
    if (printDebugMessages) std::clog << "debug: Reading response..." << std::endl;
    protobuf::readMessage(protobuf::ProtocolMessageType::kServerResponse, response, input);
    if (printDebugMessages) {
        std::clog << "debug: Received response." << std::endl;
        std::clog << "debug: ===== stream bytes: " << input.ByteCount() << "=====" << std::endl;
    }
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
            std::uint32_t value = 0;
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
            std::ostringstream oss;
            oss << value;
            result = oss.str();
            return true;
        }

        case ColumnDataType::COLUMN_DATA_TYPE_DOUBLE: {
            double value = 0.0;
            if (SIODB_UNLIKELY(!codedInput.Read(&value))) return false;
            std::ostringstream oss;
            oss << value;
            result = oss.str();
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
                oss << std::setw(2) << static_cast<std::uint16_t>(bv[i]);
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
        const std::vector<std::string>& columnNames)
{
    return buildSelectStatementCore(databaseName.c_str(), tableName.c_str(), columnNames);
}

std::string buildSelectStatementCore(const char* databaseName, const char* tableName,
        const std::vector<std::string>& columnNames)
{
    std::ostringstream oss;
    oss << "SELECT ";
    for (std::size_t i = 0; i < columnNames.size(); ++i) {
        if (i > 0) oss << ',';
        oss << columnNames[i];
    }
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
