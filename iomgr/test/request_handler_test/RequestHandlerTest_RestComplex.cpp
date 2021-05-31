// Copyright (C) 2021 Siodb GmbH. All rights reserved.
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

namespace {

void PostRow(dbengine::RequestHandler& requestHandler, const std::string& tableObjectName,
        std::uint64_t expectedTrid);

void PatchRow(dbengine::RequestHandler& requestHandler, const std::string& tableObjectName,
        std::uint64_t trid);

}  // anonymous namespace

TEST(RestComplex, PostAndUpdateMultipleTimes)
{
    // Create request handler
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    requestHandler->suppressSuperUserRights();

    // Find database
    const std::string kDatabaseName("SYS");
    const auto database = instance->findDatabaseChecked(kDatabaseName);

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"C1", siodb::COLUMN_DATA_TYPE_INT64},
            {"C2", siodb::COLUMN_DATA_TYPE_INT64},
            {"C3", siodb::COLUMN_DATA_TYPE_INT64},
            {"C4", siodb::COLUMN_DATA_TYPE_INT32},
            {"C5", siodb::COLUMN_DATA_TYPE_TEXT},
            {"C6", siodb::COLUMN_DATA_TYPE_TEXT},
            {"C7", siodb::COLUMN_DATA_TYPE_INT32},
            {"C8", siodb::COLUMN_DATA_TYPE_TEXT},
    };
    const std::string kTableName("REST_COMPLEX_POST_UPDATE_MULTIPLE_T1");
    database->createUserTable(std::string(kTableName), dbengine::TableType::kDisk, tableColumns,
            dbengine::User::kSuperUserId, {});

    const auto kTableObjectName = kDatabaseName + "." + kTableName;

    // was failing with minimum number of rounds 63
    constexpr std::uint64_t kNumberOfRounds = 100;
    for (std::uint64_t id = 1; id <= kNumberOfRounds; ++id) {
        PostRow(*requestHandler, kTableObjectName, id);
        PatchRow(*requestHandler, kTableObjectName, id);
    }
}

namespace {

void PostRow(dbengine::RequestHandler& requestHandler, const std::string& tableObjectName,
        std::uint64_t expectedTrid)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::POST);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name_or_query(tableObjectName);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kSingleRowJson = R"json(
            [ { "c1": 23, "c2": 1, "c3": 23, "c4": 0, "c5": "abcdefghij", "c7": 2 } ]
        )json";
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg, &in);

    // Execute request
    requestHandler.executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

    // Receive response message
    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    // Validate response message
    ASSERT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_TRUE(response.has_affected_row_count());
    ASSERT_EQ(response.affected_row_count(), 1U);
    ASSERT_EQ(response.response_id(), 0U);
    ASSERT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 0);

    // Read JSON
    const auto jsonPayload = siodb::io::readChunkedString(inputStream);

    // Valdiate JSON
    ASSERT_FALSE(jsonPayload.empty());
    LOG_DEBUG << "Response payload: " << jsonPayload;
    const auto j = nlohmann::json::parse(jsonPayload);
    ASSERT_TRUE(j.is_object());
    ASSERT_EQ(static_cast<int>(j["status"]), 201);
    ASSERT_EQ(static_cast<int>(j["affectedRowCount"]), 1);

    // Check TRIDs
    const auto& jTrids = j["trids"];
    ASSERT_TRUE(jTrids.is_array());
    ASSERT_EQ(jTrids.size(), 1U);
    const auto& trid = jTrids.at(0);
    ASSERT_TRUE(trid.is_number_unsigned());
    ASSERT_EQ(static_cast<std::uint64_t>(trid), expectedTrid);
}

void PatchRow(dbengine::RequestHandler& requestHandler, const std::string& tableObjectName,
        std::uint64_t trid)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name_or_query(tableObjectName);
    requestMsg.set_object_id(trid);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kSingleRowJson = R"json(
            [ { "c4": 1 } ]
        )json";
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg, &in);

    // Execute request
    requestHandler.executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

    // Receive response message
    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    // Validate response message
    ASSERT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_TRUE(response.has_affected_row_count());
    ASSERT_EQ(response.affected_row_count(), 1U);
    ASSERT_EQ(response.response_id(), 0U);
    ASSERT_EQ(response.response_count(), 1U);
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
    const auto& jTrids = j["trids"];
    ASSERT_TRUE(jTrids.is_array());
    ASSERT_EQ(jTrids.size(), 1U);
    const auto& jTrid = jTrids.at(0);
    ASSERT_TRUE(jTrid.is_number_unsigned());
    ASSERT_EQ(static_cast<std::uint64_t>(jTrid), trid);
}

}  // anonymous namespace
