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
