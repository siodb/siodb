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
#include <siodb/iomgr/shared/dbengine/SystemObjectNames.h>
#include <siodb/iomgr/shared/dbengine/util/RowDecoder.h>

namespace parser_ns = dbengine::parser;
namespace util_ns = dbengine::util;

TEST(UserPermissions, ShowPermissions_SuperUser)
{
    // Create request handler
    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();

    // Prepare input stream
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    {
        const std::string statement("SHOW PERMISSIONS");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        constexpr int kColumnCount = 6;
        ASSERT_EQ(response.column_description_size(), kColumnCount);
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
        EXPECT_FALSE(response.column_description(0).is_null());
        EXPECT_FALSE(response.column_description(1).is_null());
        EXPECT_FALSE(response.column_description(2).is_null());
        EXPECT_FALSE(response.column_description(3).is_null());
        EXPECT_FALSE(response.column_description(4).is_null());
        EXPECT_FALSE(response.column_description(5).is_null());

        std::vector<siodb::ColumnDataType> dataTypesForDecoding;
        for (const auto& columnDescription : response.column_description())
            dataTypesForDecoding.push_back(columnDescription.type());

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);
        const auto decoded = util_ns::decodeRow(
                inputStream, codedInput, kColumnCount, dataTypesForDecoding.data());
        ASSERT_EQ(decoded.size(), kColumnCount);
        ASSERT_EQ(decoded[0], "ROOT");
        ASSERT_EQ(decoded[1], "*");
        ASSERT_EQ(decoded[2], "*");
        ASSERT_EQ(decoded[3], "*");
        ASSERT_EQ(decoded[4], "*");
        ASSERT_EQ(decoded[5], true);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(UserPermissions, ShowPermissions_NormalUser)
{
    // Create request handler
    const auto requestHandler = TestEnvironment::makeRequestHandlerForNormalUser(2);

    // Prepare input stream
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

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

    {
        const std::string statement("SHOW PERMISSIONS");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        constexpr int kColumnCount = 6;
        ASSERT_EQ(response.column_description_size(), kColumnCount);
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
        EXPECT_FALSE(response.column_description(0).is_null());
        EXPECT_FALSE(response.column_description(1).is_null());
        EXPECT_FALSE(response.column_description(2).is_null());
        EXPECT_FALSE(response.column_description(3).is_null());
        EXPECT_FALSE(response.column_description(4).is_null());
        EXPECT_FALSE(response.column_description(5).is_null());

        std::vector<siodb::ColumnDataType> dataTypesForDecoding;
        for (const auto& columnDescription : response.column_description())
            dataTypesForDecoding.push_back(columnDescription.type());

        using CollectedPermissions =
                std::map<std::tuple<std::string, std::string, std::string, std::string>,
                        std::pair<std::string, bool>>;

        CollectedPermissions actualPermissions;
        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

//#define DEBUG_ShowPermissions_NormalUser
#ifdef DEBUG_ShowPermissions_NormalUser
        std::cout << std::endl;
#endif
        std::size_t rowCount = 0;
        while (true) {
            const auto decoded = util_ns::decodeRow(
                    inputStream, codedInput, kColumnCount, dataTypesForDecoding.data());
            if (decoded.empty()) break;
            ASSERT_EQ(decoded.size(), kColumnCount);
            ++rowCount;

#ifdef DEBUG_ShowPermissions_NormalUser
            bool first = true;
            for (const auto& v : decoded) {
                if (!first) std::cout << '\t';
                std::cout << v;
                first = false;
            }
            std::cout << std::endl;
#endif

            actualPermissions.emplace(
                    std::make_tuple(decoded[0].getString(), decoded[1].getString(),
                            decoded[2].getString(), decoded[3].getString()),
                    std::make_pair(decoded[4].getString(), decoded[5].getBool()));
        }

#ifdef DEBUG_ShowPermissions_NormalUser
        std::cout << "ROW COUNT=" << rowCount << std::endl;
#endif

        ASSERT_EQ(rowCount, 10);

        const auto userName = TestEnvironment::getTestUserName(2);
        const auto dbName = database->getName();
        const CollectedPermissions expectedPermissions {
                {std::make_tuple(userName, "*", "Database", dbName), std::make_pair("Show", false)},
                {std::make_tuple(userName, dbName, "Table", "*"), std::make_pair("Create", false)},
                {std::make_tuple(userName, dbName, "Table", kTableName),
                        std::make_pair("Select", true)},
                {std::make_tuple(userName, dbName, "Table", kTableName),
                        std::make_pair("Insert", true)},
                {std::make_tuple(userName, dbName, "Table", kTableName),
                        std::make_pair("Delete", true)},
                {std::make_tuple(userName, dbName, "Table", kTableName),
                        std::make_pair("Update", true)},
                {std::make_tuple(userName, dbName, "Table", kTableName),
                        std::make_pair("Show", true)},
                {std::make_tuple(userName, dbName, "Table", kTableName),
                        std::make_pair("Drop", true)},
                {std::make_tuple(userName, dbName, "Table", kTableName),
                        std::make_pair("Alter", true)},
                {std::make_tuple(userName, dbName, "Table", dbengine::kSysTablesTableName),
                        std::make_pair("SelectSystem", false)},
        };

        ASSERT_EQ(actualPermissions, expectedPermissions);
    }
}

TEST(UserPermissions, SelectFromTable_WithPermission)
{
    // Create request handler
    const auto requestHandler = TestEnvironment::makeRequestHandlerForNormalUser();

    // Prepare input stream
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
    };
    const auto database = instance->findDatabase(TestEnvironment::getTestDatabaseName());
    constexpr const char* kTableName = "PERM_SELECT_FROM_TABLE_WITH_PERM";
    const auto table = database->createUserTable(kTableName, dbengine::TableType::kDisk,
            tableColumns, TestEnvironment::getTestUserId(0), {});

    // Insert some data into table
    const dbengine::TransactionParameters tp(TestEnvironment::getTestUserId(0),
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
    };
    table->insertRow(std::move(values), tp);

    // Try to select from table as permitted user
    {
        const std::string statement(
                "SELECT A FROM " + TestEnvironment::getTestDatabaseName() + "." + kTableName);
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 1);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT32);

        EXPECT_EQ(response.column_description(0).name(), "A");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_GT(rowLength, 0);
        std::int32_t a = 0;
        ASSERT_TRUE(codedInput.Read(&a));
        ASSERT_EQ(a, 1);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(UserPermissions, SelectFromTable_WithoutPermission)
{
    // Prepare input stream
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
    };
    const auto database = instance->findDatabase(TestEnvironment::getTestDatabaseName());
    constexpr const char* kTableName = "PERM_SELECT_FROM_TABLE_WITHOUT_PERM";
    const auto table = database->createUserTable(kTableName, dbengine::TableType::kDisk,
            tableColumns, TestEnvironment::getTestUserId(0), {});

    // Insert some data into table
    const dbengine::TransactionParameters tp(TestEnvironment::getTestUserId(0),
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
    };
    table->insertRow(std::move(values), tp);

    // Try to select from table as non-permitted user
    {
        // Create request handler
        const auto requestHandler = TestEnvironment::makeRequestHandlerForNormalUser(1);

        const std::string statement(
                "SELECT A FROM " + TestEnvironment::getTestDatabaseName() + "." + kTableName);
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.message_size(), 1);
    }
}

// TODO: Add more user permissions tests here
