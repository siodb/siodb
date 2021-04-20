// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
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

TEST(RestPatch, PatchExistingRow)
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
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"C", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
            {"D", siodb::COLUMN_DATA_TYPE_BOOL, true},
            {"E", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };
    const std::string kTableName("REST_PATCH_ROW_T1");
    const auto table = database->createUserTable(std::string(kTableName),
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    // Insert data into table
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
            dbengine::Variant(15.0),
            dbengine::Variant(false),
            dbengine::Variant(),
    };
    std::vector<std::uint64_t> trids;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);
    requestMsg.set_object_id(1);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kSingleRowJson = R"json(
            [ { "a": -2, "b": "hello world!!!", "c": 33.0, "d": true, "e": "buzzzz" } ]
        )json";
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg, &in);

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
    const auto& jTrids = j["trids"];
    ASSERT_TRUE(jTrids.is_array());
    ASSERT_EQ(jTrids.size(), 1U);
    const auto& jTrid = jTrids.at(0);
    ASSERT_TRUE(jTrid.is_number_unsigned());
    ASSERT_EQ(static_cast<unsigned>(jTrid), 1U);
}

TEST(RestPatch, PatchNonExistingRow)
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
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"C", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
            {"D", siodb::COLUMN_DATA_TYPE_BOOL, true},
            {"E", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };
    const std::string kTableName("REST_PATCH_ROW_T2");
    const auto table = database->createUserTable(std::string(kTableName),
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    // Insert data into table
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
            dbengine::Variant(15.0),
            dbengine::Variant(false),
            dbengine::Variant(),
    };
    std::vector<std::uint64_t> trids;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);
    requestMsg.set_object_id(1001);  // non-existing TRID

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kSingleRowJson = R"json(
            [ { "a": -2, "b": "hello world!!!", "c": 33.0, "d": true, "e": "buzzzz" } ]
        )json";
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg, &in);

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
    ASSERT_EQ(response.message_size(), 0);

    // Read JSON
    const auto jsonPayload = siodb::io::readChunkedString(inputStream);

    // Valdiate JSON
    ASSERT_FALSE(jsonPayload.empty());
    LOG_DEBUG << "Response payload: " << jsonPayload;
    const auto j = nlohmann::json::parse(jsonPayload);
    ASSERT_TRUE(j.is_object());
    ASSERT_EQ(static_cast<int>(j["status"]), 404);
    ASSERT_EQ(static_cast<int>(j["affectedRowCount"]), 0);

    // Check TRIDs
    const auto& jTrids = j["trids"];
    ASSERT_TRUE(jTrids.is_array());
    ASSERT_EQ(jTrids.size(), 0U);
}

TEST(RestPatch, PatchExstingRowWithInvalidData)
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
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"C", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
            {"D", siodb::COLUMN_DATA_TYPE_BOOL, true},
            {"E", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };
    const std::string kTableName("REST_PATCH_ROW_T3");
    const auto table = database->createUserTable(std::string(kTableName),
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    // Insert data into table
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
            dbengine::Variant(15.0),
            dbengine::Variant(false),
            dbengine::Variant(),
    };
    std::vector<std::uint64_t> trids;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);
    requestMsg.set_object_id(1);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        // Introduce invalid data: "B" can't be NULL
        constexpr const char* kSingleRowJson = R"json(
            [ { "a": -2, "b": null, "c": 33.0, "d": true, "e": null } ]
        )json";
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg, &in);

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
    ASSERT_EQ(response.message_size(), 1);
}

TEST(RestPatch, PatchExistingRowNonExistingColumn)
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
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"C", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
            {"D", siodb::COLUMN_DATA_TYPE_BOOL, true},
            {"E", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };
    const std::string kTableName("REST_PATCH_ROW_T4");
    const auto table = database->createUserTable(std::string(kTableName),
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    // Insert data into table
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
            dbengine::Variant(15.0),
            dbengine::Variant(false),
            dbengine::Variant(),
    };
    std::vector<std::uint64_t> trids;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);
    requestMsg.set_object_id(1);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        // Column "Z" doesn't exist in the test table
        constexpr const char* kSingleRowJson = R"json(
            [ { "a": 3, "z": "hello", "b": "world" } ]
        )json";
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg, &in);

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
    ASSERT_EQ(response.message_size(), 1);
}

TEST(RestPatch, PatchTrid)
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
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
            {"C", siodb::COLUMN_DATA_TYPE_DOUBLE, true},
            {"D", siodb::COLUMN_DATA_TYPE_BOOL, true},
            {"E", siodb::COLUMN_DATA_TYPE_TEXT, false},
    };
    const std::string kTableName("REST_PATCH_ROW_T5");
    const auto table = database->createUserTable(std::string(kTableName),
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    // Insert data into table
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
            dbengine::Variant(15.0),
            dbengine::Variant(false),
            dbengine::Variant(),
    };
    std::vector<std::uint64_t> trids;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);
    requestMsg.set_object_id(1);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        // Attempt to update TRID
        constexpr const char* kSingleRowJson = R"json(
            [ { "a": 3, "trid": 10, "b": "world" } ]
        )json";
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg, &in);

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
    ASSERT_EQ(response.message_size(), 1);
}
