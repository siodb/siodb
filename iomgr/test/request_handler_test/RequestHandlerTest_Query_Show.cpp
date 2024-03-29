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

TEST(Query, ShowDatabases)
{
    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();

    const std::string statement("SHOW DATABASES");
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
    EXPECT_EQ(response.column_description(1).name(), "UUID");

    siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

    std::uint64_t rowLength = 0;
    std::vector<std::uint8_t> rowData;
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    for (std::size_t i = 0, n = instance->getDatabaseCount(); i < n; ++i) {
        rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_GT(rowLength, 0);
        ASSERT_LT(rowLength, 100);
        if (rowData.size() < rowLength) rowData.resize(rowLength);
        ASSERT_TRUE(codedInput.ReadRaw(rowData.data(), rowLength));
    }

    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    EXPECT_EQ(rowLength, 0U);
}

TEST(Query, ShowTables)
{
    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();

    const std::string statement("SHOW TABLES");
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
    EXPECT_EQ(response.column_description(1).name(), "DESCRIPTION");

    siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

    std::uint64_t rowLength = 0;
    std::vector<std::uint8_t> rowData;
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto& systemDatabase = instance->getSystemDatabase();
    for (std::size_t i = 0, n = systemDatabase.getTableCount(); i < n; ++i) {
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
