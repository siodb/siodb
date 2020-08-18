// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "DBEngineRequest.h"
#include "expr/Expression.h"

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine::requests {

/** GET databases request */
struct GetDatabasesRestRequest : public DBEngineRequest {
    /** Initializes object of class GetDatabasesRestRequest. */
    GetDatabasesRestRequest() noexcept
        : DBEngineRequest(DBEngineRequestType::kRestGetDatabases)
    {
    }
};

/** GET tables request */
struct GetTablesRestRequest : public DBEngineRequest {
    /**
     * Initializes object of class GetTablesRestRequest.
     * @param database A database.
     */
    explicit GetTablesRestRequest(std::string&& database) noexcept
        : DBEngineRequest(DBEngineRequestType::kRestGetTables)
        , m_database(std::move(database))
    {
    }

    /** Database name */
    const std::string m_database;
};

/** GET all rows request */
struct GetAllRowsRestRequest : public DBEngineRequest {
    /**
     * Initializes object of class GetAllRowsRestRequest.
     * @param database A database.
     * @param table A table.
     */
    explicit GetAllRowsRestRequest(std::string&& database, std::string&& table) noexcept
        : DBEngineRequest(DBEngineRequestType::kRestGetAllRows)
        , m_database(std::move(database))
        , m_table(std::move(table))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;
};

/** GET single row request */
struct GetSingleRowRestRequest : public DBEngineRequest {
    /**
     * Initializes object of class GetSingleRowRestRequest.
     * @param database A database.
     * @param table A table.
     * @param trid Table row ID.
     */
    explicit GetSingleRowRestRequest(
            std::string&& database, std::string&& table, std::uint64_t trid) noexcept
        : DBEngineRequest(DBEngineRequestType::kRestGetSingleRow)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_trid(trid)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** Table row ID */
    const std::uint64_t m_trid;
};

/** POST row request */
struct PostRowsRestRequest : public DBEngineRequest {
    /**
     * Initializes object of class PostRowsRestRequest.
     * @param database A database.
     * @param table A table.
     * @param values Column values.
     */
    explicit PostRowsRestRequest(std::string&& database, std::string&& table,
            std::vector<std::vector<ConstExpressionPtr>>&& values) noexcept
        : DBEngineRequest(DBEngineRequestType::kRestPostRows)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_values(std::move(values))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** Column values */
    const std::vector<std::vector<ConstExpressionPtr>> m_values;
};

}  // namespace siodb::iomgr::dbengine::requests
