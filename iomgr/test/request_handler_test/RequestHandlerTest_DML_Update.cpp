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

TEST(DML_Update, UpdateAllValues)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I16", siodb::COLUMN_DATA_TYPE_INT16, true},
    };

    instance->getDatabase("SYS")->createUserTable("UPDATE_TEST_1", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO UPDATE_TEST_1 VALUES (0), (1), (2), (3), (4), (5)");

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

    /// ----------- UPDATE -----------
    {
        const std::string statement("UPDATE SYS.UPDATE_TEST_1 SET I16=I16+100");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto updateRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*updateRequest, TestEnvironment::kTestRequestId, 0, 1);

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
        const std::string statement("SELECT * FROM UPDATE_TEST_1");
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
        for (std::size_t i = 0; i < 6; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&trid));
            EXPECT_EQ(trid, i + 1);

            std::int16_t int16 = 0;
            ASSERT_TRUE(codedInput.ReadRaw(&int16, 2));
            boost::endian::little_to_native_inplace(int16);
            EXPECT_EQ(int16, static_cast<std::int16_t>(i + 100));
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(DML_Update, UpdateWhereByTRID)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"U8", siodb::COLUMN_DATA_TYPE_UINT8, true},
    };

    instance->getDatabase("SYS")->createUserTable("UPDATE_TEST_2", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        const std::string statement("INSERT INTO UPDATE_TEST_2 VALUES (10), (20), (30)");

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
        ASSERT_EQ(response.affected_row_count(), 3U);
    }

    /// ----------- UPDATE -----------
    {
        const std::string statement("UPDATE SYS.UPDATE_TEST_2 SET U8=TRID WHERE TRID=2");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto updateRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*updateRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 1U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM UPDATE_TEST_2");
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
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_UINT8);
        EXPECT_EQ(response.column_description(1).name(), "U8");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0U);

        std::uint64_t trid = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&trid));
        EXPECT_EQ(trid, 1U);

        std::uint8_t uint8 = 0;
        ASSERT_TRUE(codedInput.ReadRaw(&uint8, 1));
        EXPECT_EQ(uint8, 10U);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0U);

        ASSERT_TRUE(codedInput.ReadVarint64(&trid));
        EXPECT_EQ(trid, 2U);

        ASSERT_TRUE(codedInput.ReadRaw(&uint8, 1));
        EXPECT_EQ(uint8, 2U);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0U);

        ASSERT_TRUE(codedInput.ReadVarint64(&trid));
        EXPECT_EQ(trid, 3U);

        ASSERT_TRUE(codedInput.ReadRaw(&uint8, 1));
        EXPECT_EQ(uint8, 30u);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(DML_Update, UpdateOneColumnFromThree)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"F", siodb::COLUMN_DATA_TYPE_FLOAT, true},
            {"DT", siodb::COLUMN_DATA_TYPE_TIMESTAMP, true},
            {"D", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
    };

    instance->getDatabase("SYS")->createUserTable("UPDATE_TEST_3", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO UPDATE_TEST_3 VALUES"
                "(0.0, '2019-11-11', 0.00),"
                "(0.1, '2019-11-12', 0.01),"
                "(0.2, '2019-11-13', 0.02)");

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
        ASSERT_EQ(response.affected_row_count(), 3U);
    }

    /// ----------- UPDATE -----------
    {
        const std::string statement(
                "UPDATE SYS.UPDATE_TEST_3 SET DT='2017-11-10' WHERE DT BETWEEN '2019-11-11' AND "
                "'2019-11-12'");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto updateRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*updateRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 2U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM UPDATE_TEST_3");
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
        ASSERT_EQ(response.column_description_size(), 4);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_FLOAT);
        EXPECT_EQ(response.column_description(1).name(), "F");
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_TIMESTAMP);
        EXPECT_EQ(response.column_description(2).name(), "DT");
        ASSERT_EQ(response.column_description(3).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);
        EXPECT_EQ(response.column_description(3).name(), "D");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (std::size_t i = 0; i < 3; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&trid));
            EXPECT_EQ(trid, i + 1);

            float flt = 0;
            ASSERT_TRUE(codedInput.ReadLittleEndian32(reinterpret_cast<std::uint32_t*>(&flt)));
            EXPECT_FLOAT_EQ(flt, i * 0.1f);

            siodb::RawDateTime date;
            ASSERT_TRUE(siodb::protobuf::readRawDateTime(codedInput, date));
            if (i < 2) {
                EXPECT_EQ(date.m_datePart.m_year, 2017);
                EXPECT_EQ(date.m_datePart.m_month, 10U);
                EXPECT_EQ(date.m_datePart.m_dayOfMonth, 9U);
                EXPECT_FALSE(date.m_datePart.m_hasTimePart);
            } else {
                EXPECT_EQ(date.m_datePart.m_year, 2019);
                EXPECT_EQ(date.m_datePart.m_month, 10U);
                EXPECT_EQ(date.m_datePart.m_dayOfMonth, 12u);
                EXPECT_FALSE(date.m_datePart.m_hasTimePart);
            }

            double dbl = 0;
            ASSERT_TRUE(codedInput.ReadLittleEndian64(reinterpret_cast<std::uint64_t*>(&dbl)));
            EXPECT_NEAR(dbl, i * 0.01, 0.0001);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(DML_Update, UpdateSeveralColumns)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"U8", siodb::COLUMN_DATA_TYPE_UINT8, true},
            {"U32", siodb::COLUMN_DATA_TYPE_UINT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("UPDATE_TEST_4", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        // U8 {1, 2, 3, 4, 5}
        // U32 {5, 4, 3, 2, 1}
        // TRID {1, 2, 3, 4, 5}
        const std::string statement(
                "INSERT INTO UPDATE_TEST_4 VALUES (1, 5), (2, 4), (3, 3), (4, 2) , (5, 1)");

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

    /// ----------- UPDATE -----------
    {
        // U8 => {2, 4, 7, 4, 5}
        // U32 => {15, 14, 13, 2, 1}
        const std::string statement(
                "UPDATE SYS.UPDATE_TEST_4 SET U32=10+U32, U8=U8+TRID WHERE "
                "U8 <= 3");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto updateRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*updateRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 3U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM UPDATE_TEST_4");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_UINT8);
        EXPECT_EQ(response.column_description(1).name(), "U8");
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_UINT32);
        EXPECT_EQ(response.column_description(2).name(), "U32");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;

        std::uint64_t expectedTrid[] = {1, 2, 3, 4, 5};
        std::uint8_t expectedU8[] = {2, 4, 6, 4, 5};
        std::uint32_t expectedU32[] = {15, 14, 13, 2, 1};
        for (std::size_t i = 0; i < 5; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            std::uint64_t trid;
            ASSERT_TRUE(codedInput.ReadVarint64(&trid));
            EXPECT_EQ(trid, expectedTrid[i]);

            std::uint8_t uint8 = 0;
            ASSERT_TRUE(codedInput.ReadRaw(&uint8, 1));
            EXPECT_EQ(uint8, expectedU8[i]);

            std::uint32_t uint32 = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(&uint32));
            EXPECT_EQ(uint32, expectedU32[i]);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(DML_Update, UpdateConcatString)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"T", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"U8", siodb::COLUMN_DATA_TYPE_UINT8, true},
    };

    instance->getDatabase("SYS")->createUserTable("UPDATE_TEST_5", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO UPDATE_TEST_5 VALUES ('', 0), ('A', 1), ('AA', 2)");

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
        ASSERT_EQ(response.affected_row_count(), 3U);
    }

    /// ----------- UPDATE -----------
    {
        const std::string statement("UPDATE SYS.UPDATE_TEST_5 SET T=T+'B'");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto updateRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*updateRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 3U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT T FROM UPDATE_TEST_5");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        EXPECT_EQ(response.column_description(0).name(), "T");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (std::size_t i = 0; i < 3; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            std::string expectedText(i, 'A');
            expectedText += 'B';

            std::uint32_t textLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(&textLength));
            ASSERT_EQ(textLength, i + 1U);
            std::string text(i + 1U, '\0');
            ASSERT_TRUE(codedInput.ReadRaw(text.data(), textLength));
            EXPECT_TRUE(text == expectedText);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}
