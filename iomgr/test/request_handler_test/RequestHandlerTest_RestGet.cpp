// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"
#include "dbengine/handlers/RequestHandler.h"
#include "dbengine/parser/DBEngineRestRequestFactory.h"

// Common project headers
#include <siodb/common/io/InputStreamUtils.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/StreamInputStream.h>
#include <siodb/common/stl_ext/utility_ext.h>

// JSON library
#include <nlohmann/json.hpp>

namespace parser_ns = dbengine::parser;

TEST(RestGet, GetDatabases)
{
    // Create request handler
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    requestHandler->suppressSuperUserRights();

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::DATABASE);

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

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
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 0);

    // Read JSON
    const auto jsonPayload = siodb::io::readChunkedString(inputStream);

    // Valdiate JSON
    ASSERT_FALSE(jsonPayload.empty());
    const auto j = nlohmann::json::parse(jsonPayload);

    const auto& jStatus = j["status"];
    ASSERT_TRUE(jStatus.is_number());
    ASSERT_EQ(static_cast<int>(jStatus), 200);

    const auto& jRows = j["rows"];
    ASSERT_TRUE(jRows.is_array());
    // Exclude system database
    ASSERT_EQ(jRows.size(), instance->getDatbaseCount() - 1);
}

TEST(RestGet, GetTables)
{
    // Create request handler
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    requestHandler->suppressSuperUserRights();

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_INT32, true},
    };
    instance->findDatabase("SYS")->createUserTable("REST_GET_TABLES_1", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::TABLE);
    requestMsg.set_object_name("sys");

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

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
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 0);

    // Read JSON
    const auto jsonPayload = siodb::io::readChunkedString(inputStream);

    // Valdiate JSON
    ASSERT_FALSE(jsonPayload.empty());
    const auto j = nlohmann::json::parse(jsonPayload);

    const auto& jStatus = j["status"];
    ASSERT_TRUE(jStatus.is_number());
    ASSERT_EQ(static_cast<int>(jStatus), 200);

    const auto& jRows = j["rows"];
    ASSERT_TRUE(jRows.is_array());
    ASSERT_TRUE(jRows.size() > 0);
    ASSERT_TRUE(std::all_of(jRows.cbegin(), jRows.cend(),
            [](const auto& e) { return e.is_object() && e["name"].is_string(); }));
    ASSERT_TRUE(std::find_if(jRows.cbegin(), jRows.cend(), [](const auto& e) {
        return static_cast<std::string>(e["name"]) == "REST_GET_TABLES_1";
    }) != jRows.cend());
}

TEST(RestGet, GetAllRows)
{
    // Create request handler
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    requestHandler->suppressSuperUserRights();

    // Create table
    const std::vector<dbengine::SimpleColumnSpecification> tableColumns {
            {"A", siodb::COLUMN_DATA_TYPE_INT32, true},
            {"B", siodb::COLUMN_DATA_TYPE_TEXT, true},
    };
    const auto database = instance->findDatabase("SYS");
    const auto table = database->createUserTable("REST_GET_ALL_ROWS_1", dbengine::TableType::kDisk,
            tableColumns, dbengine::User::kSuperUserId, {});

    // Insert data into table
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
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
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name("SYS.rest_GET_ALL_ROWS_1");

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

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
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 0);

    // Read JSON
    const auto jsonPayload = siodb::io::readChunkedString(inputStream);

    // Valdiate JSON
    ASSERT_FALSE(jsonPayload.empty());
    const auto j = nlohmann::json::parse(jsonPayload);

    const auto& jStatus = j["status"];
    ASSERT_TRUE(jStatus.is_number());
    ASSERT_EQ(static_cast<int>(jStatus), 200);

    const auto& jRows = j["rows"];
    ASSERT_TRUE(jRows.is_array());
    ASSERT_EQ(jRows.size(), trids.size());
    ASSERT_TRUE(std::all_of(jRows.cbegin(), jRows.cend(), [](const auto& e) {
        return e.is_object() && e["TRID"].is_number() && e["A"].is_number() && e["B"].is_string();
    }));
    for (std::size_t i = 0, n = trids.size(); i < n; ++i) {
        ASSERT_EQ(static_cast<std::uint64_t>(jRows[i]["TRID"]), trids[i]);
    }
}

TEST(RestGet, GetAllRowsFromSystemTable)
{
    // Create request handler
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    requestHandler->suppressSuperUserRights();

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name("SYS.SYS_tables");

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

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
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 1);
}

TEST(RestGet, GetSingleRowWithMatch)
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
    };
    const std::string kTableName("REST_GET_SINGLE_ROW_1");
    const auto table = database->createUserTable(std::string(kTableName),
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    // Insert data into table
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
    };
    std::vector<std::uint64_t> trids;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());

    // Create source protobuf message
    constexpr std::size_t kCheckedRowIndex = 1;
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);
    requestMsg.set_object_id(trids.at(kCheckedRowIndex));

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

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
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 0);

    // Read JSON
    const auto jsonPayload = siodb::io::readChunkedString(inputStream);

    // Valdiate JSON
    ASSERT_FALSE(jsonPayload.empty());
    const auto j = nlohmann::json::parse(jsonPayload);

    const auto& jStatus = j["status"];
    ASSERT_TRUE(jStatus.is_number());
    ASSERT_EQ(static_cast<int>(jStatus), 200);

    const auto& jRows = j["rows"];
    ASSERT_TRUE(jRows.is_array());
    ASSERT_EQ(jRows.size(), 1U);
    ASSERT_TRUE(std::all_of(jRows.cbegin(), jRows.cend(), [](const auto& e) {
        return e.is_object() && e["TRID"].is_number() && e["A"].is_number() && e["B"].is_string();
    }));
    ASSERT_EQ(static_cast<std::uint64_t>(jRows[0]["TRID"]), trids.at(kCheckedRowIndex));
}

TEST(RestGet, GetSingleRowNoMatch)
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
    };
    const std::string kTableName("REST_GET_SINGLE_ROW_2");
    const auto table = database->createUserTable(std::string(kTableName),
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});

    // Insert data into table
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));
    std::vector<dbengine::Variant> values {
            dbengine::Variant(1),
            dbengine::Variant("hello"),
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
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);
    requestMsg.set_object_id(*std::max_element(trids.cbegin(), trids.cend()) + 1);

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

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
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 0);

    // Read JSON
    const auto jsonPayload = siodb::io::readChunkedString(inputStream);

    // Valdiate JSON
    ASSERT_FALSE(jsonPayload.empty());
    const auto j = nlohmann::json::parse(jsonPayload);

    const auto& jStatus = j["status"];
    ASSERT_TRUE(jStatus.is_number());
    ASSERT_EQ(static_cast<int>(jStatus), 200);

    const auto& jRows = j["rows"];
    ASSERT_TRUE(jRows.is_array());
    ASSERT_EQ(jRows.size(), 0U);
}

TEST(RestGet, GetSingleRowFromSystemTable)
{
    // Create request handler
    const auto instance = TestEnvironment::getInstance();
    ASSERT_NE(instance, nullptr);
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    requestHandler->suppressSuperUserRights();

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::GET);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name("SYS.sys_TABLES");
    requestMsg.set_object_id(1);

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

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
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
    ASSERT_EQ(response.column_description_size(), 0);
    ASSERT_EQ(response.message_size(), 1);
}
