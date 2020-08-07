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

// STL headers
#include <algorithm>
#include <iomanip>
#include <random>

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
    bool m_active = true;
};

struct TestUserAccessKey {
    void create(bool newKey) const;
    void drop(bool keyExists) const;
    void alter(bool keyExists) const;
    void checkExists(bool mustExist) const;

    std::string m_userName;
    std::string m_keyName;
    std::string m_keyText;
    bool m_active = true;
};

struct TestUserToken {
    void create(bool newToken) const;
    void drop(bool tokenExists) const;
    void alter(bool tokenExists) const;
    void checkExists(bool mustExist) const;

    std::string m_userName;
    std::string m_tokenName;
    std::optional<siodb::BinaryValue> m_tokenValue;
    std::optional<std::time_t> m_expirationTimestamp;
};

constexpr const char* kTestPublicKey =
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
       << "' WITH STATE = " << keyActiveStr;

    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    requestHandler->executeRequest(*dbeRequest, TestEnvironment::kTestRequestId, 0, 1);

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

    ASSERT_EQ(response.message_size(), newKey ? 0 : 1);
}

void TestUserAccessKey::drop(bool keyExists) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    std::stringstream ss;
    ss << "ALTER USER " << m_userName << " DROP ACCESS KEY " << m_keyName;
    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    requestHandler->executeRequest(*dbeRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), keyExists ? 0 : 1);

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

    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    requestHandler->executeRequest(*dbeRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), keyExists ? 0 : 1);

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

void TestUserToken::create(bool newToken) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    std::stringstream ss;

    ss << "ALTER USER " << m_userName << " ADD TOKEN " << m_tokenName;
    if (m_tokenValue) {
        auto& tokenValue = *m_tokenValue;
        ss << " x'";
        for (std::size_t i = 0, n = tokenValue.size(); i < n; ++i) {
            ss << std::hex << std::setfill('0') << std::setw(2) << (unsigned short) tokenValue[i];
        }
        ss << "'";
    }
    if (m_expirationTimestamp) {
        std::tm tm;
        gmtime_r(&*m_expirationTimestamp, &tm);
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
        ss << " WITH EXPIRATION_TIMESTAMP = '" << buffer << "'";
    }
    ss << ';';

    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    requestHandler->executeRequest(*dbeRequest, TestEnvironment::kTestRequestId, 0, 1);

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

    ASSERT_EQ(response.message_size(), newToken ? 0 : 1);
    if (newToken) {
        ASSERT_EQ(response.freetext_message_size(), m_tokenValue ? 0 : 1);
    }
}

void TestUserToken::drop(bool tokenExists) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    std::stringstream ss;
    ss << "ALTER USER " << m_userName << " DROP TOKEN " << m_tokenName;
    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    requestHandler->executeRequest(*dbeRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), tokenExists ? 0 : 1);

    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
}

void TestUserToken::alter(bool tokenExists) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    std::stringstream ss;
    ss << "ALTER USER " << m_userName << " ALTER TOKEN " << m_tokenName
       << " SET EXPIRATION_TIMESTAMP = ";
    if (m_expirationTimestamp) {
        std::tm tm;
        gmtime_r(&*m_expirationTimestamp, &tm);
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
        ss << "'" << buffer << "'";
    } else {
        ss << " NULL";
    }

    parser_ns::SqlParser parser(ss.str());
    parser.parse();

    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    siodb::iomgr_protocol::DatabaseEngineResponse response;
    siodb::protobuf::CustomProtobufInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    requestHandler->executeRequest(*dbeRequest, TestEnvironment::kTestRequestId, 0, 1);

    siodb::protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, inputStream);

    EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
    ASSERT_EQ(response.message_size(), tokenExists ? 0 : 1);

    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
}

void TestUserToken::checkExists(bool mustExist) const
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();
    std::stringstream ss;
    ss << "SELECT * FROM SYS.SYS_USER_TOKENS WHERE NAME = '" << boost::to_upper_copy(m_tokenName)
       << "' AND EXPIRATION_TIMESTAMP";
    if (m_expirationTimestamp) {
        std::tm tm;
        gmtime_r(&*m_expirationTimestamp, &tm);
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
        ss << " = '" << buffer << "'";
    } else {
        ss << " IS NULL";
    }

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

TEST(UM, CreateAccessKey)
{
    TestUser user;
    user.m_name = "CreateAccessKey_user";
    user.create(true);
    user.checkExists(true);

    auto keyIndex = 0;
    for (bool stateActive : {false, true}) {
        TestUserAccessKey key;
        key.m_userName = user.m_name;
        key.m_active = stateActive;
        key.m_keyName = "CreateAccessKey_key_" + std::to_string(keyIndex);
        key.m_keyText = kTestPublicKey;

        key.create(true);
        key.checkExists(true);
        ++keyIndex;
    }
}

TEST(UM, AddExistingAccessKey)
{
    TestUser user;
    user.m_active = true;
    user.m_name = "AddExistingAccessKey_user";
    user.create(true);
    user.checkExists(true);

    TestUserAccessKey key;
    key.m_userName = user.m_name;
    key.m_keyName = "AddExistingAccessKey_key";
    key.m_keyText = kTestPublicKey;

    key.create(true);
    key.checkExists(true);
    key.create(false);
    key.checkExists(true);
}

TEST(UM, DropExistingAccessKey)
{
    TestUser user;
    user.m_active = true;
    user.m_name = "DropExistingAccessKey_user";
    user.create(true);
    user.checkExists(true);

    TestUserAccessKey key;
    key.m_userName = user.m_name;
    key.m_keyName = "DropExistingAccessKey_key";
    key.m_keyText = kTestPublicKey;

    key.create(true);
    key.checkExists(true);
    key.drop(true);
    key.checkExists(false);
}

TEST(UM, DropNonExistingUserAccessKey)
{
    TestUserAccessKey key;
    key.m_userName = "NOT_EXIST";
    key.m_keyName = "DropNonExistingUserAccessKey_key";
    key.m_keyText = kTestPublicKey;

    key.drop(false);
    key.checkExists(false);
}

TEST(UM, AlterExistingAccessKey)
{
    TestUser user;
    user.m_name = "AlterExistingAccessKey_user";
    user.m_realName = "UserRealName";
    user.create(true);
    user.checkExists(true);

    TestUserAccessKey key;
    key.m_userName = user.m_name;
    key.m_keyName = "AlterExistingAccessKey_key";
    key.m_keyText = kTestPublicKey;
    key.m_active = true;

    key.create(true);
    key.checkExists(true);

    key.m_active = false;
    key.checkExists(false);
    key.alter(true);
    key.checkExists(true);
}

TEST(UM, AlterNonExistingAccessKey)
{
    TestUser user;
    user.m_name = "AlterNonExistingAccessKey_user";
    user.m_realName = "UserRealName";

    user.create(true);
    user.checkExists(true);

    TestUserAccessKey key;
    key.m_userName = user.m_name;
    key.m_keyName = "AlterNonExistingAccessKey_key";
    key.m_keyText = kTestPublicKey;
    key.m_active = false;

    key.checkExists(false);
    key.alter(false);
}

TEST(UM, CreateTokenWithoutValue)
{
    TestUser user;
    user.m_name = "CreateTokenWithoutValue_user";
    user.create(true);
    user.checkExists(true);

    auto tokenIndex = 0;
    for (std::time_t expirationTimestamp : {0, 1596669999}) {
        TestUserToken token;
        token.m_userName = user.m_name;
        token.m_tokenName = "CreateTokenWithoutValue_token_" + std::to_string(tokenIndex);
        if (expirationTimestamp > 0) token.m_expirationTimestamp = expirationTimestamp;

        token.create(true);
        token.checkExists(true);
        ++tokenIndex;
    }
}

TEST(UM, CreateTokenWithValue)
{
    TestUser user;
    user.m_name = "CreateTokenWithValue_user";
    user.create(true);
    user.checkExists(true);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned> dist(0, 255);
    auto tokenIndex = 0;
    for (std::time_t expirationTimestamp : {0, 1596669999}) {
        TestUserToken token;
        token.m_userName = user.m_name;
        token.m_tokenName = "CreateTokenWithValue_token_" + std::to_string(tokenIndex);
        token.m_tokenValue = siodb::BinaryValue(32);
        for (std::size_t i = 0, n = (*token.m_tokenValue).size(); i < n; ++i)
            (*token.m_tokenValue)[i] = dist(gen);
        if (expirationTimestamp > 0) token.m_expirationTimestamp = expirationTimestamp;

        token.create(true);
        token.checkExists(true);
        ++tokenIndex;
    }
}

TEST(UM, AddExistingToken)
{
    TestUser user;
    user.m_active = true;
    user.m_name = "AddExistingToken_user";
    user.create(true);
    user.checkExists(true);

    TestUserToken token;
    token.m_userName = user.m_name;
    token.m_tokenName = "AddExistingToken_token";

    token.create(true);
    token.checkExists(true);
    token.create(false);
    token.checkExists(true);
}

TEST(UM, DropExistingToken)
{
    TestUser user;
    user.m_active = true;
    user.m_name = "DropExistingToken_user";
    user.create(true);
    user.checkExists(true);

    TestUserToken token;
    token.m_userName = user.m_name;
    token.m_tokenName = "DropExistingToken_token";

    token.create(true);
    token.checkExists(true);
    token.drop(true);
    token.checkExists(false);
}

TEST(UM, DropNonExistingUserToken)
{
    TestUserToken token;
    token.m_userName = "NOT_EXIST";
    token.m_tokenName = "DropNonExistingUserAccessToken_token";

    token.drop(false);
    token.checkExists(false);
}

TEST(UM, AlterExistingToken)
{
    TestUser user;
    user.m_name = "AlterExistingToken_user";
    user.m_realName = "UserRealName";
    user.create(true);
    user.checkExists(true);

    TestUserToken token;
    token.m_userName = user.m_name;
    token.m_tokenName = "AlterExistingToken_token";

    token.create(true);
    token.checkExists(true);

    //token.m_expirationTimestamp = 1596669999;
    //token.checkExists(false);
    //token.alter(true);
    //token.checkExists(true);
}

TEST(UM, AlterNonExistingToken)
{
    TestUser user;
    user.m_name = "AlterNonExistingToken_user";
    user.m_realName = "UserRealName";

    user.create(true);
    user.checkExists(true);

    TestUserToken token;
    token.m_userName = user.m_name;
    token.m_tokenName = "AlterNonExistingToken_token";

    token.checkExists(false);
    token.alter(false);
}
