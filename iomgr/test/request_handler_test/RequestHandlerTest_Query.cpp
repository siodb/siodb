// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"
#include "dbengine/parser/DBEngineRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>

namespace parser_ns = dbengine::parser;

// SELECT * FROM SYS.SYS_DATABASES
TEST(Query, SelectFromSys_Databases)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    instance->createDatabase(
            "TEST", "none", siodb::BinaryValue(), dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    const std::string statement("SELECT * FROM SYS.SYS_DATABASES");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto selectRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);

    google::protobuf::io::CodedInputStream codedInput(&inputStream);

    std::uint64_t rowLength = 0;
    // SYS and TEST databases

    for (std::size_t i = 0, n = instance->getDatbaseCount(); i < n; ++i) {
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength < 200);
        ASSERT_TRUE(rowLength > 0);
        siodb::BinaryValue rowData(rowLength);
        ASSERT_TRUE(codedInput.ReadRaw(rowData.data(), rowLength));
    }

    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    EXPECT_EQ(rowLength, 0U);
}

TEST(Query, ShowDatabases)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    const std::string statement("SHOW DATABASES");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto showDatabasesRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    requestHandler->executeRequest(*showDatabasesRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 2);
    EXPECT_EQ(response.column_description(0).name(), "NAME");
    EXPECT_EQ(response.column_description(1).name(), "UUID");

    google::protobuf::io::CodedInputStream codedInput(&inputStream);

    std::uint64_t rowLength = 0;
    for (std::size_t i = 0, n = instance->getDatbaseCount(); i < n; ++i) {
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength < 100);
        ASSERT_TRUE(rowLength > 0);
        std::vector<std::uint8_t> rowData(rowLength);
        ASSERT_TRUE(codedInput.ReadRaw(rowData.data(), rowLength));
    }

    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    EXPECT_EQ(rowLength, 0U);
}

TEST(Query, SelectWithWhere)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_1", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_WHERE_1 VALUES ";
        // a is always 300, b is [-0, 100, ..., 1000]
        std::int32_t bValue = 0;
        ss << '(' << 300 << ',' << bValue << ')';
        for (auto i = 1; i < 10; ++i) {
            ss << ", (" << 300 << ',' << bValue + (100 * i) << ')';
        }

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM SYS.SELECT_WITH_WHERE_1 WHERE (A*2) > B");
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
        ASSERT_EQ(response.column_description_size(), 3);  // + TRID
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT32);
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_INT32);

        // Table order
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        EXPECT_EQ(response.column_description(1).name(), "A");
        EXPECT_EQ(response.column_description(2).name(), "B");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);
        std::uint64_t rowLength = 0;
        for (auto i = 0; i < 6; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&trid));
            ASSERT_TRUE(trid > 0);

            std::int32_t a = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(reinterpret_cast<std::uint32_t*>(&a)));
            ASSERT_TRUE(a == 300);

            std::int32_t b = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(reinterpret_cast<std::uint32_t*>(&b)));
            ASSERT_TRUE(b == std::int32_t((100 * i)));
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithWhereBetweenDatetime)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"DT", siodb::COLUMN_DATA_TYPE_TIMESTAMP, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_2", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_WHERE_2 VALUES";
        ss << "('2012-03-12'),";
        ss << "('2015-03-01'),";
        ss << "('2015-03-02'),";
        ss << "('2015-03-03'),";
        ss << "('2019-03-14')";

        const std::string statement(ss.str());

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

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT DT FROM SYS.SELECT_WITH_WHERE_2 WHERE DT BETWEEN '2015-03-01' AND "
                "'2015-03-03'");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_TIMESTAMP);

        EXPECT_EQ(response.column_description(0).name(), "DT");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (unsigned i = 0U; i < 3U; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            siodb::RawDateTime date;
            ASSERT_TRUE(siodb::protobuf::readRawDateTime(codedInput, date));
            EXPECT_EQ(date.m_datePart.m_year, 2015);
            EXPECT_EQ(date.m_datePart.m_month, 2U);
            EXPECT_EQ(date.m_datePart.m_dayOfMonth, i);
            EXPECT_FALSE(date.m_datePart.m_hasTimePart);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithWhereCompoundExpression)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"D", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
            {"I8", siodb::COLUMN_DATA_TYPE_INT8, true},
            {"U32", siodb::COLUMN_DATA_TYPE_UINT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_3", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_WHERE_3 VALUES";
        ss << "(0.0, 0, 4000000),";
        ss << "(4.0, 2, 3000000),";
        ss << "(8.0, 8, 20000000),";
        ss << "(16.0, 32, 10000000),";  // 32 > 16
        ss << "(32.0, 64, 10000000),";  // 64 > 32
        ss << "(64.0, 127, 0)";  // 127 > 64

        const std::string statement(ss.str());

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

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT D, I8, U32 FROM SYS.SELECT_WITH_WHERE_3 WHERE ((U32 + I8) / 2) > (D + "
                "U32) / 2");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT8);
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_UINT32);

        EXPECT_EQ(response.column_description(0).name(), "D");
        EXPECT_EQ(response.column_description(1).name(), "I8");
        EXPECT_EQ(response.column_description(2).name(), "U32");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (auto i = 0U; i < 3U; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);
            double dbl = 0;
            ASSERT_TRUE(codedInput.ReadLittleEndian64(reinterpret_cast<std::uint64_t*>(&dbl)));

            std::int8_t int8 = 0;
            ASSERT_TRUE(codedInput.ReadRaw(&int8, sizeof(std::int8_t)));

            std::uint32_t uint32 = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(&uint32));

            EXPECT_TRUE(((uint32 + int8) / 2) > (uint32 + dbl) / 2);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithWhereNonSelectedColumn)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"U8", siodb::COLUMN_DATA_TYPE_UINT8, true},
            {"I64", siodb::COLUMN_DATA_TYPE_INT64, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_4", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_WHERE_4 VALUES ";
        ss << "(0, 100),";
        ss << "(1, 200),";
        ss << "(2, 300)";

        const std::string statement(ss.str());

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

    /// ----------- SELECT -----------
    {
        // 3 kinds of column expressions in where:
        // 1) <NoTable> column
        // 2) Table.Column
        // 3) TableAlias.Column
        for (const std::string statement :
                {"SELECT I64 FROM SYS.SELECT_WITH_WHERE_4 WHERE U8 = 1"
                 "SELECT I64 FROM SYS.SELECT_WITH_WHERE_4 WHERE SELECT_WITH_WHERE_4.U8 = 1",
                        "SELECT I64 FROM SYS.SELECT_WITH_WHERE_4 as T WHERE T.U8 = 1"}) {
            parser_ns::SqlParser parser(statement);
            parser.parse();

            const auto selectRequest =
                    parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

            siodb::iomgr_protocol::DatabaseEngineResponse response;
            requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
            siodb::protobuf::readMessage(
                    siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response,
                    inputStream);

            EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
            ASSERT_EQ(response.message_size(), 0);
            EXPECT_FALSE(response.has_affected_row_count());
            ASSERT_EQ(response.column_description_size(), 1);
            ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT64);

            EXPECT_EQ(response.column_description(0).name(), "I64");

            google::protobuf::io::CodedInputStream codedInput(&inputStream);

            std::uint64_t rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            std::int64_t int64 = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(reinterpret_cast<std::uint64_t*>(&int64)));
            EXPECT_EQ(int64, 200);

            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            EXPECT_EQ(rowLength, 0U);
        }

        // Check column alias
        for (const std::string statement :
                {"SELECT I64 FROM SYS.SELECT_WITH_WHERE_4 WHERE U8 = 1"
                 "SELECT I64 FROM SYS.SELECT_WITH_WHERE_4 WHERE SELECT_WITH_WHERE_4.U8 = 1",
                        "SELECT I64 FROM SYS.SELECT_WITH_WHERE_4 as T WHERE T.U8 = 1"}) {
            parser_ns::SqlParser parser(statement);
            parser.parse();

            const auto selectRequest =
                    parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

            siodb::iomgr_protocol::DatabaseEngineResponse response;
            requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
            siodb::protobuf::readMessage(
                    siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response,
                    inputStream);

            EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
            ASSERT_EQ(response.message_size(), 0);
            EXPECT_FALSE(response.has_affected_row_count());
            ASSERT_EQ(response.column_description_size(), 1);
            ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT64);

            EXPECT_EQ(response.column_description(0).name(), "I64");

            google::protobuf::io::CodedInputStream codedInput(&inputStream);

            std::uint64_t rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            std::int64_t int64 = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(reinterpret_cast<std::uint64_t*>(&int64)));
            EXPECT_EQ(int64, 200);

            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            EXPECT_EQ(rowLength, 0U);
        }
    }
}

/** Select with using aliased table in WHERE */
TEST(Query, SelectWithWhereUsingTableAlias)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_WITH_TABLE_ALIAS",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO SELECT_WITH_WHERE_WITH_TABLE_ALIAS VALUES (0), (1), (2)");

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

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT ALIASED_TABLE.A AS ALIASED_COLUMN FROM "
                "SELECT_WITH_WHERE_WITH_TABLE_ALIAS AS ALIASED_TABLE WHERE ALIASED_TABLE.A = 1");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT32);

        // Table order
        EXPECT_EQ(response.column_description(0).name(), "ALIASED_COLUMN");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);
        std::uint64_t rowLength = 0;

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0);

        std::int32_t a = 0;
        ASSERT_TRUE(codedInput.ReadVarint32(reinterpret_cast<std::uint32_t*>(&a)));
        ASSERT_TRUE(a == 1);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithWhereColumnAlias)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"C", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_5", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        const std::string statement(
                "INSERT INTO SYS.SELECT_WITH_WHERE_5 VALUES (1),(2),(3),(4),(5)");

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

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT C AS AC FROM SYS.SELECT_WITH_WHERE_5 WHERE C = 2");
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
        ASSERT_EQ(response.column_description_size(), 1);  // + TRID
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT32);

        EXPECT_EQ(response.column_description(0).name(), "AC");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0);

        std::int32_t a = 0;
        ASSERT_TRUE(codedInput.ReadVarint32(reinterpret_cast<std::uint32_t*>(&a)));
        ASSERT_TRUE(a == 2);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithWhereBetweenAndLogicalAnd)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"DT", siodb::COLUMN_DATA_TYPE_TIMESTAMP, true},
            {"T", siodb::COLUMN_DATA_TYPE_TEXT, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_6", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_WHERE_6 VALUES";
        ss << "('2012-03-12', 'abc'),";
        ss << "('2015-03-01', 'bca'),";
        ss << "('2015-03-02', 'abc'),";
        ss << "('2015-03-03', 'cab'),";
        ss << "('2019-03-14',  'bac')";

        const std::string statement(ss.str());

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

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT DT, T FROM SYS.SELECT_WITH_WHERE_6 WHERE DT BETWEEN '2015-03-01' AND "
                "'2015-03-03' AND T = 'abc'");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_TIMESTAMP);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_TEXT);

        EXPECT_EQ(response.column_description(0).name(), "DT");
        EXPECT_EQ(response.column_description(1).name(), "T");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0U);

        // Read 2015-03-02', 'abc'
        siodb::RawDateTime date;
        ASSERT_TRUE(siodb::protobuf::readRawDateTime(codedInput, date));
        EXPECT_EQ(date.m_datePart.m_year, 2015);
        EXPECT_EQ(date.m_datePart.m_month, 2U);
        EXPECT_EQ(date.m_datePart.m_dayOfMonth, 1U);
        EXPECT_FALSE(date.m_datePart.m_hasTimePart);

        std::uint32_t textLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint32(&textLength));
        ASSERT_EQ(textLength, 3U);
        std::string text(3, '\0');
        ASSERT_TRUE(codedInput.ReadRaw(text.data(), textLength));
        EXPECT_TRUE(text == "abc");

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectFrom2Tables)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> table1Columns {
            {"I8", siodb::COLUMN_DATA_TYPE_INT8, true},
    };

    const std::vector<dbengine::SimpleColumnSpecification> table2Columns {
            {"F", siodb::COLUMN_DATA_TYPE_FLOAT, true},
            {"B", siodb::COLUMN_DATA_TYPE_BOOL, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_7_1",
            dbengine::TableType::kDisk, table1Columns, dbengine::User::kSuperUserId, {});

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_7_2",
            dbengine::TableType::kDisk, table2Columns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT_1 -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_WHERE_7_1 VALUES (0),(1),(2),(3),(4)";

        const std::string statement(ss.str());

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

    /// ----------- INSERT_2 -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_WHERE_7_2 VALUES (6.0, false), (5.0, false), (4.0, "
              "false),(3.0, false), (2.0, true), (1.0, true), (0.0, true)";

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT8);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_BOOL);
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_FLOAT);

        EXPECT_EQ(response.column_description(0).name(), "I8");
        EXPECT_EQ(response.column_description(1).name(), "B");
        EXPECT_EQ(response.column_description(2).name(), "F");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;

        for (std::size_t i = 0; i < 5; ++i) {
            for (int j = 2; j >= 0; --j) {
                ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
                ASSERT_TRUE(rowLength > 0U);

                std::int8_t i8 = 0;
                ASSERT_TRUE(codedInput.ReadRaw(&i8, 1));
                EXPECT_EQ(i8, static_cast<std::int8_t>(i));

                std::uint8_t boolValue = 2;
                ASSERT_TRUE(codedInput.ReadRaw(&boolValue, 1));
                EXPECT_EQ(boolValue, std::uint8_t(1));

                float flt = 0;
                ASSERT_TRUE(codedInput.ReadLittleEndian32(reinterpret_cast<std::uint32_t*>(&flt)));
                EXPECT_FLOAT_EQ(flt, j * 1.0f);
            }
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

TEST(Query, SelectWithExpression)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> table1Columns {
            {"U32", siodb::COLUMN_DATA_TYPE_UINT32, true},
            {"U16", siodb::COLUMN_DATA_TYPE_UINT16, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_8", dbengine::TableType::kDisk,
            table1Columns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_WHERE_8 VALUES (0, 0),(10, 1),(20, 2),(30, 3),(40, 4)";

        const std::string statement(ss.str());

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

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT U32 + U16 AS TEST FROM SYS.SELECT_WITH_WHERE_8 WHERE U32 + U16 > 22");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT32);
        EXPECT_EQ(response.column_description(0).name(), "TEST");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;

        for (std::size_t i = 3; i < 5; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0U);

            siodb::utils::Bitmask nullBitmask(response.column_description_size(), false);
            ASSERT_TRUE(codedInput.ReadRaw(nullBitmask.getData(), nullBitmask.getByteSize()));
            ASSERT_FALSE(nullBitmask.getBit(0));

            std::uint32_t u32 = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(&u32));
            EXPECT_EQ(u32, static_cast<std::uint32_t>(i * 10 + i));
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

/**
 * Test executes query below:
 * SELECT sys_tables.name, sys_columns.name from sys_tables, sys_columns 
 *        WHERE sys_tables.trid = sys_columns.table_id AND sys_tables.trid < 4096;
 */
TEST(Query, SelectWithExpressionFrom2Tables)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT sys_tables.name, sys_columns.name from sys_tables, sys_columns WHERE "
                "sys_tables.trid = sys_columns.table_id AND sys_tables.trid < 4096;");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        EXPECT_EQ(response.column_description(0).name(), "NAME");
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        EXPECT_EQ(response.column_description(1).name(), "NAME");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;

        do {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            std::vector<std::uint8_t> data(rowLength);
            ASSERT_TRUE(codedInput.ReadRaw(data.data(), rowLength));
        } while (rowLength > 0);
    }
}

/**
 * Selects expressions from table with a null value
 */
TEST(Query, SelectWithExpressionWithNull)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> table1Columns {
            {"I64", siodb::COLUMN_DATA_TYPE_INT64, true},
    };

    instance->getDatabase("SYS")->createUserTable("TEST_EXPRESSION", dbengine::TableType::kDisk,
            table1Columns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.TEST_EXPRESSION VALUES (10)";

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.affected_row_count(), 1U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT NULL, 13, I64 + 0, I64 + NULL FROM SYS.TEST_EXPRESSION");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UNKNOWN);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_UINT8);
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_INT64);
        ASSERT_EQ(response.column_description(3).type(), siodb::COLUMN_DATA_TYPE_UNKNOWN);

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0U);

        siodb::utils::Bitmask nullBitmask(response.column_description_size(), false);
        ASSERT_TRUE(codedInput.ReadRaw(nullBitmask.getData(), nullBitmask.getByteSize()));
        ASSERT_TRUE(nullBitmask.getBit(0));
        ASSERT_FALSE(nullBitmask.getBit(1));
        ASSERT_FALSE(nullBitmask.getBit(2));
        ASSERT_TRUE(nullBitmask.getBit(3));

        std::uint8_t u8 = 0;
        ASSERT_TRUE(codedInput.ReadRaw(&u8, 1));
        EXPECT_EQ(u8, 13u);

        std::int64_t i64 = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(reinterpret_cast<std::uint64_t*>(&i64)));
        EXPECT_EQ(i64, 10);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

/**
 * Selects expression from empty table
 */
TEST(Query, SelectWithExpressionWithEmptyTable)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> table1Columns {
            {"I64", siodb::COLUMN_DATA_TYPE_INT64, true},
    };

    instance->getDatabase("SYS")->createUserTable("TEST_EXPRESSION_EMPTY",
            dbengine::TableType::kDisk, table1Columns, dbengine::User::kSuperUserId, {});

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT 12 + 100 as TEST FROM SYS.TEST_EXPRESSION_EMPTY");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT32);

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

/**
 * SELECT * FROM NULL_TEST_TABLE_1 WHERE T IS NULL 
 */
TEST(Query, SelectWithWhereIsNull)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I", siodb::COLUMN_DATA_TYPE_INT8, true},
            {"T", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };

    instance->getDatabase("SYS")->createUserTable("NULL_TEST_TABLE_1", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    {
        const std::string statement("INSERT INTO SYS.NULL_TEST_TABLE_1 values (1, NULL)");

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
        ASSERT_EQ(response.affected_row_count(), 1U);
    }

    {
        const std::string statement("SELECT * FROM NULL_TEST_TABLE_1 WHERE T IS NULL");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 3);  // + TRID
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT8);
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_TEXT);

        ASSERT_FALSE(response.column_description(0).is_null());
        ASSERT_FALSE(response.column_description(1).is_null());
        ASSERT_TRUE(response.column_description(2).is_null());

        // Table order
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        EXPECT_EQ(response.column_description(1).name(), "I");
        EXPECT_EQ(response.column_description(2).name(), "T");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0);
        siodb::utils::Bitmask nullBitmask(response.column_description_size(), false);
        ASSERT_TRUE(codedInput.ReadRaw(nullBitmask.getData(), nullBitmask.getByteSize()));

        ASSERT_FALSE(nullBitmask.getBit(0));
        ASSERT_FALSE(nullBitmask.getBit(1));
        ASSERT_TRUE(nullBitmask.getBit(2));

        std::uint64_t trid;
        ASSERT_TRUE(codedInput.ReadVarint64(&trid));
        ASSERT_EQ(trid, 1U);
        std::uint8_t int8Value = 0;
        ASSERT_TRUE(codedInput.ReadRaw(&int8Value, 1));
        ASSERT_EQ(int8Value, std::uint8_t(1));

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength == 0);
    }
}

/**
 * SELECT * FROM NULL_TEST_TABLE_2 WHERE T = NULL
 */
TEST(Query, SelectWithWhereEqualNull)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I", siodb::COLUMN_DATA_TYPE_INT8, true},
            {"T", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };

    instance->getDatabase("SYS")->createUserTable("NULL_TEST_TABLE_2", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    {
        const std::string statement("INSERT INTO SYS.NULL_TEST_TABLE_2 values (1, NULL)");

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
        ASSERT_EQ(response.affected_row_count(), 1U);
    }

    {
        const std::string statement("SELECT * FROM NULL_TEST_TABLE_2 WHERE T = NULL");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 3);  // + TRID
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT8);
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_TEXT);

        ASSERT_FALSE(response.column_description(0).is_null());
        ASSERT_FALSE(response.column_description(1).is_null());
        ASSERT_TRUE(response.column_description(2).is_null());

        // Table order
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        EXPECT_EQ(response.column_description(1).name(), "I");
        EXPECT_EQ(response.column_description(2).name(), "T");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength == 0);
    }
}

TEST(Query, SelectWithLimit)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_1", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_LIMIT_1 VALUES (0), (1), (2), (3), (4), (5), (6), (7), "
              "(8), (9)";

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT A FROM SYS.SELECT_WITH_LIMIT_1 LIMIT 5");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT32);

        EXPECT_EQ(response.column_description(0).name(), "A");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);
        std::uint64_t rowLength = 0;
        for (auto i = 0; i < 5; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0);

            std::int32_t a = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(reinterpret_cast<std::uint32_t*>(&a)));
            ASSERT_TRUE(a == i);
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

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_2", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_LIMIT_2 VALUES (0), (1), (2), (3), (4), (5), (6), (7), "
              "(8), (9)";

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT A FROM SYS.SELECT_WITH_LIMIT_2 LIMIT 0");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT32);

        EXPECT_EQ(response.column_description(0).name(), "A");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

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

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_3", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_LIMIT_2 VALUES (0), (1), (2), (3), (4), (5), (6), (7), "
              "(8), (9)";

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT A FROM SYS.SELECT_WITH_LIMIT_3 LIMIT -1");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
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

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_AND_OFFSET_1",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_LIMIT_AND_OFFSET_1 VALUES (0), (1), (2), (3), (4), (5), "
              "(6), (7), "
              "(8), (9)";

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT A FROM SYS.SELECT_WITH_LIMIT_AND_OFFSET_1 LIMIT 5 OFFSET 5");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT32);

        EXPECT_EQ(response.column_description(0).name(), "A");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);
        std::uint64_t rowLength = 0;
        for (auto i = 5; i < 10; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0);

            std::int32_t a = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(reinterpret_cast<std::uint32_t*>(&a)));
            ASSERT_TRUE(a == i);
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

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_AND_OFFSET_2",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_LIMIT_AND_OFFSET_2 VALUES (0), (1), (2), (3), (4), (5), "
              "(6), (7), "
              "(8), (9)";

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT A FROM SYS.SELECT_WITH_LIMIT_AND_OFFSET_2 LIMIT 5 OFFSET 10");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT32);

        EXPECT_EQ(response.column_description(0).name(), "A");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);

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

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_LIMIT_AND_OFFSET_3",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_LIMIT_AND_OFFSET_3 VALUES (0), (1), (2), (3), (4), (5), "
              "(6), (7), "
              "(8), (9)";

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT A FROM SYS.SELECT_WITH_LIMIT_AND_OFFSET_3 LIMIT 10 OFFSET -1");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
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

    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
    };

    instance->getDatabase("SYS")->createUserTable("SELECT_WITH_WHERE_LIMIT_AND_OFFSET_1",
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    /// ----------- INSERT -----------
    {
        std::stringstream ss;
        ss << "INSERT INTO SYS.SELECT_WITH_WHERE_LIMIT_AND_OFFSET_1 VALUES (0), (1), (2), (3), "
              "(4), (5), "
              "(6), (7), "
              "(8), (9)";

        const std::string statement(ss.str());

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
        ASSERT_EQ(response.affected_row_count(), 10U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement(
                "SELECT A FROM SYS.SELECT_WITH_WHERE_LIMIT_AND_OFFSET_1 WHERE A > 3 LIMIT 5 "
                "OFFSET 5");
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
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_INT32);

        EXPECT_EQ(response.column_description(0).name(), "A");

        google::protobuf::io::CodedInputStream codedInput(&inputStream);
        std::uint64_t rowLength = 0;
        for (auto i = 9; i < 10; ++i) {
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength > 0);

            std::int32_t a = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(reinterpret_cast<std::uint32_t*>(&a)));
            ASSERT_TRUE(a == i);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}