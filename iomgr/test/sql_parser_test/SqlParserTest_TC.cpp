// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "TestContext.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;

TEST(TC, BeginSimpleTransaction)
{
    // Parse statement and prepare request
    const std::string statement("BEGIN TRANSACTION");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kBeginTransaction);

    // Check request
    const auto& request = dynamic_cast<const requests::BeginTransactionRequest&>(*dbeRequest);
    ASSERT_EQ(request.m_type, requests::TransactionType::kDeferred);
    ASSERT_TRUE(request.m_transaction.empty());
}

TEST(TC, BeginDefaultTransaction)
{
    // Parse statement and prepare request
    const std::string statement("BEGIN TRANSACTION tx1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kBeginTransaction);

    // Check request
    const auto& request = dynamic_cast<const requests::BeginTransactionRequest&>(*dbeRequest);
    ASSERT_EQ(request.m_type, requests::TransactionType::kDeferred);
    ASSERT_EQ(request.m_transaction, "TX1");
}

TEST(TC, BeginDeferredTransaction)
{
    // Parse statement and prepare request
    const std::string statement("BEGIN DEFERRED TRANSACTION tx1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kBeginTransaction);

    // Check request
    const auto& request = dynamic_cast<const requests::BeginTransactionRequest&>(*dbeRequest);
    ASSERT_EQ(request.m_type, requests::TransactionType::kDeferred);
    ASSERT_EQ(request.m_transaction, "TX1");
}

TEST(TC, BeginImmediateTransaction)
{
    // Parse statement and prepare request
    const std::string statement("BEGIN IMMEDIATE TRANSACTION tx1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kBeginTransaction);

    // Check request
    const auto& request = dynamic_cast<const requests::BeginTransactionRequest&>(*dbeRequest);
    ASSERT_EQ(request.m_type, requests::TransactionType::kImmediate);
    ASSERT_EQ(request.m_transaction, "TX1");
}

TEST(TC, BeginExclusiveTransaction)
{
    // Parse statement and prepare request
    const std::string statement("BEGIN EXCLUSIVE TRANSACTION tx1");
    parser_ns::SqlParser parser(statement);
    parser.parse();

    parser_ns::DBEngineSqlRequestFactory factory(parser);
    const auto dbeRequest = factory.createSqlRequest();

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kBeginTransaction);

    // Check request
    const auto& request = dynamic_cast<const requests::BeginTransactionRequest&>(*dbeRequest);
    ASSERT_EQ(request.m_type, requests::TransactionType::kExclusive);
    ASSERT_EQ(request.m_transaction, "TX1");
}
