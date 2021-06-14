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

TEST(Query, SelectWithLimit)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->findDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_1",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::ostringstream oss;
        oss << "INSERT INTO SYS.SELECT_WITH_LIMIT_1 VALUES (0), (1), (2), (3), (4), (5), (6), (7), "
               "(8), (9)";

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT A FROM SYS.SELECT_WITH_LIMIT_1 LIMIT 5");
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
        for (auto i = 0; i < 5; ++i) {
            rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_GT(rowLength, 0);

            std::int32_t a = 0;
            ASSERT_TRUE(codedInput.Read(&a));
            ASSERT_EQ(a, i);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithZeroLimit)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->findDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_2",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::ostringstream oss;
        oss << "INSERT INTO SYS.SELECT_WITH_LIMIT_2 VALUES (0), (1), (2), (3), (4), (5), (6), (7), "
               "(8), (9)";

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT A FROM SYS.SELECT_WITH_LIMIT_2 LIMIT 0");
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
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithNegativeLimit)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->findDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_3",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::ostringstream oss;
        oss << "INSERT INTO SYS.SELECT_WITH_LIMIT_2 VALUES (0), (1), (2), (3), (4), (5), (6), (7), "
               "(8), (9)";

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT A FROM SYS.SELECT_WITH_LIMIT_3 LIMIT -1");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 1);  // LIMIT -1 Exception message
    }
}

TEST(Query, SelectWithLimitAndOffset)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->findDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_AND_OFFSET_1",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::ostringstream oss;
        oss << "INSERT INTO SYS.SELECT_WITH_LIMIT_AND_OFFSET_1 VALUES (0), (1), (2), (3), (4), "
               "(5), "
               "(6), (7), "
               "(8), (9)";

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT A FROM SYS.SELECT_WITH_LIMIT_AND_OFFSET_1 LIMIT 5 OFFSET 5");
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
        for (auto i = 5; i < 10; ++i) {
            rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_GT(rowLength, 0);

            std::int32_t a = 0;
            ASSERT_TRUE(codedInput.Read(&a));
            ASSERT_EQ(a, i);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithLimitAndOffsetLargerThanRowCount)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->findDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_AND_OFFSET_2",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::ostringstream oss;
        oss << "INSERT INTO SYS.SELECT_WITH_LIMIT_AND_OFFSET_2 VALUES (0), (1), (2), (3), (4), "
               "(5), "
               "(6), (7), "
               "(8), (9)";

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT A FROM SYS.SELECT_WITH_LIMIT_AND_OFFSET_2 LIMIT 5 OFFSET 10");
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
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithNegativeOffset)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->findDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_AND_OFFSET_3",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::ostringstream oss;
        oss << "INSERT INTO SYS.SELECT_WITH_LIMIT_AND_OFFSET_3 VALUES (0), (1), (2), (3), (4), "
               "(5), "
               "(6), (7), "
               "(8), (9)";

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT A FROM SYS.SELECT_WITH_LIMIT_AND_OFFSET_3 LIMIT 10 OFFSET -1");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 1);  // LIMIT -1 Exception message
    }
}

TEST(Query, SelectWithWhere_LimitAndOffset)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->findDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_LIMIT_AND_OFFSET_1",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::ostringstream oss;
        oss << "INSERT INTO SYS.SELECT_WITH_WHERE_LIMIT_AND_OFFSET_1 VALUES (0), (1), (2), (3), "
               "(4), (5), "
               "(6), (7), "
               "(8), (9)";

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT A FROM SYS.SELECT_WITH_WHERE_LIMIT_AND_OFFSET_1 WHERE A > 3 LIMIT 5 "
                "OFFSET 5");
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
        for (auto i = 9; i < 10; ++i) {
            rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_GT(rowLength, 0);

            std::int32_t a = 0;
            ASSERT_TRUE(codedInput.Read(&a));
            ASSERT_EQ(a, i);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}
