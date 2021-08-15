// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
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

// Boost headers
#include <boost/endian/conversion.hpp>

// Google Test
#include <gtest/gtest.h>

namespace requests = dbengine::requests;
namespace parser_ns = dbengine::parser;

/** Test makes several inserts/deletes
 * And then checks result table data with select */
TEST(DML_Complex, ComplexInsertDeleteTest)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I8", siodb::COLUMN_DATA_TYPE_INT8, true},
            {"U64", siodb::COLUMN_DATA_TYPE_UINT64, true},
    };

    instance->findDatabase("SYS")->createUserTable("COMPLEX_TEST_1", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    // ----------- INSERT -----------
    {
        const std::string statement("INSERT INTO COMPLEX_TEST_1 VALUES(1, 1000000)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- DELETE -----------
    {
        const std::string statement("DELETE FROM COMPLEX_TEST_1 WHERE I8 = 1");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- INSERT -----------
    {
        const std::string statement("INSERT INTO COMPLEX_TEST_1 VALUES(2, 2000000), (3, 3000000)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- INSERT -----------
    {
        const std::string statement("INSERT INTO COMPLEX_TEST_1 VALUES(4, 4000000), (5, 5000000)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- DELETE -----------
    {
        const std::string statement("DELETE FROM COMPLEX_TEST_1 WHERE U64 = 3000000");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- DELETE -----------
    {
        const std::string statement("DELETE FROM COMPLEX_TEST_1 WHERE I8 = 5 AND U64 = 5000000");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM COMPLEX_TEST_1");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT8);
        EXPECT_EQ(response.column_description(1).name(), "I8");
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(2).name(), "U64");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);
        std::uint64_t rowLength = 0;
        for (std::size_t i : {2U, 4U}) {
            rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_GT(rowLength, 0U);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.Read(&trid));
            EXPECT_EQ(trid, i);

            std::int8_t int8Value = 0;
            ASSERT_TRUE(codedInput.Read(&int8Value));
            EXPECT_EQ(int8Value, static_cast<std::int8_t>(i));

            std::uint64_t uint64Value = 0;
            ASSERT_TRUE(codedInput.Read(&uint64Value));
            EXPECT_EQ(uint64Value, 1000000u * i);
        }
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

/** Test makes several inserts/updates
 * And then checks result table data with select */
TEST(DML_Complex, ComplexInsertUpdateTest)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I8", siodb::COLUMN_DATA_TYPE_INT8, true},
            {"U64", siodb::COLUMN_DATA_TYPE_UINT64, true},
    };

    instance->findDatabase("SYS")->createUserTable("COMPLEX_TEST_2", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    // ----------- Update -----------
    {
        // Table is empty now
        const std::string statement("UPDATE COMPLEX_TEST_2 SET U64=23185854094843");

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
        ASSERT_EQ(response.affected_row_count(), 0U);
    }

    // ----------- INSERT -----------
    {
        const std::string statement("INSERT INTO COMPLEX_TEST_2 VALUES(1, 1000000)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- UPDATE -----------
    {
        const std::string statement("UPDATE COMPLEX_TEST_2 SET U64=999999");

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
        ASSERT_EQ(response.affected_row_count(), 1U);
    }

    // ----------- INSERT -----------
    {
        const std::string statement("INSERT INTO COMPLEX_TEST_2 VALUES(2, 2000000), (3, 3000000)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- INSERT -----------
    {
        const std::string statement("INSERT INTO COMPLEX_TEST_2 VALUES(4, 4000000), (5, 5000000)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- UPDATE -----------
    {
        const std::string statement("UPDATE COMPLEX_TEST_2 SET U64=1000000 WHERE U64=999999");

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
        ASSERT_EQ(response.affected_row_count(), 1U);
    }

    // ----------- UPDATE -----------
    {
        // Reverse order 1,2,3,4,5 -> 5,4,3,2,1 and the same for U64
        const std::string statement("UPDATE COMPLEX_TEST_2 SET U64=6000000-U64, I8=6-I8");

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

    // ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM COMPLEX_TEST_2");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT8);
        EXPECT_EQ(response.column_description(1).name(), "I8");
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(2).name(), "U64");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);
        std::uint64_t rowLength = 0;
        for (auto i = 1U; i <= 5U; ++i) {
            rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_GT(rowLength, 0U);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.Read(&trid));
            EXPECT_EQ(trid, i);

            std::int8_t int8Value = 0;
            ASSERT_TRUE(codedInput.Read(&int8Value));
            EXPECT_EQ(int8Value, static_cast<std::int8_t>(6 - i));

            std::uint64_t uint64Value = 0;
            ASSERT_TRUE(codedInput.Read(&uint64Value));
            EXPECT_EQ(uint64Value, 6000000llu - (1000000llu * i));
        }
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

/** Test performs a list of actions below
 * 1) Inserts 3 values into test table
 * 2) Updates 2 of 3 values from test table
 * 3) Deletes one of updated value
 * 4) Selects data and validates result */
TEST(DML_Complex, ComplexInsertUpdateDeleteTest)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I8", siodb::COLUMN_DATA_TYPE_INT8, true},
            {"U64", siodb::COLUMN_DATA_TYPE_UINT64, true},
    };

    instance->findDatabase("SYS")->createUserTable("COMPLEX_TEST_3", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    // ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO COMPLEX_TEST_3 VALUES(1, 1000000), (20, 20000000), (30, 30000000)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
    }

    // ----------- UPDATE -----------
    {
        const std::string statement(
                "UPDATE COMPLEX_TEST_3 SET U64=U64/10, I8=I8/10 WHERE U64 > 3000000 AND I8 > 3");

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
        ASSERT_EQ(response.affected_row_count(), 2U);
    }

    // ----------- DELETE -----------
    {
        const std::string statement("DELETE FROM COMPLEX_TEST_3 WHERE U64 = 3000000");

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
        ASSERT_EQ(response.affected_row_count(), 1U);
    }

    // ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM COMPLEX_TEST_3");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT8);
        EXPECT_EQ(response.column_description(1).name(), "I8");
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(2).name(), "U64");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);
        std::uint64_t rowLength = 0;
        for (auto i = 1U; i <= 2U; ++i) {
            rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_GT(rowLength, 0U);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.Read(&trid));
            EXPECT_EQ(trid, i);

            std::int8_t int8Value = 0;
            ASSERT_TRUE(codedInput.Read(&int8Value));
            EXPECT_EQ(int8Value, static_cast<std::int8_t>(i));

            std::uint64_t uint64Value = 0;
            ASSERT_TRUE(codedInput.Read(&uint64Value));
            EXPECT_EQ(uint64Value, 1000000u * i);
        }
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}
