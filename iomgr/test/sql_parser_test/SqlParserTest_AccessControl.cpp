// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "TestContext.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"
#include "dbengine/parser/EmptyExpressionEvaluationContext.h"
#include "dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/config/SiodbDataFileDefs.h>
#include <siodb/iomgr/shared/dbengine/PermissionType.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;

TEST(AccessControl, GrantPermissionForTable1)
{
    // Parse statement and prepare request
    const std::string statement(
            "GRANT SELECT, INSERT, UPDATE, DELETE, DROP, ALTER, SHOW ON TABLE database1.table1 TO "
            "user1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kGrantPermissionsForTable);

    // Check request
    const auto& request =
            dynamic_cast<const requests::GrantPermissionsForTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "DATABASE1");
    EXPECT_EQ(request.m_table, "TABLE1");
    EXPECT_EQ(request.m_user, "USER1");
    const auto expectedPermissions =
            dbengine::buildMultiPermissionMask<dbengine::PermissionType::kSelect,
                    dbengine::PermissionType::kInsert, dbengine::PermissionType::kUpdate,
                    dbengine::PermissionType::kDelete, dbengine::PermissionType::kDrop,
                    dbengine::PermissionType::kAlter, dbengine::PermissionType::kShow>();
    EXPECT_EQ(request.m_permissions, expectedPermissions);
    EXPECT_FALSE(request.m_withGrantOption);
}

TEST(AccessControl, GrantPermissionForTable2)
{
    // Parse statement and prepare request
    const std::string statement("GRANT ALL ON TABLE table1 TO user1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kGrantPermissionsForTable);

    // Check request
    const auto& request =
            dynamic_cast<const requests::GrantPermissionsForTableRequest&>(*dbeRequest);
    EXPECT_TRUE(request.m_database.empty());
    EXPECT_EQ(request.m_table, "TABLE1");
    EXPECT_EQ(request.m_user, "USER1");
    const auto expectedPermissions =
            dbengine::buildMultiPermissionMask<dbengine::PermissionType::kSelect,
                    dbengine::PermissionType::kInsert, dbengine::PermissionType::kUpdate,
                    dbengine::PermissionType::kDelete, dbengine::PermissionType::kDrop,
                    dbengine::PermissionType::kAlter, dbengine::PermissionType::kShow>();
    EXPECT_EQ(request.m_permissions, expectedPermissions);
    EXPECT_FALSE(request.m_withGrantOption);
}

TEST(AccessControl, GrantPermissionForTable3)
{
    // Parse statement and prepare request
    const std::string statement("GRANT READ_ONLY ON database1.table1 TO user1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kGrantPermissionsForTable);

    // Check request
    const auto& request =
            dynamic_cast<const requests::GrantPermissionsForTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "DATABASE1");
    EXPECT_EQ(request.m_table, "TABLE1");
    EXPECT_EQ(request.m_user, "USER1");
    const auto expectedPermissions =
            dbengine::buildMultiPermissionMask<dbengine::PermissionType::kSelect,
                    dbengine::PermissionType::kShow>();
    EXPECT_EQ(request.m_permissions, expectedPermissions);
    EXPECT_FALSE(request.m_withGrantOption);
}

TEST(AccessControl, GrantPermissionForTable4)
{
    // Parse statement and prepare request
    const std::string statement("GRANT READ_WRITE ON database1.table1 TO user1 WITH GRANT OPTION");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kGrantPermissionsForTable);

    // Check request
    const auto& request =
            dynamic_cast<const requests::GrantPermissionsForTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "DATABASE1");
    EXPECT_EQ(request.m_table, "TABLE1");
    EXPECT_EQ(request.m_user, "USER1");
    const auto expectedPermissions =
            dbengine::buildMultiPermissionMask<dbengine::PermissionType::kSelect,
                    dbengine::PermissionType::kInsert, dbengine::PermissionType::kUpdate,
                    dbengine::PermissionType::kDelete, dbengine::PermissionType::kShow>();
    EXPECT_EQ(request.m_permissions, expectedPermissions);
    EXPECT_TRUE(request.m_withGrantOption);
}

TEST(AccessControl, RevokePermissionForTable1)
{
    // Parse statement and prepare request
    const std::string statement(
            "REVOKE SELECT, INSERT, UPDATE, DELETE, DROP, ALTER, SHOW ON TABLE database1.table1 "
            "FROM user1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kRevokePermissionsForTable);

    // Check request
    const auto& request =
            dynamic_cast<const requests::RevokePermissionsForTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "DATABASE1");
    EXPECT_EQ(request.m_table, "TABLE1");
    EXPECT_EQ(request.m_user, "USER1");
    const auto expectedPermissions =
            dbengine::buildMultiPermissionMask<dbengine::PermissionType::kSelect,
                    dbengine::PermissionType::kInsert, dbengine::PermissionType::kUpdate,
                    dbengine::PermissionType::kDelete, dbengine::PermissionType::kDrop,
                    dbengine::PermissionType::kAlter, dbengine::PermissionType::kShow>();
    EXPECT_EQ(request.m_permissions, expectedPermissions);
}

TEST(AccessControl, RevokePermissionForTable2)
{
    // Parse statement and prepare request
    const std::string statement("REVOKE ALL ON database1.table1 FROM user1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kRevokePermissionsForTable);

    // Check request
    const auto& request =
            dynamic_cast<const requests::RevokePermissionsForTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "DATABASE1");
    EXPECT_EQ(request.m_table, "TABLE1");
    EXPECT_EQ(request.m_user, "USER1");
    const auto expectedPermissions =
            dbengine::buildMultiPermissionMask<dbengine::PermissionType::kSelect,
                    dbengine::PermissionType::kInsert, dbengine::PermissionType::kUpdate,
                    dbengine::PermissionType::kDelete, dbengine::PermissionType::kDrop,
                    dbengine::PermissionType::kAlter, dbengine::PermissionType::kShow>();
    EXPECT_EQ(request.m_permissions, expectedPermissions);
}

TEST(AccessControl, RevokePermissionForTable3)
{
    // Parse statement and prepare request
    const std::string statement("REVOKE READ_ONLY ON table1 FROM user1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kRevokePermissionsForTable);

    // Check request
    const auto& request =
            dynamic_cast<const requests::RevokePermissionsForTableRequest&>(*dbeRequest);
    EXPECT_TRUE(request.m_database.empty());
    EXPECT_EQ(request.m_table, "TABLE1");
    EXPECT_EQ(request.m_user, "USER1");
    const auto expectedPermissions =
            dbengine::buildMultiPermissionMask<dbengine::PermissionType::kSelect,
                    dbengine::PermissionType::kShow>();
    EXPECT_EQ(request.m_permissions, expectedPermissions);
}

TEST(AccessControl, RevokePermissionForTable4)
{
    // Parse statement and prepare request
    const std::string statement("REVOKE READ_WRITE ON database1.table1 FROM user1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kRevokePermissionsForTable);

    // Check request
    const auto& request =
            dynamic_cast<const requests::RevokePermissionsForTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "DATABASE1");
    EXPECT_EQ(request.m_table, "TABLE1");
    EXPECT_EQ(request.m_user, "USER1");
    const auto expectedPermissions =
            dbengine::buildMultiPermissionMask<dbengine::PermissionType::kSelect,
                    dbengine::PermissionType::kShow, dbengine::PermissionType::kInsert,
                    dbengine::PermissionType::kUpdate, dbengine::PermissionType::kDelete>();
    EXPECT_EQ(request.m_permissions, expectedPermissions);
}
