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
#include <siodb/iomgr/shared/dbengine/util/RowDecoder.h>

namespace parser_ns = dbengine::parser;
namespace util_ns = dbengine::util;

TEST(Query, DescribeTable_SysTables)
{
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);

    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();

    const std::string statement("DESCRIBE TABLE SYS.SYS_TABLES");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto request = factory.createSqlRequest();

    requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
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
    EXPECT_EQ(response.column_description(1).name(), "DATA_TYPE");

    siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

    std::uint64_t rowLength = 0;
    std::vector<std::uint8_t> rowData;
    auto& systemDatabase = instance->getSystemDatabase();
    auto sysTablesTable = systemDatabase.findTableChecked("SYS_TABLES");
    for (std::size_t i = 0, n = sysTablesTable->getColumnCount(); i < n; ++i) {
        rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_GT(rowLength, 0);
        ASSERT_LT(rowLength, 2048U);
        if (rowData.size() < rowLength) rowData.resize(rowLength);
        ASSERT_TRUE(codedInput.ReadRaw(rowData.data(), rowLength));
    }

    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    EXPECT_EQ(rowLength, 0U);
}

TEST(Query, DescribeTable_UserTable)
{
    // Create table
    std::vector<dbengine::SimpleColumnSpecification> tableColumns;
    for (int i = 1; i <= 100; ++i) {
        tableColumns.emplace_back("C" + std::to_string(i), siodb::COLUMN_DATA_TYPE_INT32, true);
    }

    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    constexpr const char* kTableName = "DESCRIBE_TABLE_1";
    instance->findDatabase("SYS")->createUserTable(
            kTableName, dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();

    const auto statement = std::string("DESCRIBE TABLE ") + kTableName;
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto request = factory.createSqlRequest();

    requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
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
    EXPECT_EQ(response.column_description(1).name(), "DATA_TYPE");

    siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

    std::uint64_t rowLength = 0;
    auto& systemDatabase = instance->getSystemDatabase();
    auto sysTablesTable = systemDatabase.findTableChecked(kTableName);
    std::vector<std::vector<std::uint8_t>> rows;
    const auto columnCount = sysTablesTable->getColumnCount();
    for (std::size_t i = 0; i < columnCount; ++i) {
        rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_GT(rowLength, 0);
        ASSERT_LT(rowLength, 2048U);
        std::vector<std::uint8_t> rowData(rowLength);
        ASSERT_TRUE(codedInput.ReadRaw(rowData.data(), rowLength));
        rows.push_back(std::move(rowData));
    }

    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    EXPECT_EQ(rowLength, 0U);

    const siodb::ColumnDataType kNameColumnDataType = siodb::COLUMN_DATA_TYPE_TEXT;
    for (std::size_t i = 0; i < columnCount; ++i) {
        const auto& rowData = rows.at(i);
        const auto decoded =
                util_ns::decodeRow(rowData.data(), rowData.size(), 2, 1, &kNameColumnDataType);
        ASSERT_EQ(decoded.size(), 1U);
        const auto expectedColumnName = i > 0 ? "C" + std::to_string(i) : "TRID";
        const auto& actualColumnName = decoded.at(0);
        ASSERT_TRUE(actualColumnName.isString());
        ASSERT_EQ(actualColumnName.getString(), expectedColumnName);
    }
}
