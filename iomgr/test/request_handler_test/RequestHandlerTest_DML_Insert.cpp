// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"
#include "dbengine/parser/SqlParser.h"
#include "dbengine/parser/expr/ConstantExpression.h"

// Common project headers
#include <siodb/common/protobuf/ExtendedCodedInputStream.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/RawDateTimeIO.h>

// Boost headers
#include <boost/date_time.hpp>
#include <boost/endian/conversion.hpp>

namespace requests = dbengine::requests;
namespace parser_ns = dbengine::parser;

// INSERT INTO SYS.TEST_ITEMS values ('TEST', 123.0)
// SELECT NAME, PRICE AS PRICE_ALIAS FROM SYS.TEST_ITEMS
TEST(DML_Insert, InsertSingleRecord)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"NAME", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"PRICE", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
    };

    instance->findDatabase("SYS")->createUserTable("TEST_ITEMS", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- INSERT -----------
    const std::string statement("INSERT INTO SYS.TEST_ITEMS values ('TEST', 123.0)");

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto insertRequest =
            parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

    requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_TRUE(response.has_affected_row_count());
    ASSERT_EQ(response.affected_row_count(), 1U);

    /// ----------- SELECT -----------

    {
        const std::string statement("SELECT name, price as price_alias FROM sys.test_items");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 2);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);
        EXPECT_EQ(response.column_description(0).name(), "NAME");
        EXPECT_EQ(response.column_description(1).name(), "PRICE_ALIAS");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_EQ(rowLength, 13U);

        std::string name;
        ASSERT_TRUE(codedInput.Read(&name));
        ASSERT_EQ(name.length(), 4U);

        double priceValue = 0.0;
        ASSERT_TRUE(codedInput.Read(&priceValue));
        EXPECT_DOUBLE_EQ(priceValue, 123.0);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

// INSERT INTO SYS.TEST_ITEMS_DV_1(name) values ('TEST')
// SELECT NAME, PRICE AS PRICE_ALIAS FROM SYS.TEST_ITEMS_DV_1
TEST(DML_Insert, InsertSingleRecordWithDefaultValue1)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"NAME", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"PRICE", siodb::COLUMN_DATA_TYPE_DOUBLE, true, 123.0},
    };

    instance->findDatabase("SYS")->createUserTable("TEST_ITEMS_DV_1", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- INSERT -----------
    const std::string statement("INSERT INTO SYS.TEST_ITEMS_DV_1(name) values ('TEST')");

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto insertRequest =
            parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

    requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_TRUE(response.has_affected_row_count());
    ASSERT_EQ(response.affected_row_count(), 1U);

    /// ----------- SELECT -----------

    {
        const std::string statement("SELECT name, price as price_alias FROM sys.TEST_ITEMS_DV_1");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 2);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);
        EXPECT_EQ(response.column_description(0).name(), "NAME");
        EXPECT_EQ(response.column_description(1).name(), "PRICE_ALIAS");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_EQ(rowLength, 13U);

        std::string name;
        ASSERT_TRUE(codedInput.Read(&name));
        ASSERT_EQ(name.length(), 4U);

        double priceValue = 0.0;
        ASSERT_TRUE(codedInput.Read(&priceValue));
        EXPECT_DOUBLE_EQ(priceValue, 123.0);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

// INSERT INTO SYS.TEST_ITEMS_DV_2(name) values ('TEST')
// SELECT NAME, PRICE AS PRICE_ALIAS FROM SYS.TEST_ITEMS_DV_2
TEST(DML_Insert, InsertSingleRecordWithDefaultValue2)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"NAME", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"PRICE", siodb::COLUMN_DATA_TYPE_DOUBLE, true, 123.0},
    };

    instance->findDatabase("SYS")->createUserTable("TEST_ITEMS_DV_2", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- INSERT -----------
    const std::string statement("INSERT INTO SYS.TEST_ITEMS_DV_2 values ('TEST')");

    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto insertRequest =
            parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

    requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_TRUE(response.has_affected_row_count());
    ASSERT_EQ(response.affected_row_count(), 1U);

    /// ----------- SELECT -----------

    {
        const std::string statement("SELECT name, price as price_alias FROM sys.TEST_ITEMS_DV_2");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 2);
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);
        EXPECT_EQ(response.column_description(0).name(), "NAME");
        EXPECT_EQ(response.column_description(1).name(), "PRICE_ALIAS");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_EQ(rowLength, 13U);

        std::string name;
        ASSERT_TRUE(codedInput.Read(&name));
        ASSERT_EQ(name.length(), 4U);

        double priceValue = 0.0;
        ASSERT_TRUE(codedInput.Read(&priceValue));
        EXPECT_DOUBLE_EQ(priceValue, 123.0);

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

// INSERT INTO SYS.TEST_CUSTOMERS (LAST_NAME, FIRST_NAME) values ('TEST0', 'TEST1'), ('TEST2', 'TEST'3), ..., ('TEST8,
// 'TEST9')
// SELECT * FROM SYS.TEST_CUSTOMERS
TEST(DML_Insert, InsertMultipleRecords)
{
    constexpr std::size_t kInsertRows = 5;
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"FIRST_NAME", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"LAST_NAME", siodb::COLUMN_DATA_TYPE_TEXT, true},
    };

    instance->findDatabase("SYS")->createUserTable("TEST_CUSTOMERS", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- INSERT -----------
    {
        std::ostringstream ss;
        ss << "INSERT INTO SYS.TEST_CUSTOMERS (LAST_NAME, FIRST_NAME) values ";

        // 2 columns
        for (size_t i = 0; i < kInsertRows * 2; i += 2) {
            if (i > 0) ss << ", ";
            ss << "('TEST" << i << "', 'TEST" << (i + 1) << "')";
        }

        parser_ns::SqlParser parser(ss.str());
        parser.parse();

        const auto insertRequest =
                parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

        requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::StreamInputStream inputStream(
                TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), kInsertRows);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM TEST_CUSTOMERS");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::StreamInputStream inputStream(
                TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 3);  // + TRID
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_TEXT);

        // Table order
        EXPECT_EQ(response.column_description(0).name(), "TRID");
        EXPECT_EQ(response.column_description(1).name(), "FIRST_NAME");
        EXPECT_EQ(response.column_description(2).name(), "LAST_NAME");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (size_t i = 0; i < kInsertRows; ++i) {
            rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_EQ(rowLength, 13U);

            std::uint64_t trid = 0;
            ASSERT_TRUE(codedInput.Read(&trid));
            ASSERT_EQ(trid, i + 1);

            std::string name;
            ASSERT_TRUE(codedInput.Read(&name));
            EXPECT_EQ(name, std::string("TEST") + std::to_string((i * 2) + 1));

            ASSERT_TRUE(codedInput.Read(&name));
            EXPECT_EQ(name, std::string("TEST") + std::to_string(i * 2));
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

// 1) Insert into SYS.TEST_DIGITAL_BOOKS 32, 64 and 128 kb of data
// 2) Selects data from this table
TEST(DML_Insert, InsertDataTypesWithLength)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"DIGITAL_SIGNATURE", siodb::COLUMN_DATA_TYPE_BINARY, true},
            {"BOOK_TEXT", siodb::COLUMN_DATA_TYPE_TEXT, true},
    };

    instance->findDatabase("SYS")->createUserTable("TEST_DIGITAL_BOOKS", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- INSERT -----------

    std::vector<std::string> columns;
    columns.reserve(2);
    columns.push_back("DIGITAL_SIGNATURE");
    columns.push_back("BOOK_TEXT");

    std::vector<std::vector<requests::ConstExpressionPtr>> values;
    constexpr std::array<std::size_t, 3> kBufferSize = {32u * 1024u, 64u * 1024u, 128u * 1024u};
    for (std::size_t size : kBufferSize) {
        auto& v = values.emplace_back();

        // Binary values can hold '\0' symbols inside
        siodb::BinaryValue binData(size);
        binData[0] = 'T';
        binData[123] = 'E';
        binData[256] = 'S';
        binData[2321] = 'T';

        v.push_back(std::make_unique<requests::ConstantExpression>(std::move(binData)));

        std::string strData(size, 'a');
        strData[0] = 'T';
        strData[13] = 'E';
        strData[512] = 'S';
        strData[3293] = 'T';
        v.push_back(std::make_unique<requests::ConstantExpression>(std::move(strData)));
    }

    requests::InsertRequest insertRequest(
            "SYS", "TEST_DIGITAL_BOOKS", std::move(columns), std::move(values));
    requestHandler->executeRequest(insertRequest, TestEnvironment::kTestRequestId, 0, 1);
    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_TRUE(response.has_affected_row_count());
    ASSERT_EQ(response.affected_row_count(), kBufferSize.size());

    /// ----------- SELECT -----------
    const std::string statement("SELECT * FROM TEST_DIGITAL_BOOKS");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto selectRequest =
            parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

    requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_FALSE(response.has_affected_row_count());
    ASSERT_EQ(response.column_description_size(), 3);  // + TRID
    ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
    ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_BINARY);
    ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_TEXT);
    // Table order
    EXPECT_EQ(response.column_description(0).name(), "TRID");
    EXPECT_EQ(response.column_description(1).name(), "DIGITAL_SIGNATURE");
    EXPECT_EQ(response.column_description(2).name(), "BOOK_TEXT");

    siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

    std::uint64_t rowLength = 0;
    for (size_t i = 0; i < kBufferSize.size(); ++i) {
        rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        // rowLength must be (kBufferSize[i]*2) + (varint32 * 2) + varint64
        // (2 data length and TRID, TRID assumed to be only 1 byte for this test)
        ASSERT_GE(rowLength, (kBufferSize[i] * 2) + 3 && rowLength <= (kBufferSize[i] * 2) + 9);

        std::uint64_t trid = 0;
        ASSERT_TRUE(codedInput.Read(&trid));
        ASSERT_EQ(trid, i + 1);

        std::string digitalSignature;
        ASSERT_TRUE(codedInput.Read(&digitalSignature));
        ASSERT_EQ(digitalSignature.length(), kBufferSize[i]);
        EXPECT_EQ(digitalSignature[0], 'T');
        EXPECT_EQ(digitalSignature[123], 'E');
        EXPECT_EQ(digitalSignature[256], 'S');
        EXPECT_EQ(digitalSignature[2321], 'T');

        std::string bookText;
        ASSERT_TRUE(codedInput.Read(&bookText));
        ASSERT_EQ(bookText.length(), kBufferSize[i]);
        EXPECT_EQ(bookText[0], 'T');
        EXPECT_EQ(bookText[13], 'E');
        EXPECT_EQ(bookText[512], 'S');
        EXPECT_EQ(bookText[3293], 'T');
    }

    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    EXPECT_EQ(rowLength, 0U);
}

// 1) Creates a table with all number type columns
// 2) Insert Min/Max value for the column datatype
// 3) Read and compare these values
TEST(DML_Insert, InsertMinMaxValues)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"INT8", siodb::COLUMN_DATA_TYPE_INT8, true},
            {"UINT8", siodb::COLUMN_DATA_TYPE_UINT8, true},
            {"INT16", siodb::COLUMN_DATA_TYPE_INT16, true},
            {"UINT16", siodb::COLUMN_DATA_TYPE_UINT16, true},
            {"INT32", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"UINT32", siodb::COLUMN_DATA_TYPE_UINT32, true},
            {"INT64", siodb::COLUMN_DATA_TYPE_INT64, true},
            {"UINT64", siodb::COLUMN_DATA_TYPE_UINT64, true},
            {"FLOAT", siodb::COLUMN_DATA_TYPE_FLOAT, true},
            {"DOUBLE", siodb::COLUMN_DATA_TYPE_DOUBLE, true}};

    instance->findDatabase("SYS")->createUserTable("TEST_TABLE_MIN_MAX", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    std::vector<std::vector<requests::ConstExpressionPtr>> values;

    // Min values is a first row.
    auto v = &values.emplace_back();
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::int8_t>::min()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::uint8_t>::min()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::int16_t>::min()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::uint16_t>::min()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::int32_t>::min()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::uint32_t>::min()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::int64_t>::min()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::uint64_t>::min()));
    v->push_back(std::make_unique<requests::ConstantExpression>(std::numeric_limits<float>::min()));
    v->push_back(
            std::make_unique<requests::ConstantExpression>(std::numeric_limits<double>::min()));

    // Max values is a second row.
    v = &values.emplace_back();
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::int8_t>::max()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::uint8_t>::max()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::int16_t>::max()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::uint16_t>::max()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::int32_t>::max()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::uint32_t>::max()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::int64_t>::max()));
    v->push_back(std::make_unique<requests::ConstantExpression>(
            std::numeric_limits<std::uint64_t>::max()));
    v->push_back(std::make_unique<requests::ConstantExpression>(std::numeric_limits<float>::max()));
    v->push_back(
            std::make_unique<requests::ConstantExpression>(std::numeric_limits<double>::max()));

    requests::InsertRequest insertRequest("SYS", "TEST_TABLE_MIN_MAX", std::move(values));
    requestHandler->executeRequest(insertRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_TRUE(response.has_affected_row_count());
    ASSERT_EQ(response.affected_row_count(), 2U);

    /// ----------- SELECT -----------
    const std::string statement("SELECT * FROM TEST_TABLE_MIN_MAX");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto selectRequest =
            parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

    requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_FALSE(response.has_affected_row_count());
    ASSERT_EQ(response.column_description_size(), 11);  // + TRID
    ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);  // TRID
    ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT8);
    ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_UINT8);
    ASSERT_EQ(response.column_description(3).type(), siodb::COLUMN_DATA_TYPE_INT16);
    ASSERT_EQ(response.column_description(4).type(), siodb::COLUMN_DATA_TYPE_UINT16);
    ASSERT_EQ(response.column_description(5).type(), siodb::COLUMN_DATA_TYPE_INT32);
    ASSERT_EQ(response.column_description(6).type(), siodb::COLUMN_DATA_TYPE_UINT32);
    ASSERT_EQ(response.column_description(7).type(), siodb::COLUMN_DATA_TYPE_INT64);
    ASSERT_EQ(response.column_description(8).type(), siodb::COLUMN_DATA_TYPE_UINT64);
    ASSERT_EQ(response.column_description(9).type(), siodb::COLUMN_DATA_TYPE_FLOAT);
    ASSERT_EQ(response.column_description(10).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);

    siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

    std::uint64_t rowLength = 0;
    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    ASSERT_GT(rowLength, 0U);

    // Check min values
    std::uint64_t trid = 0;
    ASSERT_TRUE(codedInput.Read(&trid));
    ASSERT_EQ(trid, 1U);

    std::int8_t int8Value = 0;
    ASSERT_TRUE(codedInput.Read(&int8Value));
    ASSERT_EQ(int8Value, std::numeric_limits<std::int8_t>::min());

    std::uint8_t uint8Value = 0;
    ASSERT_TRUE(codedInput.Read(&uint8Value));
    ASSERT_EQ(uint8Value, std::numeric_limits<std::uint8_t>::min());

    std::int16_t int16Value = 0;
    ASSERT_TRUE(codedInput.Read(&int16Value));
    ASSERT_EQ(int16Value, std::numeric_limits<std::int16_t>::min());

    std::uint16_t uint16Value = 0;
    ASSERT_TRUE(codedInput.Read(&uint16Value));
    ASSERT_EQ(uint16Value, std::numeric_limits<std::uint16_t>::min());

    std::int32_t int32Value = 0;
    ASSERT_TRUE(codedInput.Read(&int32Value));
    ASSERT_EQ(int32Value, std::numeric_limits<std::int32_t>::min());

    std::uint32_t uint32Value = 0;
    ASSERT_TRUE(codedInput.Read(&uint32Value));
    ASSERT_EQ(uint32Value, std::numeric_limits<std::uint32_t>::min());

    std::int64_t int64Value = 0;
    ASSERT_TRUE(codedInput.Read(&int64Value));
    ASSERT_EQ(int64Value, std::numeric_limits<std::int64_t>::min());

    std::uint64_t uint64Value = 0;
    ASSERT_TRUE(codedInput.ReadVarint64(&uint64Value));
    ASSERT_EQ(uint64Value, std::numeric_limits<std::uint64_t>::min());

    float floatValue = 0;
    ASSERT_TRUE(codedInput.Read(&floatValue));
    ASSERT_EQ(floatValue, std::numeric_limits<float>::min());

    double doubleValue = 0;
    ASSERT_TRUE(codedInput.Read(&doubleValue));
    ASSERT_EQ(doubleValue, std::numeric_limits<double>::min());

    rowLength = 0;
    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    ASSERT_GT(rowLength, 0U);

    // Check max values
    ASSERT_TRUE(codedInput.Read(&trid));
    ASSERT_EQ(trid, 2U);

    ASSERT_TRUE(codedInput.Read(&int8Value));
    ASSERT_EQ(int8Value, std::numeric_limits<std::int8_t>::max());

    ASSERT_TRUE(codedInput.Read(&uint8Value));
    ASSERT_EQ(uint8Value, std::numeric_limits<std::uint8_t>::max());

    ASSERT_TRUE(codedInput.Read(&int16Value));
    ASSERT_EQ(int16Value, std::numeric_limits<std::int16_t>::max());

    ASSERT_TRUE(codedInput.Read(&uint16Value));
    ASSERT_EQ(uint16Value, std::numeric_limits<std::uint16_t>::max());

    ASSERT_TRUE(codedInput.Read(&int32Value));
    ASSERT_EQ(int32Value, std::numeric_limits<std::int32_t>::max());

    ASSERT_TRUE(codedInput.Read(&uint32Value));
    ASSERT_EQ(uint32Value, std::numeric_limits<std::uint32_t>::max());

    ASSERT_TRUE(codedInput.Read(&int64Value));
    ASSERT_EQ(int64Value, std::numeric_limits<std::int64_t>::max());

    ASSERT_TRUE(codedInput.Read(&uint64Value));
    ASSERT_EQ(uint64Value, std::numeric_limits<std::uint64_t>::max());

    ASSERT_TRUE(codedInput.Read(&floatValue));
    ASSERT_EQ(floatValue, std::numeric_limits<float>::max());

    ASSERT_TRUE(codedInput.Read(&doubleValue));
    ASSERT_EQ(doubleValue, std::numeric_limits<double>::max());

    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    EXPECT_EQ(rowLength, 0U);
}

// 1) Inserts 2 rows with dates into TEST_CONTRACTS
// 2) Selects and check added dates
TEST(DML_Insert, InsertDateTime)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    // create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"START_DATE", siodb::COLUMN_DATA_TYPE_TIMESTAMP, true},
            {"FINISH_DATE", siodb::COLUMN_DATA_TYPE_TIMESTAMP, true},
    };

    instance->findDatabase("SYS")->createUserTable("TEST_CONTRACTS", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- INSERT -----------
    std::vector<std::vector<requests::ConstExpressionPtr>> values;

    std::array<siodb::RawDateTime, 4> dateTime;
    // First row
    const auto rawTime = std::time(nullptr);
    struct tm timeInfo;
    char buffer[80] = {'\0'};
    localtime_r(&rawTime, &timeInfo);
    const auto n =
            strftime(buffer, sizeof(buffer), siodb::RawDateTime::kDefaultDateTimeFormat, &timeInfo);
    ASSERT_TRUE(n > 0);
    dateTime[0].parse(buffer, siodb::RawDateTime::kDefaultDateTimeFormat);
    auto v = &values.emplace_back();
    v->push_back(std::make_unique<requests::ConstantExpression>(dateTime[0]));

    dateTime[1].m_datePart.m_hasTimePart = true;
    dateTime[1].m_datePart.m_dayOfWeek = boost::gregorian::date(2019, 12, 22).day_of_week();
    dateTime[1].m_datePart.m_dayOfMonth = 21;
    dateTime[1].m_datePart.m_month = 11;
    dateTime[1].m_datePart.m_year = 2019;
    dateTime[1].m_timePart.m_nanos = 0;
    dateTime[1].m_timePart.m_seconds = 59;
    dateTime[1].m_timePart.m_minutes = 12;
    dateTime[1].m_timePart.m_hours = 12;
    dateTime[1].m_timePart.m_reserved1 = 0;
    dateTime[1].m_timePart.m_reserved2 = 0;
    v->push_back(std::make_unique<requests::ConstantExpression>(dateTime[1]));

    // Second row without time values
    dateTime[2].m_datePart.m_hasTimePart = false;
    dateTime[2].m_datePart.m_dayOfWeek = boost::gregorian::date(2017, 2, 13).day_of_week();
    dateTime[2].m_datePart.m_dayOfMonth = 12;
    dateTime[2].m_datePart.m_month = 1;
    dateTime[2].m_datePart.m_year = 2017;
    dateTime[2].m_timePart.m_nanos = 0;
    dateTime[2].m_timePart.m_seconds = 0;
    dateTime[2].m_timePart.m_minutes = 0;
    dateTime[2].m_timePart.m_hours = 0;
    dateTime[2].m_timePart.m_reserved1 = 0;
    dateTime[2].m_timePart.m_reserved2 = 0;
    v = &values.emplace_back();
    v->push_back(std::make_unique<requests::ConstantExpression>(dateTime[2]));

    dateTime[3].m_datePart.m_hasTimePart = false;
    dateTime[3].m_datePart.m_dayOfWeek = boost::gregorian::date(9999, 4, 3).day_of_week();
    dateTime[3].m_datePart.m_dayOfMonth = 2;
    dateTime[3].m_datePart.m_month = 3;
    dateTime[3].m_datePart.m_year = 9999;
    dateTime[3].m_timePart.m_nanos = 0;
    dateTime[3].m_timePart.m_seconds = 0;
    dateTime[3].m_timePart.m_minutes = 0;
    dateTime[3].m_timePart.m_hours = 0;
    dateTime[3].m_timePart.m_reserved1 = 0;
    dateTime[3].m_timePart.m_reserved2 = 0;
    v->push_back(std::make_unique<requests::ConstantExpression>(dateTime[3]));

    requests::InsertRequest insertRequest("SYS", "TEST_CONTRACTS", std::move(values));
    requestHandler->executeRequest(insertRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_TRUE(response.has_affected_row_count());
    ASSERT_EQ(response.affected_row_count(), 2U);

    /// ----------- SELECT -----------
    const std::string statement("SELECT * FROM TEST_CONTRACTS");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    const auto selectRequest =
            parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

    requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), 0);
    EXPECT_FALSE(response.has_affected_row_count());
    ASSERT_EQ(response.column_description_size(), 3);  // + TRID
    ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);  // TRID
    ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_TIMESTAMP);
    ASSERT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_TIMESTAMP);

    siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

    std::uint64_t rowLength = 0;
    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    ASSERT_GT(rowLength, 0U);

    std::uint64_t trid = 0;
    ASSERT_TRUE(codedInput.Read(&trid));
    ASSERT_EQ(trid, 1U);

    siodb::RawDateTime readDateTime;
    ASSERT_TRUE(siodb::protobuf::readRawDateTime(codedInput, readDateTime));
    ASSERT_EQ(readDateTime, dateTime[0]);

    ASSERT_TRUE(siodb::protobuf::readRawDateTime(codedInput, readDateTime));
    ASSERT_EQ(readDateTime, dateTime[1]);

    rowLength = 0;
    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    ASSERT_GT(rowLength, 0U);

    ASSERT_TRUE(codedInput.Read(&trid));
    ASSERT_EQ(trid, 2U);

    ASSERT_TRUE(siodb::protobuf::readRawDateTime(codedInput, readDateTime));
    ASSERT_EQ(readDateTime, dateTime[2]);

    ASSERT_TRUE(siodb::protobuf::readRawDateTime(codedInput, readDateTime));
    ASSERT_EQ(readDateTime, dateTime[3]);

    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    EXPECT_EQ(rowLength, 0U);
}

TEST(DML_Insert, InsertNullValue)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"I", siodb::COLUMN_DATA_TYPE_INT8, true},
            {"T", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };

    instance->findDatabase("SYS")->createUserTable("NULL_TEST_TABLE", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    {
        const std::string statement("INSERT INTO SYS.NULL_TEST_TABLE values (1, NULL)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto insertRequest =
                parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

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
        const std::string statement("SELECT * FROM NULL_TEST_TABLE");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

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

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_GT(rowLength, 0);
        stdext::bitmask nullBitmask(response.column_description_size(), false);
        ASSERT_TRUE(codedInput.ReadRaw(nullBitmask.data(), nullBitmask.size()));

        ASSERT_FALSE(nullBitmask.get(0));
        ASSERT_FALSE(nullBitmask.get(1));
        ASSERT_TRUE(nullBitmask.get(2));

        std::uint64_t trid = 0;
        ASSERT_TRUE(codedInput.Read(&trid));
        ASSERT_EQ(trid, 1U);

        std::uint8_t int8Value = 0;
        ASSERT_TRUE(codedInput.Read(&int8Value));
        ASSERT_EQ(int8Value, std::uint8_t(1));

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength == 0);
    }
}

TEST(DML_Insert, InsertDefaultNullValue)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"U1", siodb::COLUMN_DATA_TYPE_UINT32, true},
            {"U2", siodb::COLUMN_DATA_TYPE_UINT32, false},
    };

    instance->findDatabase("SYS")->createUserTable("TEST_DEFAULT_NULL", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    {
        // Equal to inserting of:
        // 0, NULL,
        // 1, NULL
        const std::string statement("INSERT INTO TEST_DEFAULT_NULL values (0), (1)");

        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto insertRequest =
                parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

        requestHandler->executeRequest(*insertRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_TRUE(response.has_affected_row_count());
        ASSERT_EQ(response.affected_row_count(), 2U);
    }

    {
        const std::string statement("SELECT U1, U2 FROM TEST_DEFAULT_NULL");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        const auto selectRequest =
                parser_ns::DBEngineSqlRequestFactory::createSqlRequest(parser.findStatement(0));

        requestHandler->executeRequest(*selectRequest, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 2);  // + TRID
        ASSERT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT32);
        ASSERT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_UINT32);

        ASSERT_FALSE(response.column_description(0).is_null());
        ASSERT_TRUE(response.column_description(1).is_null());

        // Table order
        EXPECT_EQ(response.column_description(0).name(), "U1");
        EXPECT_EQ(response.column_description(1).name(), "U2");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        for (std::uint32_t i = 0; i < 2; ++i) {
            rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_GT(rowLength, 0);
            stdext::bitmask nullBitmask(response.column_description_size(), false);
            ASSERT_TRUE(codedInput.ReadRaw(nullBitmask.data(), nullBitmask.size()));

            ASSERT_FALSE(nullBitmask.get(0));
            ASSERT_TRUE(nullBitmask.get(1));

            std::uint32_t uint32Value = 0;
            ASSERT_TRUE(codedInput.ReadVarint32(&uint32Value));
            ASSERT_EQ(uint32Value, i);
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength == 0);
    }
}
