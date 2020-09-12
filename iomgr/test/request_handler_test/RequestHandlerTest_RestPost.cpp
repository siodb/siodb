// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"
#include "dbengine/handlers/RequestHandler.h"
#include "dbengine/parser/DBEngineRestRequestFactory.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/BufferedChunkedOutputStream.h>
#include <siodb/common/io/InputStreamUtils.h>
#include <siodb/common/io/MemoryInputStream.h>
#include <siodb/common/io/MemoryOutputStream.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/StreamInputStream.h>

// JSON library
#include <nlohmann/json.hpp>

namespace parser_ns = dbengine::parser;

TEST(RestPost, PostSingleRow)
{
    // Create request handler
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    requestHandler->suppressSuperUserRights();

    // Create database
    const std::string kDatabaseName("REST_POST_ROW_DB1");
    instance->createDatabase(std::string(kDatabaseName), "none", siodb::BinaryValue(), {},
            dbengine::User::kSuperUserId);

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"C", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
            {"D", siodb::COLUMN_DATA_TYPE_BOOL, true},
            {"E", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };
    const std::string kTableName("REST_POST_ROW_T1");
    instance->findDatabase(kDatabaseName)
            ->createUserTable(std::string(kTableName), dbengine::TableType::kDisk, tableColumns,
                    dbengine::User::kSuperUserId, {});

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::POST);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kSingleRowJson = R"json(
            [ { "a": -2, "b": "hello world!!!", "c": 33.0, "d": true, "e": null } ]
        )json";
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    const auto request = parser_ns::DBEngineRestRequestFactory::createRestRequest(requestMsg, &in);

    // Execute request
    requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

    // Receive response message
    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    // Validate response message
    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    EXPECT_TRUE(response.has_affected_row_count());
    EXPECT_EQ(response.affected_row_count(), 1U);
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 0);

    // Read JSON
    const auto jsonPayload = siodb::io::readChunkedString(inputStream);

    // Valdiate JSON
    ASSERT_FALSE(jsonPayload.empty());
    LOG_DEBUG << "Response payload: " << jsonPayload;
    const auto j = nlohmann::json::parse(jsonPayload);
    ASSERT_TRUE(j.is_object());
    ASSERT_EQ(static_cast<int>(j["status"]), 200);
    ASSERT_EQ(static_cast<int>(j["affectedRowCount"]), 1);

    // Check TRIDs
    const auto& trids = j["trids"];
    ASSERT_TRUE(trids.is_array());
    ASSERT_EQ(trids.size(), 1U);
    const auto& trid = trids.at(0);
    ASSERT_TRUE(trid.is_number_unsigned());
    ASSERT_EQ(static_cast<unsigned>(trid), 1U);
}

TEST(RestPost, PostMultipleRows)
{
    // Create request handler
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    requestHandler->suppressSuperUserRights();

    // Create database
    const std::string kDatabaseName("REST_POST_ROW_DB2");
    instance->createDatabase(std::string(kDatabaseName), "none", siodb::BinaryValue(), {},
            dbengine::User::kSuperUserId);

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"C", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
            {"D", siodb::COLUMN_DATA_TYPE_BOOL, true},
            {"E", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };
    const std::string kTableName("REST_POST_ROW_T2");
    instance->findDatabase(kDatabaseName)
            ->createUserTable(std::string(kTableName), dbengine::TableType::kDisk, tableColumns,
                    dbengine::User::kSuperUserId, {});

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::POST);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kSingleRowJson = R"json(
            [
                { "a": -2, "b": "hello world!!!", "c": 33.0, "d": true, "e": null },
                { "a": 3, "b": "hello world once again!!!", "c": 42.0, "d": false, "e": "zzz" },
                { "a": 5, "b": "hello world one more time!!!", "c": 29.0, "d": true, "e": "xyz" }
            ]
        )json";
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    const auto request = parser_ns::DBEngineRestRequestFactory::createRestRequest(requestMsg, &in);

    // Execute request
    requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

    // Receive response message
    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    // Validate response message
    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    EXPECT_TRUE(response.has_affected_row_count());
    EXPECT_EQ(response.affected_row_count(), 3U);
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 0);

    // Read JSON
    const auto jsonPayload = siodb::io::readChunkedString(inputStream);

    // Valdiate JSON
    ASSERT_FALSE(jsonPayload.empty());
    LOG_DEBUG << "Response payload: " << jsonPayload;
    const auto j = nlohmann::json::parse(jsonPayload);
    ASSERT_TRUE(j.is_object());
    ASSERT_EQ(static_cast<int>(j["status"]), 200);
    ASSERT_EQ(static_cast<int>(j["affectedRowCount"]), 3);

    // Check TRIDs
    const auto& trids = j["trids"];
    ASSERT_TRUE(trids.is_array());
    ASSERT_EQ(trids.size(), 3U);
    for (std::size_t i = 0; i < 3; ++i) {
        const auto& trid = trids.at(i);
        ASSERT_TRUE(trid.is_number_unsigned());
        ASSERT_EQ(static_cast<unsigned>(trid), i + 1);
    }
}

TEST(RestPost, PostWithIncorrectData)
{
    // Create request handler
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    requestHandler->suppressSuperUserRights();

    // Create database
    const std::string kDatabaseName("REST_POST_ROW_DB3");
    instance->createDatabase(std::string(kDatabaseName), "none", siodb::BinaryValue(), {},
            dbengine::User::kSuperUserId);

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"C", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
            {"D", siodb::COLUMN_DATA_TYPE_BOOL, true},
            {"E", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };
    const std::string kTableName("REST_POST_ROW_T3");
    instance->findDatabase(kDatabaseName)
            ->createUserTable(std::string(kTableName), dbengine::TableType::kDisk, tableColumns,
                    dbengine::User::kSuperUserId, {});

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::POST);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        // Incorrect data: A value is NULL, but declared NOT NULL.
        constexpr const char* kJson = R"json(
            [ { "a": null, "b": "hello world!!!", "c": 33.0, "d": true, "e": null } ]
        )json";
        chunkedOutput.write(kJson, ::ct_strlen(kJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    const auto request = parser_ns::DBEngineRestRequestFactory::createRestRequest(requestMsg, &in);

    // Execute request
    requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

    // Receive response message
    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    // Validate response message
    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    EXPECT_TRUE(response.has_affected_row_count());
    EXPECT_EQ(response.affected_row_count(), 0U);
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_GT(response.message_size(), 0);
}
