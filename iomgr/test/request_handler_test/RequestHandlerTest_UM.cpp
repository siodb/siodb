// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"
#include "dbengine/handlers/RequestHandler.h"
#include "dbengine/parser/DBEngineRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/utils/Debug.h>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>

namespace parser_ns = dbengine::parser;
namespace requests = dbengine::requests;

namespace {
struct TestUser {
    void create(bool newUser) const;
    void drop(bool userExists) const;
    void alter(bool userExists) const;
    void checkExists(bool mustExist) const;

    std::string m_name;
    std::string m_realName;
    bool m_active;
};

struct TestUserAccessKey {
    void create(bool newKey) const;
    void drop(bool keyExists) const;
    void alter(bool keyExists) const;
    void checkExists(bool mustExist) const;

    std::string m_userName;
    std::string m_keyName;
    std::string m_keyText;
    bool m_active;
};

constexpr const char* testPublicUserKey =
        "ssh-rsa "
        "AAAAB3NzaC1yc2EAAAADAQABAAABAQDoBVv3EJHcAasNU4nYdJtdfCVeSH4+"
        "5iTQEfx4xGrc0cA4TM5VwGdxTfyUU8wREsTuDi7GsWunFEKsPGZmHH+d/"
        "NNfDitK9esnG5QqdFgYEnKvWu9wHijoQHaEIKk+A6vCJrPRwfullOMPQV+R1ItRxLJY/"
        "BSO89tOBbD1+E+GMz9K0XRm1a3hegAmPq/nJSAjdyafKVk/8CXwFHCeMAlmFiI3iJ0Na/J4Qq6Xx5DW/"
        "bHcgum8LFDHrCT+GS1opoSLvoqC6C5k5vNkefBOYg3I3yd55XWYn5aaME0R63IyIyaf2WWYaljSlK73uI/"
        "GHBG9BLyr87X9p8ce1HlV0qWl";

void TestUser::create(bool newUser) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    const std::string stateActiveStr = m_active ? "ACTIVE" : "INACTIVE";
    std::stringstream ss;
    ss << "CREATE USER " << m_name << " WITH STATE = " << stateActiveStr << ", REAL_NAME = '"
       << m_realName << '\'';

    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto createUserRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    requestHandler->executeRequest(*createUserRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    // Actual both for negative and positive answers
    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);

    if (newUser)
        ASSERT_EQ(response.message_size(), 0);
    else
        ASSERT_EQ(response.message_size(), 1);
}

void TestUser::drop(bool userExists) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    const auto str = "DROP USER " + m_name;
    parser_ns::SqlParser parser(str);
    parser.parse();

    const auto dropUserRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    requestHandler->executeRequest(*dropUserRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    if (userExists)
        ASSERT_EQ(response.message_size(), 0);
    else
        ASSERT_EQ(response.message_size(), 1);

    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
}

void TestUser::alter(bool userExists) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    const std::string stateActiveStr = m_active ? "ACTIVE" : "INACTIVE";
    std::stringstream ss;
    ss << "ALTER USER " << m_name << " SET STATE = " << stateActiveStr << ", REAL_NAME = '"
       << m_realName << '\'';

    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto alterUserRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    requestHandler->executeRequest(*alterUserRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    if (userExists)
        ASSERT_EQ(response.message_size(), 0);
    else
        ASSERT_EQ(response.message_size(), 1);

    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
}

void TestUser::checkExists(bool mustExist) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    std::string sql;
    {
        std::stringstream ss;
        ss << "SELECT * FROM SYS.SYS_USERS WHERE name = '" << boost::to_upper_copy(m_name)
           << "' AND REAL_NAME = '" << m_realName << "' AND STATE = " << (m_active ? '1' : '0');
        sql = ss.str();
    }

    parser_ns::SqlParser parser(sql);
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    requestHandler->executeRequest(*dbeRequest, TestEnvironment::kTestRequestId, 0, 1);

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
    if (mustExist) {
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength > 0);
        ASSERT_TRUE(rowLength < 1000);
        siodb::BinaryValue rowData(rowLength);
        ASSERT_TRUE(codedInput.ReadRaw(rowData.data(), rowLength));
    }
    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    ASSERT_EQ(rowLength, 0U);
}

void TestUserAccessKey::create(bool newKey) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    const std::string keyActiveStr = m_active ? "ACTIVE" : "INACTIVE";
    std::stringstream ss;

    ss << "ALTER USER " << m_userName << " ADD ACCESS KEY " << m_keyName << " '" << m_keyText
       << "' STATE = " << keyActiveStr;

    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto createUserRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    requestHandler->executeRequest(*createUserRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    // Actual both for negative and positive answers
    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);

    if (newKey)
        ASSERT_EQ(response.message_size(), 0);
    else
        ASSERT_EQ(response.message_size(), 1);
}

void TestUserAccessKey::drop(bool keyExists) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    std::stringstream ss;
    ss << "ALTER USER " << m_userName << " DROP ACCESS KEY " << m_keyName;
    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto dropUserRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    requestHandler->executeRequest(*dropUserRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    if (keyExists)
        ASSERT_EQ(response.message_size(), 0);
    else
        ASSERT_EQ(response.message_size(), 1);

    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
}

void TestUserAccessKey::alter(bool keyExists) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    std::stringstream ss;
    const std::string keyActiveStr = m_active ? "ACTIVE" : "INACTIVE";
    ss << "ALTER USER " << m_userName << " ALTER ACCESS KEY " << m_keyName
       << " SET STATE = " << keyActiveStr;

    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto alterUserRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    requestHandler->executeRequest(*alterUserRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    if (keyExists)
        ASSERT_EQ(response.message_size(), 0);
    else
        ASSERT_EQ(response.message_size(), 1);

    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
}

void TestUserAccessKey::checkExists(bool mustExist) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    std::stringstream ss;
    ss << "SELECT * FROM SYS.SYS_USER_ACCESS_KEYS WHERE NAME = '" << boost::to_upper_copy(m_keyName)
       << "' AND TEXT = '" << m_keyText << "' AND STATE = " << (m_active ? '1' : '0');

    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSelect);

    requestHandler->executeRequest(*dbeRequest, TestEnvironment::kTestRequestId, 0, 1);

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
    if (mustExist) {
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_TRUE(rowLength < 1000);
        ASSERT_TRUE(rowLength > 0);
        siodb::BinaryValue rowData(rowLength);
        ASSERT_TRUE(codedInput.ReadRaw(rowData.data(), rowLength));
    }
    ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
    ASSERT_EQ(rowLength, 0U);
}

}  // anonymous namespace

TEST(UM, CreateUser)
{
    auto userIndex = 0;
    for (const bool stateActive : {false, true}) {
        TestUser user;
        const auto userIndexStr = std::to_string(userIndex);
        user.m_active = stateActive;
        user.m_name = "User_" + userIndexStr;
        user.m_realName = "FirstName_" + userIndexStr + " LastName_" + userIndexStr;
        user.create(true);
        user.checkExists(true);
        ++userIndex;
    }
}

TEST(UM, CreateExistingUser)
{
    TestUser user;
    user.m_active = true;
    user.m_name = "CreateExistingUser_user";
    user.m_realName = "UserRealName";

    user.create(true);
    user.checkExists(true);

    user.create(false);
    user.checkExists(true);
}

TEST(UM, DropExistingUser)
{
    TestUser user;
    user.m_name = "DropExistingUser_user";
    user.create(true);
    user.checkExists(true);

    user.drop(true);
    user.checkExists(false);
}

TEST(UM, DropNonExistingUser)
{
    TestUser user;
    user.m_name = "DropNonExistingUser_user";
    user.checkExists(false);
    user.drop(false);
}

TEST(UM, AlterExistingUser)
{
    TestUser user;
    user.m_name = "AlterUserStateAndRealName_user";
    user.m_realName = "UserRealName";
    user.m_active = true;
    user.create(true);

    user.m_realName = "UserRealName_changed";
    user.m_active = true;
    user.alter(true);
    user.checkExists(true);
}

TEST(UM, AlterNonExistingUser)
{
    TestUser user;
    user.m_name = "AlterNonExistingUser_user";

    user.alter(false);
    user.checkExists(false);
}

TEST(UM, AlterUserCreateAccessKey)
{
    TestUser user;
    user.m_name = "AlterUserCreateAccessKey_user";
    user.create(true);
    user.checkExists(true);

    auto keyIndex = 0;
    for (bool stateActive : {false, true}) {
        TestUserAccessKey key;
        key.m_userName = user.m_name;
        key.m_active = stateActive;
        key.m_keyName = "AlterUserCreateAccessKey_key_" + std::to_string(keyIndex);
        key.m_keyText = testPublicUserKey;

        key.create(true);
        key.checkExists(true);
        ++keyIndex;
    }
}

TEST(UM, AlterUserAddExistingKey)
{
    TestUser user;
    user.m_active = true;
    user.m_name = "AlterUserAddExistingKey_user";
    user.create(true);
    user.checkExists(true);

    TestUserAccessKey key;
    key.m_userName = user.m_name;
    key.m_keyName = "AlterUserAddExistingKey_key";
    key.m_keyText = testPublicUserKey;

    key.create(true);
    key.checkExists(true);
    key.create(false);
    key.checkExists(true);
}

TEST(UM, AlterUserDropExistingAccessKey)
{
    TestUser user;
    user.m_active = true;
    user.m_name = "AlterUserDropExistingAccessKey_user";
    user.create(true);
    user.checkExists(true);

    TestUserAccessKey key;
    key.m_userName = user.m_name;
    key.m_keyName = "AlterUserDropExistingAccessKey_key";
    key.m_keyText = testPublicUserKey;

    key.create(true);
    key.checkExists(true);
    key.drop(true);
    key.checkExists(false);
}

TEST(UM, AlterUserDropNonExistingUserAccessKey)
{
    TestUserAccessKey key;
    key.m_userName = "NOT_EXIST";
    key.m_keyName = "AlterUserDropNonExistingUserAccessKey_key";
    key.m_keyText = testPublicUserKey;

    key.drop(false);
    key.checkExists(false);
}

TEST(UM, AlterUserAlterExistingAccessKey)
{
    TestUser user;
    user.m_name = "AlterUserAlterExistingAccessKey_user";
    user.m_realName = "UserRealName";
    user.create(true);
    user.checkExists(true);

    TestUserAccessKey key;
    key.m_userName = user.m_name;
    key.m_keyName = "AlterUserAlterExistingAccessKey_key";
    key.m_keyText = testPublicUserKey;
    key.m_active = true;

    key.create(true);
    key.checkExists(true);

    key.m_active = false;
    key.checkExists(false);
    key.alter(true);
    key.checkExists(true);
}

TEST(UM, AlterUserAlterNonExistingAccessKey)
{
    TestUser user;
    user.m_name = "AlterUserAlterNonExistingAccessKey_user";
    user.m_realName = "UserRealName";

    user.create(true);
    user.checkExists(true);

    TestUserAccessKey key;
    key.m_userName = user.m_name;
    key.m_keyName = "AlterUserAlterNonExistingAccessKey_user";
    key.m_keyText = testPublicUserKey;
    key.m_active = false;

    key.checkExists(false);
    key.alter(false);
}
