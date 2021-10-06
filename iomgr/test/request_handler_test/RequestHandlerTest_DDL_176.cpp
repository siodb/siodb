// Copyright (C) 2021 Siodb GmbH. All rights reserved.
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
#include <siodb/common/protobuf/ExtendedCodedInputStream.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/sstream_ext.h>
#include <siodb/iomgr/shared/dbengine/util/RowDecoder.h>

namespace parser_ns = dbengine::parser;

namespace {

void CreateNColumnsAndTryToSelectFromSysColumns(std::size_t numberOfColumns)
{
    const auto requestHandler = TestEnvironment::makeRequestHandlerForSuperUser();
    const auto databaseName = stdext::concat("DB_MANY_COLS_", numberOfColumns);

    // ----------- CREATE DATABASE -----------
    {
        const auto statement = stdext::concat("CREATE DATABASE ", databaseName);
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

    // Create table
    {
        const auto instance = TestEnvironment::getInstance();
        const auto database = instance->findDatabase(databaseName);
        const auto numberOfColumnsInSystemTables = database->countColumnsInSystemTables();
        LOG_INFO << "numberOfColumnsInSystemTables=" << numberOfColumnsInSystemTables;
        // below -1 is for TRID
        const auto numberOfCustomColumns = numberOfColumns - numberOfColumnsInSystemTables - 1;
        std::vector<dbengine::SimpleColumnSpecification> tableColumns;
        for (std::size_t i = 1; i <= numberOfCustomColumns; ++i) {
            tableColumns.emplace_back("C" + std::to_string(i), siodb::COLUMN_DATA_TYPE_INT32, true);
        }

        ASSERT_NE(instance, nullptr);
        constexpr const char* kTableName = "TABLE_1";
        instance->findDatabase(databaseName)
                ->createUserTable(kTableName, dbengine::TableType::kDisk, tableColumns,
                        dbengine::User::kSuperUserId, {});
    }

    // SELECT from SYS_COLUMNS
    {
        const auto statement = stdext::concat("SELECT * FROM ", databaseName, ".SYS_COLUMNS");
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

        siodb::protobuf::ExtendedCodedInputStream codedInput(&inputStream);

        std::uint64_t rowLength = 0;
        std::vector<std::uint8_t> rowData;
        for (std::size_t i = 0; i < numberOfColumns; ++i) {
            rowLength = 0;
            ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
            ASSERT_LT(rowLength, 512);
            ASSERT_GT(rowLength, 0);
            if (rowData.size() < rowLength) rowData.resize(rowLength);
            ASSERT_TRUE(codedInput.ReadRaw(rowData.data(), rowLength));
        }

        ASSERT_TRUE(codedInput.ReadVarint64(&rowLength));
        EXPECT_EQ(rowLength, 0U);
    }
}

}  // anonymous namespace

TEST(DDL, Create184ColumnsAndTryToSelectFromSysColumns)
{
    CreateNColumnsAndTryToSelectFromSysColumns(184);
}

TEST(DDL, DISABLED_Create185ColumnsAndTryToSelectFromSysColumns)
{
    CreateNColumnsAndTryToSelectFromSysColumns(185);
}
