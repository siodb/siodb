// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/parser/DBEngineRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// STL headers
#include <iostream>

// Google Test
#include <gtest/gtest.h>

TEST(UM, CreateUser)
{
    const std::string statement = "CREATE USER user_name";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kCreateUser);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::CreateUserRequest&>(*dbeRequest);

    EXPECT_EQ(request.m_name, "USER_NAME");
    EXPECT_FALSE(request.m_realName.has_value());
    EXPECT_FALSE(request.m_description.has_value());
    EXPECT_EQ(request.m_active, true);
}

TEST(UM, CreateActiveUser)
{
    const std::string statement = "CREATE USER user_name WITH STATE=ACTIVE";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kCreateUser);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::CreateUserRequest&>(*dbeRequest);

    EXPECT_EQ(request.m_name, "USER_NAME");
    EXPECT_FALSE(request.m_realName.has_value());
    EXPECT_FALSE(request.m_description.has_value());
    EXPECT_EQ(request.m_active, true);
}

TEST(UM, CreateInactiveUser)
{
    const std::string statement = "CREATE USER user_name WITH STATE=INACTIVE";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kCreateUser);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::CreateUserRequest&>(*dbeRequest);

    EXPECT_EQ(request.m_name, "USER_NAME");
    EXPECT_FALSE(request.m_realName.has_value());
    EXPECT_FALSE(request.m_description.has_value());
    EXPECT_EQ(request.m_active, false);
}

TEST(UM, CreateUserWithRealNameAndDescription)
{
    const std::string statement =
            "CREATE USER user_name WITH REAL_NAME='real name', DESCRIPTION='description'";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kCreateUser);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::CreateUserRequest&>(*dbeRequest);

    EXPECT_EQ(request.m_name, "USER_NAME");
    EXPECT_TRUE(request.m_realName.has_value());
    EXPECT_EQ(request.m_realName.value(), "real name");
    EXPECT_TRUE(request.m_description.has_value());
    EXPECT_EQ(request.m_description.value(), "description");
    EXPECT_EQ(request.m_active, true);
}

TEST(UM, CreateUserWithNullRealNameAndDescription)
{
    const std::string statement = "CREATE USER user_name WITH REAL_NAME=NULL, DESCRIPTION=NULL";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kCreateUser);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::CreateUserRequest&>(*dbeRequest);

    EXPECT_EQ(request.m_name, "USER_NAME");
    EXPECT_FALSE(request.m_realName.has_value());
    EXPECT_FALSE(request.m_description.has_value());
    EXPECT_EQ(request.m_active, true);
}

TEST(UM, DropUser)
{
    const std::string statement = "DROP USER user_name";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kDropUser);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::DropUserRequest&>(*dbeRequest);

    ASSERT_EQ(request.m_name, "USER_NAME");
}

TEST(UM, AlterUserSetRealName)
{
    const std::string statement = "ALTER USER user_name SET REAL_NAME = 'new real name'";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kSetUserAttributes);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::SetUserAttributesRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    ASSERT_TRUE(request.m_params.m_realName.has_value());
    ASSERT_TRUE(request.m_params.m_realName.value().has_value());
    EXPECT_EQ(request.m_params.m_realName.value().value(), "new real name");
    EXPECT_FALSE(request.m_params.m_active.has_value());
}

TEST(UM, AlterUserSetState)
{
    const std::string statement = "ALTER USER user_name SET STATE = ACTIVE";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kSetUserAttributes);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::SetUserAttributesRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    ASSERT_TRUE(request.m_params.m_active.has_value());
    EXPECT_TRUE(request.m_params.m_active.value());
    EXPECT_FALSE(request.m_params.m_realName.has_value());
}

TEST(UM, AlterUserSetStateAndRealName)
{
    const std::string statement =
            "ALTER USER user_name SET STATE = INACTIVE, REAL_NAME = 'newRealName'";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kSetUserAttributes);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::SetUserAttributesRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_TRUE(request.m_params.m_realName.has_value());
    EXPECT_TRUE(request.m_params.m_realName.value().has_value());
    EXPECT_EQ(request.m_params.m_realName.value().value(), "newRealName");
    EXPECT_TRUE(request.m_params.m_active.has_value());
    EXPECT_FALSE(request.m_params.m_active.value());
}

TEST(UM, AlterUserAddAccessKey)
{
    const std::string statement =
            "ALTER USER user_name ADD ACCESS KEY keyName 'KeyText' WITH STATE = INACTIVE";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kAddUserAccessKey);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::AddUserAccessKeyRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_keyName, "KEYNAME");
    EXPECT_EQ(request.m_text, "KeyText");
    EXPECT_FALSE(request.m_active);
}

TEST(UM, AlterUserDropAccessKey)
{
    const std::string statement = "ALTER USER user_name DROP ACCESS KEY keyName";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kDropUserAccessKey);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::DropUserAccessKeyRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_keyName, "KEYNAME");
    EXPECT_FALSE(request.m_ifExists);
}

TEST(UM, AlterUserDropAccessKeyIfExists)
{
    const std::string statement = "ALTER USER user_name DROP ACCESS KEY IF EXISTS keyName";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kDropUserAccessKey);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::DropUserAccessKeyRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_keyName, "KEYNAME");
    EXPECT_TRUE(request.m_ifExists);
}

TEST(UM, AlterUserSetUserAccessKeyAttributes)
{
    const std::string statement =
            "ALTER USER user_name ALTER ACCESS KEY keyName SET STATE = INACTIVE";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kSetUserAccessKeyAttributes);

    const auto& request = dynamic_cast<
            const siodb::iomgr::dbengine::requests::SetUserAccessKeyAttributesRequest&>(
            *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_keyName, "KEYNAME");
    ASSERT_TRUE(request.m_params.m_active.has_value());
    EXPECT_FALSE(request.m_params.m_active.value());
}

TEST(UM, AlterUserRenameAccessKey)
{
    const std::string statement =
            "ALTER USER user_name ALTER ACCESS KEY keyName RENAME TO key_name";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kRenameUserAccessKey);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::RenameUserAccessKeyRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_keyName, "KEYNAME");
    EXPECT_EQ(request.m_newKeyName, "KEY_NAME");
    EXPECT_FALSE(request.m_ifExists);
}

TEST(UM, AlterUserRenameAccessKeyIfExists)
{
    const std::string statement =
            "ALTER USER user_name ALTER ACCESS KEY keyName RENAME IF EXISTS TO key_name";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kRenameUserAccessKey);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::RenameUserAccessKeyRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_keyName, "KEYNAME");
    EXPECT_EQ(request.m_newKeyName, "KEY_NAME");
    EXPECT_TRUE(request.m_ifExists);
}

TEST(UM, AlterUserAddToken1)
{
    const std::string statement = "ALTER USER user_name ADD TOKEN tokenName";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kAddUserToken);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::AddUserTokenRequest&>(*dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_tokenName, "TOKENNAME");
    EXPECT_FALSE(request.m_value.has_value());
    EXPECT_FALSE(request.m_expirationTimestamp.has_value());
    EXPECT_FALSE(request.m_description.has_value());
}

TEST(UM, AlterUserAddToken2)
{
    const std::string statement =
            "ALTER USER user_name ADD TOKEN tokenName WITH DESCRIPTION='my token'";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kAddUserToken);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::AddUserTokenRequest&>(*dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_tokenName, "TOKENNAME");
    EXPECT_FALSE(request.m_value.has_value());
    EXPECT_FALSE(request.m_expirationTimestamp.has_value());
    ASSERT_TRUE(request.m_description.has_value());
    EXPECT_EQ(*request.m_description, "my token");
}

TEST(UM, AlterUserAddToken3)
{
    const std::string statement =
            "ALTER USER user_name ADD TOKEN tokenName x'0123456789' WITH EXPIRATION_TIMESTAMP = "
            "'2021-01-01 12:21:25', DESCRIPTION='my token'";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kAddUserToken);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::AddUserTokenRequest&>(*dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_tokenName, "TOKENNAME");
    ASSERT_TRUE(request.m_value.has_value());
    const siodb::BinaryValue bv {0x01, 0x23, 0x45, 0x67, 0x89};
    EXPECT_EQ(*request.m_value, bv);
    ASSERT_TRUE(request.m_expirationTimestamp.has_value());
    const siodb::RawDateTime dt("2021-01-01 12:21:25");
    EXPECT_EQ(*request.m_expirationTimestamp, dt.toEpochTimestamp());
    ASSERT_TRUE(request.m_description.has_value());
    EXPECT_EQ(*request.m_description, "my token");
}

TEST(UM, AlterUserDropToken)
{
    const std::string statement = "ALTER USER user_name DROP TOKEN tokenName";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kDropUserToken);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::DropUserTokenRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_tokenName, "TOKENNAME");
    EXPECT_FALSE(request.m_ifExists);
}

TEST(UM, AlterUserDropTokenIfExists)
{
    const std::string statement = "ALTER USER user_name DROP TOKEN IF EXISTS tokenName";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kDropUserToken);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::DropUserTokenRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_tokenName, "TOKENNAME");
    EXPECT_TRUE(request.m_ifExists);
}

TEST(UM, AlterUserSetUserTokenAttributes)
{
    const std::string statement =
            "ALTER USER user_name ALTER TOKEN tokenName SET DESCRIPTION = 'the token',"
            " EXPIRATION_TIMESTAMP='2021-01-01 01:01:01'";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kSetUserTokenAttributes);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::SetUserTokenAttributesRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_tokenName, "TOKENNAME");
    ASSERT_TRUE(request.m_params.m_expirationTimestamp.has_value());
    const siodb::RawDateTime dt("2021-01-01 01:01:01");
    EXPECT_EQ(*request.m_params.m_expirationTimestamp, dt.toEpochTimestamp());
    ASSERT_TRUE(request.m_params.m_description.has_value());
    EXPECT_EQ(*request.m_params.m_description, "the token");
}

TEST(UM, AlterUserRenameToken)
{
    const std::string statement = "ALTER USER user_name ALTER TOKEN tokenName RENAME TO token_name";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kRenameUserToken);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::RenameUserTokenRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_tokenName, "TOKENNAME");
    EXPECT_EQ(request.m_newTokenName, "TOKEN_NAME");
    EXPECT_FALSE(request.m_ifExists);
}

TEST(UM, AlterUserRenameTokenIfExists)
{
    const std::string statement =
            "ALTER USER user_name ALTER TOKEN tokenName RENAME IF EXISTS TO token_name";
    siodb::iomgr::dbengine::parser::SqlParser parser(statement);
    parser.parse();

    const auto dbeRequest = siodb::iomgr::dbengine::parser::DBEngineRequestFactory::createRequest(
            parser.findStatement(0));

    ASSERT_EQ(dbeRequest->m_requestType,
            siodb::iomgr::dbengine::requests::DBEngineRequestType::kRenameUserToken);

    const auto& request =
            dynamic_cast<const siodb::iomgr::dbengine::requests::RenameUserTokenRequest&>(
                    *dbeRequest);

    EXPECT_EQ(request.m_userName, "USER_NAME");
    EXPECT_EQ(request.m_tokenName, "TOKENNAME");
    EXPECT_EQ(request.m_newTokenName, "TOKEN_NAME");
    EXPECT_TRUE(request.m_ifExists);
}
