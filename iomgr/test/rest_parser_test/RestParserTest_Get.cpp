// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/parser/DBEngineRestRequest.h"
#include "dbengine/parser/DBEngineRestRequestFactory.h"

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;
namespace req_ns = dbengine::requests;

TEST(Get, GetDatabases)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::DATABASE);

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

    // Check request object
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestGetDatabases);
    const auto r = dynamic_cast<const req_ns::GetDatabasesRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
}

TEST(Get, GetTables)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::TABLE);
    requestMsg.set_object_name_or_query("abcd");

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

    // Check request object
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestGetTables);
    const auto r = dynamic_cast<const req_ns::GetTablesRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->m_database, "ABCD");
}

TEST(Get, GetAllRows)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name_or_query("abcd.efgh");

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

    // Check request object
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestGetAllRows);
    const auto r = dynamic_cast<const req_ns::GetAllRowsRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->m_database, "ABCD");
    EXPECT_EQ(r->m_table, "EFGH");
}

TEST(Get, GetSingleRow)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name_or_query("abcd.efgh");
    requestMsg.set_object_id(1);

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

    // Check request object
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestGetSingleRow);
    const auto r = dynamic_cast<const req_ns::GetSingleRowRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->m_database, "ABCD");
    EXPECT_EQ(r->m_table, "EFGH");
    EXPECT_EQ(r->m_trid, 1U);
}

TEST(Get, GetSqlQueryRows)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::SQL);
    requestMsg.set_object_name_or_query("SELECT * FROM SYS_TABLES");

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

    // Check request object
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestGetSqlQueryRows);
    const auto r = dynamic_cast<const req_ns::GetSqlQueryRowsRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->m_query->m_database, "");
    ASSERT_EQ(r->m_query->m_tables.size(), 1U);
    EXPECT_EQ(r->m_query->m_tables[0].m_name, "SYS_TABLES");
    ASSERT_EQ(r->m_query->m_resultExpressions[0].m_expression->getType(),
            dbengine::requests::ExpressionType::kAllColumnsReference);
}
