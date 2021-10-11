// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/protobuf/ExtendedCodedInputStream.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>
#include <siodb/common/stl_ext/sstream_ext.h>
#include <siodb/iomgr/shared/dbengine/SystemObjectNames.h>
#include <siodb/iomgr/shared/dbengine/parser/CommonConstants.h>
#include <siodb/iomgr/shared/dbengine/util/RowDecoder.h>

namespace parser_ns = dbengine::parser;
namespace util_ns = dbengine::util;
namespace req_ns = dbengine::requests;

namespace {

using CollectedPermissions =
        std::map<std::tuple<std::string, std::string, std::string, std::string, std::string>, bool>;

constexpr int kShowPermissionsResponseColumnCount = 6;

void validateShowPermissionsResponse(const siodb::iomgr_protocol::DatabaseEngineResponse& response)
{
    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_FALSE(response.has_affected_row_count());
    ASSERT_EQ(response.column_description_size(), kShowPermissionsResponseColumnCount);
    ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_TEXT);
    ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_TEXT);
    ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_TEXT);
    ASSERT_EQ(response.column_description(3).type(), siodb::COLUMN_DATA_TYPE_TEXT);
    ASSERT_EQ(response.column_description(4).type(), siodb::COLUMN_DATA_TYPE_TEXT);
    ASSERT_EQ(response.column_description(5).type(), siodb::COLUMN_DATA_TYPE_BOOL);
    EXPECT_EQ(response.column_description(0).name(), "USER");
    EXPECT_EQ(response.column_description(1).name(), "DATABASE");
    EXPECT_EQ(response.column_description(2).name(), "OBJECT_TYPE");
    EXPECT_EQ(response.column_description(3).name(), "OBJECT_NAME");
    EXPECT_EQ(response.column_description(4).name(), "PERMISSION");
    EXPECT_EQ(response.column_description(5).name(), "GRANT_OPTION");
    EXPECT_FALSE(response.column_description(0).is_nullable());
    EXPECT_FALSE(response.column_description(1).is_nullable());
    EXPECT_FALSE(response.column_description(2).is_nullable());
    EXPECT_FALSE(response.column_description(3).is_nullable());
    EXPECT_FALSE(response.column_description(4).is_nullable());
    EXPECT_FALSE(response.column_description(5).is_nullable());
}

void readAndCheckPermissions(dbengine::RequestHandler& requestHandler, const std::string& userName,
        const CollectedPermissions& expectedPermissions)
{
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    std::string statement = "SHOW PERMISSIONS";
    if (!userName.empty()) statement += " FOR " + userName;

    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto request = factory.createSqlRequest();
    requestHandler.executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);
    validateShowPermissionsResponse(response);

    std::vector<siodb::ColumnDataType> dataTypesForDecoding;
    for (const auto& columnDescription : response.column_description())
        dataTypesForDecoding.push_back(columnDescription.type());

    CollectedPermissions actualPermissions;
    siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

    std::size_t rowCount = 0;
    while (true) {
        const auto decoded = util_ns::decodeRow(inputStream, codedInput,
                kShowPermissionsResponseColumnCount, dataTypesForDecoding.data());
        if (decoded.empty()) break;
        ASSERT_EQ(decoded.size(), kShowPermissionsResponseColumnCount);
        ++rowCount;
        actualPermissions.emplace(
                std::make_tuple(decoded[0].getString(), decoded[1].getString(),
                        decoded[2].getString(), decoded[3].getString(), decoded[4].getString()),
                decoded[5].getBool());
    }
    ASSERT_EQ(rowCount, expectedPermissions.size());
    ASSERT_EQ(actualPermissions, expectedPermissions);
}

}  // namespace

TEST(UserPermissions, ShowPermissions_SuperUser)
{
    // Prepare expected permissions
    const CollectedPermissions expectedPermissions {
            {std::make_tuple("ROOT", req_ns::kAllObjectsName, req_ns::kAllObjectsName,
                     req_ns::kAllObjectsName, req_ns::kAllObjectsName),
                    true},
    };

    // Read and compare permissions
    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();
    readAndCheckPermissions(*requestHandler, std::string(), expectedPermissions);
}

TEST(UserPermissions, ShowPermissions_NormalUser)
{
    // Create table
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
    };
    const auto database = instance->findDatabase(TestEnvironment::getTestDatabaseName());
    constexpr const char* kTableName = "SHOW_PERM_FOR_NORMAL_USER";
    const auto table = database->createUserTable(kTableName, dbengine::TableType::kDisk,
            tableColumns, TestEnvironment::getTestUserId(2), {});

    // Prepare expected permissions
    const auto userName = TestEnvironment::getTestUserName(2);
    const auto dbName = database->getName();
    const CollectedPermissions expectedPermissions {
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Database", dbName, "Show"), false},
            {std::make_tuple(userName, dbName, "Table", req_ns::kAllObjectsName, "Create"), false},
            {std::make_tuple(userName, dbName, "Table", kTableName, "Select"), true},
            {std::make_tuple(userName, dbName, "Table", kTableName, "Insert"), true},
            {std::make_tuple(userName, dbName, "Table", kTableName, "Delete"), true},
            {std::make_tuple(userName, dbName, "Table", kTableName, "Update"), true},
            {std::make_tuple(userName, dbName, "Table", kTableName, "Show"), true},
            {std::make_tuple(userName, dbName, "Table", kTableName, "Drop"), true},
            {std::make_tuple(userName, dbName, "Table", kTableName, "Alter"), true},
            {std::make_tuple(
                     userName, dbName, "Table", dbengine::kSysTablesTableName, "SelectSystem"),
                    false},
    };

    // Read and compare permissions
    const auto requestHandler = TestEnvironment::makeRequestHandlerForNormalUser(2);
    readAndCheckPermissions(*requestHandler, std::string(), expectedPermissions);
}

TEST(UserPermissions, ShowPermissions_CheckAllSupportedPermissions)
{
    // Create user
    const auto instance = TestEnvironment::getInstance();
    const auto userName = stdext::concat("USER_PERM_TEST_", std::time(nullptr));
    const auto user = instance->createUser(userName, {}, {}, true, dbengine::User::kSuperUserId);

    // Grant permissions
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kInstance, 0,
            dbengine::kShutdownPermissionMask, false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kDatabase, 0,
            dbengine::kAttachPermissionMask | dbengine::kDetachPermissionMask
                    | dbengine::kCreatePermissionMask | dbengine::kDropPermissionMask
                    | dbengine::kAlterPermissionMask | dbengine::kShowPermissionMask,
            false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kTable, 0,
            dbengine::kSelectPermissionMask | dbengine::kShowSystemPermissionMask
                    | dbengine::kSelectSystemPermissionMask | dbengine::kInsertPermissionMask
                    | dbengine::kUpdatePermissionMask | dbengine::kDeletePermissionMask,
            false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kColumn, 0,
            dbengine::kAlterPermissionMask, false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kIndex, 0,
            dbengine::kDropPermissionMask, false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kConstraint, 0,
            dbengine::kDropPermissionMask, false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kTrigger, 0,
            dbengine::kDropPermissionMask, false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kProcedure, 0,
            dbengine::kDropPermissionMask, false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kFunction, 0,
            dbengine::kDropPermissionMask, false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kUser, 0,
            dbengine::kShowPermissionsPermissionMask, false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kUserAccessKey,
            0, dbengine::kDropPermissionMask, false, dbengine::User::kSuperUserId);
    instance->grantObjectPermissionsToUser(*user, 0, dbengine::DatabaseObjectType::kUserToken, 0,
            dbengine::kDropPermissionMask, false, dbengine::User::kSuperUserId);

    // Prepare expected permissions
    const CollectedPermissions expectedPermissions {
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Instance",
                     "00000000-0000-0000-0000-000000000000", "Shutdown"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Database", req_ns::kAllObjectsName,
                     "Attach"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Database", req_ns::kAllObjectsName,
                     "Detach"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Database", req_ns::kAllObjectsName,
                     "Create"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Database", req_ns::kAllObjectsName,
                     "Drop"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Database", req_ns::kAllObjectsName,
                     "Alter"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Database", req_ns::kAllObjectsName,
                     "Show"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Table", req_ns::kAllObjectsName,
                     "ShowSystem"),
                    false},
            {std::make_tuple(
                     userName, req_ns::kAllObjectsName, "Table", req_ns::kAllObjectsName, "Select"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Table", req_ns::kAllObjectsName,
                     "SelectSystem"),
                    false},
            {std::make_tuple(
                     userName, req_ns::kAllObjectsName, "Table", req_ns::kAllObjectsName, "Insert"),
                    false},
            {std::make_tuple(
                     userName, req_ns::kAllObjectsName, "Table", req_ns::kAllObjectsName, "Update"),
                    false},
            {std::make_tuple(
                     userName, req_ns::kAllObjectsName, "Table", req_ns::kAllObjectsName, "Delete"),
                    false},
            {std::make_tuple(
                     userName, req_ns::kAllObjectsName, "Column", req_ns::kAllObjectsName, "Alter"),
                    false},
            {std::make_tuple(
                     userName, req_ns::kAllObjectsName, "Index", req_ns::kAllObjectsName, "Drop"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Constraint",
                     req_ns::kAllObjectsName, "Drop"),
                    false},
            {std::make_tuple(
                     userName, req_ns::kAllObjectsName, "Trigger", req_ns::kAllObjectsName, "Drop"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Procedure",
                     req_ns::kAllObjectsName, "Drop"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "Function", req_ns::kAllObjectsName,
                     "Drop"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "User", req_ns::kAllObjectsName,
                     "ShowPermissions"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "UserAccessKey",
                     req_ns::kAllObjectsName, "Drop"),
                    false},
            {std::make_tuple(userName, req_ns::kAllObjectsName, "UserToken",
                     req_ns::kAllObjectsName, "Drop"),
                    false},
    };

    // Read and compare permissions
    // Note: this also Checks case "SHOW PERMISSIONS FOR username" run by user for itself
    const auto requestHandler = TestEnvironment::makeRequestHandlerForUser(userName);
    readAndCheckPermissions(*requestHandler, userName, expectedPermissions);
}
