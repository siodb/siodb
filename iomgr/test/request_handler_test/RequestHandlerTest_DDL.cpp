// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"
#include "dbengine/DatabaseError.h"
#include "dbengine/Table.h"
#include "dbengine/handlers/RequestHandler.h"
#include "dbengine/parser/DBEngineSqlRequestFactory.h"
#include "dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/protobuf/ExtendedCodedInputStream.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/ConstantExpression.h>

// STL headers
#include <random>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>

namespace parser_ns = dbengine::parser;

// Creates database and checks it was created by selecting from system database
TEST(DDL, CreateDatabase)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    const std::vector<std::pair<std::string, std::string>> parameters {
            {"none", ""},
            {"aes128", ""},
            {"aes128", "abc"},
            {"aes192", "def"},
            {"aes256", "xyz"},
            {"camellia128", "abc"},
            {"camellia192", "def"},
            {"camellia256", "xyz"},
    };

    std::size_t index = 0;
    for (const auto& [cipherId, keySeed] : parameters) {
        ++index;
        auto databaseName = "TEST_DB_" + cipherId + "_" + keySeed;
        boost::to_upper(databaseName);
        {
            /// ----------- CREATE DATABASE -----------
            const std::string statement = stdext::string_builder()
                                          << "CREATE DATABASE " << databaseName
                                          << " WITH CIPHER_ID = '" << cipherId
                                          << "', CIPHER_KEY_SEED = '" << keySeed << '\'';

            parser_ns::SqlParser parser(statement);
            parser.parse();

            parser_ns::DBEngineSqlRequestFactory factory(parser);
            const auto request = factory.createSqlRequest();

            requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

            siodb::iomgr_protocol::DatabaseEngineResponse response;
            siodb::protobuf::StreamInputStream inputStream(
                    TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

            siodb::protobuf::readMessage(
                    siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response,
                    inputStream);

            EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
            ASSERT_EQ(response.message_size(), 0);
            EXPECT_FALSE(response.has_affected_row_count());
            EXPECT_EQ(response.response_id(), 0U);
            EXPECT_EQ(response.response_count(), 1U);
        }

        /// ----------- SELECT -----------
        {
            const std::string statement = stdext::string_builder()
                                          << "SELECT NAME FROM SYS.SYS_DATABASES WHERE NAME = '"
                                          << databaseName << "' AND CIPHER_ID = '" << cipherId
                                          << '\'';

            parser_ns::SqlParser parser(statement);
            parser.parse();

            parser_ns::DBEngineSqlRequestFactory factory(parser);
            const auto request = factory.createSqlRequest();

            requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

            siodb::iomgr_protocol::DatabaseEngineResponse response;
            siodb::protobuf::StreamInputStream inputStream(
                    TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
            siodb::protobuf::readMessage(
                    siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response,
                    inputStream);

            EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
            ASSERT_EQ(response.message_size(), 0);
            EXPECT_FALSE(response.has_affected_row_count());
            ASSERT_EQ(response.column_description_size(), 1);
            EXPECT_FALSE(response.has_affected_row_count());

            EXPECT_EQ(response.column_description(0).name(), "NAME");

            siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

            std::uint64_t rowLength = 0;

            // Only one row with a single just created database
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_GT(rowLength, 0U);

            std::string name;
            ASSERT_TRUE(codedInput.Read(&name));
            ASSERT_EQ(name, databaseName);

            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_TRUE(rowLength == 0U);
        }

        {
            const auto requestHandler = TestEnvironment::makeRequestHandler();

            const std::string statement = stdext::string_builder()
                                          << "DROP DATABASE "
                                          << (index % 2 == 0 ? "IF EXISTS " : "") << databaseName;
            parser_ns::SqlParser parser(statement);
            parser.parse();

            parser_ns::DBEngineSqlRequestFactory factory(parser);
            const auto request = factory.createSqlRequest();

            requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

            siodb::iomgr_protocol::DatabaseEngineResponse response;
            siodb::protobuf::StreamInputStream inputStream(
                    TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

            siodb::protobuf::readMessage(
                    siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response,
                    inputStream);

            EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
            ASSERT_EQ(response.message_size(), 0);
            EXPECT_FALSE(response.has_affected_row_count());
            EXPECT_EQ(response.response_id(), 0U);
            EXPECT_EQ(response.response_count(), 1U);
        }
    }
}

TEST(DDL, DropDatabase_NonExistentDB)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    parser_ns::SqlParser parser("DROP DATABASE NO_SUCH_DATABASE_FOR_SURE;");
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
    ASSERT_EQ(response.message_size(), 1);  // message "Database doesn't exists"
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
}

TEST(DDL, DropDatabaseIfExists_NonExistentDB)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    parser_ns::SqlParser parser("DROP DATABASE IF EXISTS NO_SUCH_DATABASE_FOR_SURE;");
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
}

TEST(DDL, UseDatabase_ExistentDB)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    {
        /// ----------- CREATE DATABASE -----------
        const std::string statement = stdext::string_builder()
                                      << "CREATE DATABASE UseDatabase_ExistentDB_database";

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        EXPECT_EQ(response.response_id(), 0U);
        EXPECT_EQ(response.response_count(), 1U);
    }

    const std::string tableName = stdext::string_builder()
                                  << "TABLE_" << std::time(nullptr) << '_' << ::getpid();
    {
        /// ----------- CREATE TABLE -----------
        // Create a table in current database

        const std::string statement = stdext::string_builder()
                                      << "CREATE TABLE UseDatabase_ExistentDB_database."
                                      << tableName << " (TEST text)";
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        EXPECT_EQ(response.response_id(), 0U);
        EXPECT_EQ(response.response_count(), 1U);
    }

    {
        /// ----------- USE DATABASE -----------
        parser_ns::SqlParser parser("USE DATABASE UseDatabase_ExistentDB_database");
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        EXPECT_EQ(response.response_id(), 0U);
        EXPECT_EQ(response.response_count(), 1U);
    }

    /// ----------- SELECT -----------
    {
        const std::string statement = stdext::string_builder() << "SELECT * FROM " << tableName;

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);
        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 2);

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_EQ(rowLength, 0U);
    }
}

TEST(DDL, UseDatabase_NonExistentDB)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    parser_ns::SqlParser parser("USE DATABASE NO_SUCH_DATABASE_FOR_SURE;");
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
    ASSERT_EQ(response.message_size(), 1);  // message "Database doesn't exists"
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);
}

TEST(DDL, DropUsedDatabase)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    siodb::protobuf::StreamInputStream inputStream(
            TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

    {
        /// ----------- CREATE DATABASE -----------
        const std::string statement = stdext::string_builder()
                                      << "CREATE DATABASE DropUsedDatabase_database";

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::StreamInputStream inputStream(
                TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        EXPECT_EQ(response.response_id(), 0U);
        EXPECT_EQ(response.response_count(), 1U);
    }

    {
        /// ----------- USE DATABASE -----------
        parser_ns::SqlParser parser("USE DATABASE DropUsedDatabase_database;");
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;

        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        EXPECT_EQ(response.response_id(), 0U);
        EXPECT_EQ(response.response_count(), 1U);
    }

    {
        /// ----------- DROP DATABASE -----------
        parser_ns::SqlParser parser("DROP DATABASE DropUsedDatabase_database;");
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;

        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 1);
        EXPECT_FALSE(response.has_affected_row_count());
        EXPECT_EQ(response.response_id(), 0U);
        EXPECT_EQ(response.response_count(), 1U);  // Can't remove used database
    }
}

TEST(DDL, CreateDuplicateColumnTable)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- CREATE TABLE -----------
    const std::string statement("CREATE TABLE test.DDL_TEST_TABLE_1 (TEST text, TEST text)");

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
    ASSERT_EQ(response.message_size(), 1);
    EXPECT_FALSE(response.has_affected_row_count());
    EXPECT_EQ(response.response_id(), 0U);
    EXPECT_EQ(response.response_count(), 1U);

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM test.DDL_TEST_TABLE_1");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 1);  // error - table does not exist;
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 0);
    }
}

TEST(DDL, CreateTable)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- CREATE TABLE -----------
    const std::string statement(
            "CREATE TABLE DDL_TEST_TABLE_2 (TEST_INTEGER INTEGER, TEST_INT INT, TEST_UINT "
            "UINT,  "
            "TEST_TINYINT TINYINT,  TEST_TINYUINT TINYUINT,  TEST_SMALLINT SMALLINT,  "
            "TEST_SMALLUINT SMALLUINT,  "
            "TEST_BIGINT BIGINT,  TEST_BIGUINT BIGUINT, TEST_SMALLREAL SMALLREAL,  TEST_REAL REAL, "
            "TEST_FLOAT FLOAT,  TEST_DOUBLE DOUBLE, TEST_TEXT TEXT, TEST_CHAR CHAR, "
            "TEST_VARCHAR VARCHAR, TEST_BLOB BLOB, TEST_TIMESTAMP TIMESTAMP)");

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

    /// ----------- SELECT -----------
    {
        const std::string statement("SELECT * FROM DDL_TEST_TABLE_2");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        ASSERT_EQ(response.column_description_size(), 19);  // +1 for TRID
        EXPECT_FALSE(response.has_affected_row_count());
        // TRID
        EXPECT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        // INTEGER
        EXPECT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT32);
        // INT
        EXPECT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_INT32);
        // UINT
        EXPECT_EQ(response.column_description(3).type(), siodb::COLUMN_DATA_TYPE_UINT32);
        // TINYINT
        EXPECT_EQ(response.column_description(4).type(), siodb::COLUMN_DATA_TYPE_INT8);
        //  TINYUINT
        EXPECT_EQ(response.column_description(5).type(), siodb::COLUMN_DATA_TYPE_UINT8);
        // SMALLINT
        EXPECT_EQ(response.column_description(6).type(), siodb::COLUMN_DATA_TYPE_INT16);
        // SMALLUINT
        EXPECT_EQ(response.column_description(7).type(), siodb::COLUMN_DATA_TYPE_UINT16);
        // BIGINT
        EXPECT_EQ(response.column_description(8).type(), siodb::COLUMN_DATA_TYPE_INT64);
        // BIGUINT
        EXPECT_EQ(response.column_description(9).type(), siodb::COLUMN_DATA_TYPE_UINT64);
        // SMALLREAL
        EXPECT_EQ(response.column_description(10).type(), siodb::COLUMN_DATA_TYPE_FLOAT);
        // REAL
        EXPECT_EQ(response.column_description(11).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);
        // FLOAT
        EXPECT_EQ(response.column_description(12).type(), siodb::COLUMN_DATA_TYPE_FLOAT);
        // DOUBLE
        EXPECT_EQ(response.column_description(13).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);
        // TEXT
        EXPECT_EQ(response.column_description(14).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        // CHAR
        EXPECT_EQ(response.column_description(15).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        // VARCHAR
        EXPECT_EQ(response.column_description(16).type(), siodb::COLUMN_DATA_TYPE_TEXT);
        // BLOB
        EXPECT_EQ(response.column_description(17).type(), siodb::COLUMN_DATA_TYPE_BINARY);
        // TIMESTAMP
        EXPECT_EQ(response.column_description(18).type(), siodb::COLUMN_DATA_TYPE_TIMESTAMP);

        EXPECT_EQ(response.column_description(0).name(), "TRID");
        EXPECT_EQ(response.column_description(1).name(), "TEST_INTEGER");
        EXPECT_EQ(response.column_description(2).name(), "TEST_INT");
        EXPECT_EQ(response.column_description(3).name(), "TEST_UINT");
        EXPECT_EQ(response.column_description(4).name(), "TEST_TINYINT");
        EXPECT_EQ(response.column_description(5).name(), "TEST_TINYUINT");
        EXPECT_EQ(response.column_description(6).name(), "TEST_SMALLINT");
        EXPECT_EQ(response.column_description(7).name(), "TEST_SMALLUINT");
        EXPECT_EQ(response.column_description(8).name(), "TEST_BIGINT");
        EXPECT_EQ(response.column_description(9).name(), "TEST_BIGUINT");
        EXPECT_EQ(response.column_description(10).name(), "TEST_SMALLREAL");
        EXPECT_EQ(response.column_description(11).name(), "TEST_REAL");
        EXPECT_EQ(response.column_description(12).name(), "TEST_FLOAT");
        EXPECT_EQ(response.column_description(13).name(), "TEST_DOUBLE");
        EXPECT_EQ(response.column_description(14).name(), "TEST_TEXT");
        EXPECT_EQ(response.column_description(15).name(), "TEST_CHAR");
        EXPECT_EQ(response.column_description(16).name(), "TEST_VARCHAR");
        EXPECT_EQ(response.column_description(17).name(), "TEST_BLOB");
        EXPECT_EQ(response.column_description(18).name(), "TEST_TIMESTAMP");

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        ASSERT_EQ(rowLength, 0U);
    }
}

// Intent of this test is to check correctness of the some use cases in the block registry.
void createManyTablesTest(unsigned long long seed)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    std::mt19937 gen(seed);
    std::uniform_real_distribution realDist(0.0, 1.0);
    std::uniform_int_distribution<int> intDist(1, std::numeric_limits<int16_t>::max());
    const auto ts = std::time(nullptr);

    constexpr int kTableCount = 60;
    for (int tableNo = 0; tableNo < kTableCount; ++tableNo) {
        /// ----------- CREATE TABLE -----------
        const std::string tableName = stdext::string_builder() << "DDL_TEST_TABLE_MANY_" << tableNo
                                                               << '_' << seed << '_' << ts;
        std::size_t randomStringLength = (intDist(gen) % 100) + 1;
        std::string randomString;
        randomString.resize(randomStringLength);
        for (std::size_t i = 0; i < randomStringLength; ++i) {
            unsigned char ch;
            do {
                ch = intDist(gen);
            } while (ch == 0 || ch > 0x7f || ch == '\'' || ch == '\"');
            randomString[i] = static_cast<char>(ch);
        }
        LOG_DEBUG << "====== CREATE TABLE " << tableName << "======";
        const std::string statement =
                stdext::string_builder()
                << "CREATE TABLE " << tableName << "\n(TEST_INTEGER INTEGER DEFAULT "
                << intDist(gen) << "\n,  TEST_INT INT NOT NULL DEFAULT " << intDist(gen)
                << "\n, TEST_UINT UINT DEFAULT " << intDist(gen)
                << "\n, TEST_TINYINT TINYINT NOT NULL DEFAULT "
                << (intDist(gen) % std::numeric_limits<std::int8_t>::max())
                << "\n, TEST_TINYUINT TINYUINT DEFAULT "
                << (intDist(gen) % std::numeric_limits<std::uint8_t>::max())
                << "\n, TEST_SMALLINT SMALLINT NOT NULL DEFAULT " << intDist(gen)
                << "\n, TEST_SMALLUINT SMALLUINT DEFAULT " << intDist(gen)
                << "\n, TEST_BIGINT BIGINT NOT NULL DEFAULT " << intDist(gen)
                << "\n, TEST_BIGUINT BIGUINT DEFAULT " << intDist(gen)
                << "\n, TEST_SMALLREAL SMALLREAL NOT NULL DEFAULT " << realDist(gen)
                << "\n, TEST_REAL REAL DEFAULT " << realDist(gen)
                << "\n, TEST_FLOAT FLOAT NOT NULL DEFAULT " << intDist(gen)
                << "\n, TEST_DOUBLE DOUBLE DEFAULT " << intDist(gen)
                << "\n, TEST_TEXT TEXT NOT NULL DEFAULT '" << randomString << "-zzz'"
                << "\n, TEST_CHAR CHAR DEFAULT '" << randomString[0] << "'"
                << "\n, TEST_VARCHAR VARCHAR NOT NULL DEFAULT '" << randomString << "'"
                << "\n, TEST_BLOB BLOB,  TEST_TIMESTAMP TIMESTAMP NOT NULL)\n";

        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::StreamInputStream inputStream(
                TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());

        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        EXPECT_EQ(response.response_id(), 0U);
        EXPECT_EQ(response.response_count(), 1U);

        /// ----------- SELECT -----------
        {
            const std::string statement = stdext::string_builder() << "SELECT * FROM " << tableName;
            parser_ns::SqlParser parser(statement);
            parser.parse();

            parser_ns::DBEngineSqlRequestFactory factory(parser);
            const auto request = factory.createSqlRequest();

            requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

            siodb::iomgr_protocol::DatabaseEngineResponse response;
            siodb::protobuf::readMessage(
                    siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response,
                    inputStream);

            EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
            ASSERT_EQ(response.message_size(), 0);
            EXPECT_FALSE(response.has_affected_row_count());
            ASSERT_EQ(response.column_description_size(), 19);  // +1 for TRID
            EXPECT_FALSE(response.has_affected_row_count());
            // TRID
            EXPECT_EQ(response.column_description(0).type(), siodb::COLUMN_DATA_TYPE_UINT64);
            // INTEGER
            EXPECT_EQ(response.column_description(1).type(), siodb::COLUMN_DATA_TYPE_INT32);
            // INT
            EXPECT_EQ(response.column_description(2).type(), siodb::COLUMN_DATA_TYPE_INT32);
            // UINT
            EXPECT_EQ(response.column_description(3).type(), siodb::COLUMN_DATA_TYPE_UINT32);
            // TINYINT
            EXPECT_EQ(response.column_description(4).type(), siodb::COLUMN_DATA_TYPE_INT8);
            //  TINYUINT
            EXPECT_EQ(response.column_description(5).type(), siodb::COLUMN_DATA_TYPE_UINT8);
            // SMALLINT
            EXPECT_EQ(response.column_description(6).type(), siodb::COLUMN_DATA_TYPE_INT16);
            // SMALLUINT
            EXPECT_EQ(response.column_description(7).type(), siodb::COLUMN_DATA_TYPE_UINT16);
            // BIGINT
            EXPECT_EQ(response.column_description(8).type(), siodb::COLUMN_DATA_TYPE_INT64);
            // BIGUINT
            EXPECT_EQ(response.column_description(9).type(), siodb::COLUMN_DATA_TYPE_UINT64);
            // SMALLREAL
            EXPECT_EQ(response.column_description(10).type(), siodb::COLUMN_DATA_TYPE_FLOAT);
            // REAL
            EXPECT_EQ(response.column_description(11).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);
            // FLOAT
            EXPECT_EQ(response.column_description(12).type(), siodb::COLUMN_DATA_TYPE_FLOAT);
            // DOUBLE
            EXPECT_EQ(response.column_description(13).type(), siodb::COLUMN_DATA_TYPE_DOUBLE);
            // TEXT
            EXPECT_EQ(response.column_description(14).type(), siodb::COLUMN_DATA_TYPE_TEXT);
            // CHAR
            EXPECT_EQ(response.column_description(15).type(), siodb::COLUMN_DATA_TYPE_TEXT);
            // VARCHAR
            EXPECT_EQ(response.column_description(16).type(), siodb::COLUMN_DATA_TYPE_TEXT);
            // BLOB
            EXPECT_EQ(response.column_description(17).type(), siodb::COLUMN_DATA_TYPE_BINARY);
            // TIMESTAMP
            EXPECT_EQ(response.column_description(18).type(), siodb::COLUMN_DATA_TYPE_TIMESTAMP);

            EXPECT_EQ(response.column_description(0).name(), "TRID");
            EXPECT_EQ(response.column_description(1).name(), "TEST_INTEGER");
            EXPECT_EQ(response.column_description(2).name(), "TEST_INT");
            EXPECT_EQ(response.column_description(3).name(), "TEST_UINT");
            EXPECT_EQ(response.column_description(4).name(), "TEST_TINYINT");
            EXPECT_EQ(response.column_description(5).name(), "TEST_TINYUINT");
            EXPECT_EQ(response.column_description(6).name(), "TEST_SMALLINT");
            EXPECT_EQ(response.column_description(7).name(), "TEST_SMALLUINT");
            EXPECT_EQ(response.column_description(8).name(), "TEST_BIGINT");
            EXPECT_EQ(response.column_description(9).name(), "TEST_BIGUINT");
            EXPECT_EQ(response.column_description(10).name(), "TEST_SMALLREAL");
            EXPECT_EQ(response.column_description(11).name(), "TEST_REAL");
            EXPECT_EQ(response.column_description(12).name(), "TEST_FLOAT");
            EXPECT_EQ(response.column_description(13).name(), "TEST_DOUBLE");
            EXPECT_EQ(response.column_description(14).name(), "TEST_TEXT");
            EXPECT_EQ(response.column_description(15).name(), "TEST_CHAR");
            EXPECT_EQ(response.column_description(16).name(), "TEST_VARCHAR");
            EXPECT_EQ(response.column_description(17).name(), "TEST_BLOB");
            EXPECT_EQ(response.column_description(18).name(), "TEST_TIMESTAMP");

            siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

            std::uint64_t rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_EQ(rowLength, 0U);
        }
    }
}

TEST(DDL, CreateManyTables_Random)
{
    // TODO(cxxman): Commented out for now, but uncomment later,
    //  when CreateManyTables_Seeded2 fixed
#if 1
    std::random_device rd;
    const auto seed = rd();
    LOG_INFO << "DDL.CreateManyTables.seed=" << seed;
    createManyTablesTest(seed);
#endif
}

TEST(DDL, CreateManyTables_Seeded1)
{
    // TODO(cxxman): Commented out for now, but uncomment later,
    //  when CreateManyTables_Seeded2 fixed
#if 1
    const auto seed = 4281804057UL;
    createManyTablesTest(seed);
#endif
}

TEST(DDL, CreateManyTables_Seeded2)
{
    const auto seed = 2888146611UL;
    createManyTablesTest(seed);
}

TEST(DDL, CreateTableWithDefaultValue)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- CREATE TABLE -----------
    const std::string statement(
            "CREATE TABLE DDL_TEST_TABLE_WITH_DEFAULT_VALUE (id INTEGER DEFAULT 5)");
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
}

TEST(DDL, CreateTableWithNotNullAndDefaultValue)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    /// ----------- CREATE TABLE -----------
    const std::string statement(
            "CREATE TABLE DEFAULT_VALUE_TEST(A integer not null default 100, B integer not "
            "null)");
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
}

TEST(DDL, SetTableAttributes_NextTrid)
{
    const auto requestHandler = TestEnvironment::makeRequestHandler();

    {
        /// ----------- CREATE TABLE -----------
        const std::string statement("CREATE TABLE DDL_TEST_TABLE_444 (TEST_INTEGER INTEGER)");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::StreamInputStream inputStream(
                TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);

        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        EXPECT_EQ(response.response_id(), 0U);
        EXPECT_EQ(response.response_count(), 1U);
    }

    {
        /// ----------- ALTER TABLE -----------
        const std::string statement("ALTER TABLE DDL_TEST_TABLE_444 SET NEXT_TRID=222");
        parser_ns::SqlParser parser(statement);
        parser.parse();

        parser_ns::DBEngineSqlRequestFactory factory(parser);
        const auto request = factory.createSqlRequest();

        requestHandler->executeRequest(*request, TestEnvironment::kTestRequestId, 0, 1);

        siodb::iomgr_protocol::DatabaseEngineResponse response;
        siodb::protobuf::StreamInputStream inputStream(
                TestEnvironment::getInputStream(), siodb::utils::DefaultErrorCodeChecker());
        siodb::protobuf::readMessage(siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                response, inputStream);
        EXPECT_EQ(response.request_id(), TestEnvironment::kTestRequestId);
        ASSERT_EQ(response.message_size(), 0);
        EXPECT_FALSE(response.has_affected_row_count());
        EXPECT_EQ(response.response_id(), 0U);
        EXPECT_EQ(response.response_count(), 1U);
    }
}
