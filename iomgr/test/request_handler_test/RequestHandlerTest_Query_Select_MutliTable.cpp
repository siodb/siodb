// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"
#include "dbengine/SystemDatabase.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/protobuf/ExtendedCodedInputStream.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>

namespace parser_ns = dbengine::parser;

TEST(Query, SelectFrom2Tables)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> table1Columns {
            {"I8", siodb::COLUMN_DATA_TYPE_INT8, true},
    };

    const std::vector<dbengine::SimpleColumnSpecification> table2Columns {
            {"F", siodb::COLUMN_DATA_TYPE_FLOAT, true},
            {"B", siodb::COLUMN_DATA_TYPE_BOOL, true},
    };

    instance->findDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_7_1",
            dbengine::TableType::kDisk, table1Columns, dbengine::User::kSuperUserId, {});

    instance->findDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_7_2",
            dbengine::TableType::kDisk, table2Columns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT_1 -----------
    {
        std::ostringstream oss;
        oss << "INSERT INTO SYS.SELECT_WITH_WHERE_7_1 VALUES (0),(1),(2),(3),(4)";

        const auto statement = oss.str();

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
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 5U);
    }

    /// ----------- INSERT_2 -----------
    {
        std::ostringstream oss;
        oss << "INSERT INTO SYS.SELECT_WITH_WHERE_7_2 VALUES (6.0, false), (5.0, false), (4.0, "
               "false),(3.0, false), (2.0, true), (1.0, true), (0.0, true)";

        const auto statement = oss.str();

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
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 7U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT SELECT_WITH_WHERE_7_1.I8, SELECT_WITH_WHERE_7_2.B, SELECT_WITH_WHERE_7_2.F "
                "FROM SYS.SELECT_WITH_WHERE_7_1, SELECT_WITH_WHERE_7_2 WHERE "
                "SELECT_WITH_WHERE_7_2.B = true");
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
        ASSERT_EQ(response.column_description_size(), 3);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT8);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_BOOL);
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_FLOAT);

        EXPECT_EQ(response.column_description(0).name(), "I8");
        EXPECT_EQ(response.column_description(1).name(), "B");
        EXPECT_EQ(response.column_description(2).name(), "F");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (std::size_t i = 0; i < 5; ++i) {
            for (int j = 2; j >= 0; --j) {
                rowLength = 0;
                ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
                ASSERT_GT(rowLength, 0U);

                std::int8_t int8Value = 0;
                ASSERT_TRUE(codedInput.Read(&int8Value));
                EXPECT_EQ(int8Value, static_cast<std::int8_t>(i));

                bool boolValue = false;
                ASSERT_TRUE(codedInput.Read(&boolValue));
                EXPECT_EQ(boolValue, true);

                float floatValue = 0;
                ASSERT_TRUE(codedInput.Read(&floatValue));
                EXPECT_FLOAT_EQ(floatValue, j * 1.0f);
            }
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectFrom3TablesWithSameColumns)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create tables
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"CTEXT", siodb::COLUMN_DATA_TYPE_TEXT, true},
    };
    const auto db = instance->findDatabase("SYS");
    for (int i = 0; i < 3; ++i) {
        auto tableName = "S3T_" + std::to_string(i + 1);
        db->createUserTable(std::move(tableName), dbengine::TableType::kDisk, tableColumns,
                dbengine::User::kSuperUserId, {});
    }

    /// ----------- INSERT -----------
    {
        const std::string statement = "INSERT INTO SYS.S3T_1 VALUES ('a1'), ('b1'), ('c1'), ('d1')";
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
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 4U);
    }
    {
        const std::string statement = "INSERT INTO SYS.S3T_2 VALUES ('a2'), ('b2'), ('c2'), ('d2')";
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
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 4U);
    }
    {
        const std::string statement = "INSERT INTO SYS.S3T_3 VALUES ('a2'), ('b2'), ('c2'), ('d2')";
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
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 4U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "select * from sys.S3T_1 tab1, sys.S3T_2 tab2, sys.S3T_3 tab3 where tab1.trid = "
                "tab2.trid and tab2.trid=tab3.trid");

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
        ASSERT_EQ(response.column_description_size(), 2);  // + TRID
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        EXPECT_EQ(response.column_description(1).name(), "CTEXT");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        char expectedText[3];
        expectedText[1] = '1';
        expectedText[2] = '\0';
        for (auto i = 0U; i < 4U; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_GT(rowLength, 0);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.Read(&trid));
            ASSERT_EQ(trid, i + 1);

            expectedText[0] = 'a' + i;
            std::string text;
            ASSERT_TRUE(codedInput.Read(&text));
            EXPECT_EQ(text, expectedText);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}
