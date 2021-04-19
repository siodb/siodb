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

// JSON library
#include <nlohmann/json.hpp>

namespace parser_ns = dbengine::parser;

TEST(RestDelete, DeleteExistingRow)
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
    const std::string kTableName("REST_DELETE_ROW_1");
    const auto table = database->createUserTable(std::string(kTableName),
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));

    // Insert data into table
    std::vector<dbengine::Variant> values {dbengine::Variant(1), dbengine::Variant("hello")};
    std::vector<std::uint64_t> trids;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::DELETE);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);
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
    const auto j = nlohmann::json::parse(jsonPayload);

    const auto& jStatus = j["status"];
    ASSERT_TRUE(jStatus.is_number());
    ASSERT_EQ(static_cast<int>(jStatus), 200);

    const auto& jAffectedRowCount = j["affectedRowCount"];
    ASSERT_TRUE(jAffectedRowCount.is_number());
    ASSERT_EQ(static_cast<int>(jAffectedRowCount), 1);

    const auto& jTrids = j["trids"];
    ASSERT_TRUE(jTrids.is_array());
    ASSERT_EQ(jTrids.size(), 1U);
    ASSERT_EQ(static_cast<std::uint64_t>(jTrids[0]), 1U);
}

TEST(RestDelete, DeleteNonExistingRow)
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
    const std::string kTableName("REST_DELETE_ROW_2");
    const auto table = database->createUserTable(std::string(kTableName),
            dbengine::TableType::kDisk, tableColumns, dbengine::User::kSuperUserId, {});
    const dbengine::TransactionParameters tp(dbengine::User::kSuperUserId,
            database->generateNextTransactionId(), std::time(nullptr));

    // Insert data into table
    std::vector<dbengine::Variant> values {dbengine::Variant(1), dbengine::Variant("hello")};
    std::vector<std::uint64_t> trids;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());
    values[0] = values[0].getInt32() + 1;
    trids.push_back(table->insertRow(stdext::copy(values), tp).m_mcr->getTableRowId());

    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::DELETE);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name(kDatabaseName + "." + kTableName);
    requestMsg.set_object_id(1001);  // non-existing TRID

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
    const auto j = nlohmann::json::parse(jsonPayload);

    const auto& jStatus = j["status"];
    ASSERT_TRUE(jStatus.is_number());
    ASSERT_EQ(static_cast<int>(jStatus), 404);

    const auto& jAffectedRowCount = j["affectedRowCount"];
    ASSERT_TRUE(jAffectedRowCount.is_number());
    ASSERT_EQ(static_cast<int>(jAffectedRowCount), 0);

    const auto& jTrids = j["trids"];
    ASSERT_TRUE(jTrids.is_array());
    ASSERT_EQ(jTrids.size(), 0U);
}
