// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"
#include "dbengine/parser/DBEngineRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>

// Boost headers
#include <boost/endian/conversion.hpp>

namespace parser_ns = dbengine::parser;

TEST(DML_Delete, DeleteAllRows)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"U16", siodb::COLUMN_DATA_TYPE_UINT16, true},
    };

    instance->getDatabase("SYS")->createUserTable("DELETE_TEST_1", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId);

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO DELETE_TEST_1 VALUES (0), (1), (2), (3), (4), (5)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto insertRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 6U);
    }

    /// ----------- DELETE -----------
    {
        const std::string statement("DELETE FROM DELETE_TEST_1");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto deleteRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*deleteRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 6U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT U16 FROM DELETE_TEST_1");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 1);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT16);
        EXPECT_EQ(response.column_description(0).name(), "U16");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(DML_Delete, DeleteByTrid)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I16", siodb::COLUMN_DATA_TYPE_INT16, true},
    };

    instance->getDatabase("SYS")->createUserTable("DELETE_TEST_2", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId);

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO DELETE_TEST_2 VALUES (0), (1), (2), (3), (4), (5)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto insertRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 6U);
    }

    /// ----------- DELETE -----------
    {
        const std::string statement("DELETE FROM DELETE_TEST_2 WHERE TRID >= 3");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto deleteRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*deleteRequest, TestEnvironment::kTestRequestId, 0, 1);

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
        const std::string statement("SELECT * FROM DELETE_TEST_2");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 2);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT16);
        EXPECT_EQ(response.column_description(1).name(), "I16");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (std::size_t i = 0; i < 2; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&trid));
            EXPECT_EQ(trid, i + 1);

            std::int16_t int16 = 0;
            ASSERT_TRUE(codedInput.ReadRaw(&int16, 2));
            boost::endian::little_to_native_inplace(int16);
            EXPECT_EQ(int16, static_cast<std::int16_t>(i));
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(DML_Delete, DeleteByTridWithTableName)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I16", siodb::COLUMN_DATA_TYPE_INT16, true},
    };

    instance->getDatabase("SYS")->createUserTable("DELETE_TEST_3", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId);

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO DELETE_TEST_3 VALUES (0), (1), (2), (3), (4), (5)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto insertRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 6U);
    }

    /// ----------- DELETE -----------
    {
        const std::string statement("DELETE FROM SYS.DELETE_TEST_3 WHERE DELETE_TEST_3.TRID >= 3");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto deleteRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*deleteRequest, TestEnvironment::kTestRequestId, 0, 1);

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
        const std::string statement("SELECT * FROM DELETE_TEST_3");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 2);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT16);
        EXPECT_EQ(response.column_description(1).name(), "I16");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (std::size_t i = 0; i < 2; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&trid));
            EXPECT_EQ(trid, i + 1);

            std::int16_t int16 = 0;
            ASSERT_TRUE(codedInput.ReadRaw(&int16, 2));
            boost::endian::little_to_native_inplace(int16);
            EXPECT_EQ(int16, static_cast<std::int16_t>(i));
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(DML_Delete, DeleteByTridWithTableAlias)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I16", siodb::COLUMN_DATA_TYPE_INT16, true},
    };

    instance->getDatabase("SYS")->createUserTable("DELETE_TEST_4", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId);

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO DELETE_TEST_4 VALUES (0), (1), (2), (3), (4), (5)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto insertRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 6U);
    }

    /// ----------- DELETE -----------
    {
        const std::string statement(
                "DELETE FROM SYS.DELETE_TEST_4 AS TBL_ALIAS WHERE TBL_ALIAS.TRID >= 3");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto deleteRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*deleteRequest, TestEnvironment::kTestRequestId, 0, 1);

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
        const std::string statement("SELECT * FROM DELETE_TEST_4");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 2);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT16);
        EXPECT_EQ(response.column_description(1).name(), "I16");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (std::size_t i = 0; i < 2; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&trid));
            EXPECT_EQ(trid, i + 1);

            std::int16_t int16 = 0;
            ASSERT_TRUE(codedInput.ReadRaw(&int16, 2));
            boost::endian::little_to_native_inplace(int16);
            EXPECT_EQ(int16, static_cast<std::int16_t>(i));
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(DML_Delete, DeleteByMutlipleColumnsExpression)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I16", siodb::COLUMN_DATA_TYPE_INT16, true},
            {"U16", siodb::COLUMN_DATA_TYPE_UINT16, true},
            {"U64", siodb::COLUMN_DATA_TYPE_UINT64, true},
    };

    instance->getDatabase("SYS")->createUserTable("DELETE_TEST_5", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId);

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO DELETE_TEST_5 VALUES"
                "(0, 50, 100),"
                "(50, 50, 100),"
                "(100, 50, 100),"
                "(150, 50, 100),"
                "(200, 50, 100)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto insertRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 5U);
    }

    /// ----------- DELETE -----------
    {
        const std::string statement(
                "DELETE FROM DELETE_TEST_5 WHERE NOT (U64 > (U16 + DELETE_TEST_5.I16))");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto deleteRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*deleteRequest, TestEnvironment::kTestRequestId, 0, 1);

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
        const std::string statement("SELECT I16, U16, U64 FROM DELETE_TEST_5");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 3);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT16);
        EXPECT_EQ(response.column_description(0).name(), "I16");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_UINT16);
        EXPECT_EQ(response.column_description(1).name(), "U16");
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(2).name(), "U64");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0U);

        std::int16_t int16 = 0;
        ASSERT_TRUE(codedInput.ReadRaw(&int16, 2));
        boost::endian::little_to_native_inplace(int16);
        EXPECT_EQ(int16, 0);

        std::uint16_t uint16 = 0;
        ASSERT_TRUE(codedInput.ReadRaw(&uint16, 2));
        boost::endian::little_to_native_inplace(uint16);
        EXPECT_EQ(uint16, 50u);

        std::uint64_t uint64 = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&uint64));
        EXPECT_EQ(uint64, 100u);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}
