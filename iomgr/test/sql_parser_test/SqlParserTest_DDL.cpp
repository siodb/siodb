// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "TestContext.h"
#include "dbengine/parser/DBEngineRequestFactory.h"
#include "dbengine/parser/EmptyContext.h"
#include "dbengine/parser/SqlParser.h"

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;

TEST(DDL, AttachDatabase)
{
    // Parse statement and prepare request
    const std::string statement(
            "ATTACH DATABASE 'c44efa74-d912-4e13-a4cb-03847349531d' AS my_database");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kAttachDatabase);

    // Check request
    const auto& request = dynamic_cast<const requests::AttachDatabaseRequest&>(*dbeRequest);
    boost::uuids::string_generator gen;
    const auto uuid = gen("c44efa74-d912-4e13-a4cb-03847349531d");
    EXPECT_EQ(request.m_databaseUuid, uuid);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
}

TEST(DDL, DetachDatabase)
{
    // Parse statement and prepare request
    const std::string statement("DETACH DATABASE my_database");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDetachDatabase);

    // Check request
    const auto& request = dynamic_cast<const requests::DetachDatabaseRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
}

TEST(DDL, CreateDatabase)
{
    // Parse statement and prepare request
    const std::string statement("CREATE DATABASE my_database");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kCreateDatabase);

    // Check request
    const auto& request = dynamic_cast<const requests::CreateDatabaseRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_FALSE(request.m_temporary);
    EXPECT_EQ(request.m_cipherId, nullptr);
    EXPECT_EQ(request.m_cipherKeySeed, nullptr);
}

TEST(DDL, CreateTempDatabase)
{
    // Parse statement and prepare request
    const std::string statement("CREATE TEMP DATABASE my_database");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kCreateDatabase);

    // Check request
    const auto& request = dynamic_cast<const requests::CreateDatabaseRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_TRUE(request.m_temporary);
    EXPECT_EQ(request.m_cipherId, nullptr);
    EXPECT_EQ(request.m_cipherKeySeed, nullptr);
}

TEST(DDL, CreateDatabaseWithOptions)
{
    // Parse statement and prepare request
    const std::string statement(
            "CREATE DATABASE my_database with CIPHER_ID='aes128k128', CIPHER_KEY_SEED = "
            "'fksgksgjrekgjerkglerjg'");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    EXPECT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kCreateDatabase);

    // Check request
    const auto& request = dynamic_cast<const requests::CreateDatabaseRequest&>(*dbeRequest);

    requests::EmptyContext emptyContext;
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_FALSE(request.m_temporary);
    EXPECT_EQ(request.m_cipherId->evaluate(emptyContext), "aes128k128");
    EXPECT_EQ(request.m_cipherKeySeed->evaluate(emptyContext), "fksgksgjrekgjerkglerjg");
}

TEST(DDL, DropDatabase)
{
    // Parse statement and prepare request
    const std::string statement("DROP DATABASE my_database");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDropDatabase);

    // Check request
    const auto& request = dynamic_cast<const requests::DropDatabaseRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_FALSE(request.m_ifExists);
}

TEST(DDL, DropDatabaseIfExists)
{
    // Parse statement and prepare request
    const std::string statement("DROP DATABASE IF EXISTS my_database");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDropDatabase);

    // Check request
    const auto& request = dynamic_cast<const requests::DropDatabaseRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_TRUE(request.m_ifExists);
}

TEST(DDL, CreateTable)
{
    // Parse statement and prepare request
    const std::string statement(
            "CREATE TABLE my_database.my_table (\n"
            "  first_name TEXT NOT NULL,\n"
            "  address1 TEXT CONSTRAINT FK_OTHER REFERENCES xx(yy),\n"
            "  address2 TEXT NULL DEFAULT 'zzz',\n"
            "  birth_date TIMESTAMP NULL DEFAULT '1970-01-01',\n"
            "  balance REAL CONSTRAINT NN_BALANCE NOT NULL CONSTRAINT DEF_BALANCE DEFAULT 0.0\n"
            ");");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kCreateTable);

    // Check request
    const auto& request = dynamic_cast<const requests::CreateTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");

    // Check columns
    ASSERT_EQ(request.m_columns.size(), 5U);
    EXPECT_EQ(request.m_columns[0].m_name, "FIRST_NAME");
    EXPECT_EQ(request.m_columns[1].m_name, "ADDRESS1");
    EXPECT_EQ(request.m_columns[2].m_name, "ADDRESS2");
    EXPECT_EQ(request.m_columns[3].m_name, "BIRTH_DATE");
    EXPECT_EQ(request.m_columns[4].m_name, "BALANCE");
    EXPECT_EQ(request.m_columns[0].m_dataType, siodb::COLUMN_DATA_TYPE_TEXT);
    EXPECT_EQ(request.m_columns[1].m_dataType, siodb::COLUMN_DATA_TYPE_TEXT);
    EXPECT_EQ(request.m_columns[2].m_dataType, siodb::COLUMN_DATA_TYPE_TEXT);
    EXPECT_EQ(request.m_columns[3].m_dataType, siodb::COLUMN_DATA_TYPE_TIMESTAMP);
    EXPECT_EQ(request.m_columns[4].m_dataType, siodb::COLUMN_DATA_TYPE_DOUBLE);
}

TEST(DDL, DropTable)
{
    // Parse statement and prepare request
    const std::string statement("DROP TABLE my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDropTable);

    // Check request
    const auto& request = dynamic_cast<const requests::DropTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");
    EXPECT_FALSE(request.m_ifExists);
}

TEST(DDL, DropTableIfExists)
{
    // Parse statement and prepare request
    const std::string statement("DROP TABLE IF EXISTS my_database.my_table;");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDropTable);

    // Check request
    const auto& request = dynamic_cast<const requests::DropTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");
    EXPECT_TRUE(request.m_ifExists);
}

TEST(DDL, AlterTableRenameTo)
{
    // Parse statement and prepare request
    const std::string statement("ALTER TABLE my_database.my_table RENAME TO my_table2");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kRenameTable);

    // Check request
    const auto& request = dynamic_cast<const requests::RenameTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_oldTable, "MY_TABLE");
    EXPECT_EQ(request.m_newTable, "MY_TABLE2");
    EXPECT_FALSE(request.m_ifExists);
}

TEST(DDL, AlterTableRenameToIfExists)
{
    // Parse statement and prepare request
    const std::string statement("ALTER TABLE my_database.my_table RENAME IF EXISTS TO my_table2");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kRenameTable);

    // Check request
    const auto& request = dynamic_cast<const requests::RenameTableRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_oldTable, "MY_TABLE");
    EXPECT_EQ(request.m_newTable, "MY_TABLE2");
    EXPECT_TRUE(request.m_ifExists);
}

TEST(DDL, AlterTableSetTableAttributes)
{
    // Parse statement and prepare request
    const std::string statement("ALTER TABLE my_database.my_table SET next_trid=288449");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kSetTableAttributes);

    // Check request
    const auto& request = dynamic_cast<const requests::SetTableAttributesRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");
    ASSERT_TRUE(request.m_nextTrid.has_value());
    EXPECT_EQ(*request.m_nextTrid, 288449U);
}

TEST(DDL, AlterTableAddColumn)
{
    // Parse statement and prepare request
    const std::string statement(
            "ALTER TABLE my_database.my_table"
            " ADD COLUMN last_name TEXT NOT NULL");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kAddColumn);

    // Check request
    const auto& request = dynamic_cast<const requests::AddColumnRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");
    EXPECT_EQ(request.m_column.m_name, "LAST_NAME");
    EXPECT_EQ(request.m_column.m_dataType, siodb::COLUMN_DATA_TYPE_TEXT);
}

TEST(DDL, AlterTableDropColumn)
{
    // Parse statement and prepare request
    const std::string statement("ALTER TABLE my_database.my_table DROP COLUMN column1;");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDropColumn);

    // Check request
    const auto& request = dynamic_cast<const requests::DropColumnRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");
    EXPECT_EQ(request.m_column, "COLUMN1");
    EXPECT_FALSE(request.m_ifExists);
}

TEST(DDL, AlterTableDropColumnIfExists)
{
    // Parse statement and prepare request
    const std::string statement(
            "ALTER TABLE my_database.my_table"
            " DROP COLUMN IF EXISTS column1;");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDropColumn);

    // Check request
    const auto& request = dynamic_cast<const requests::DropColumnRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");
    EXPECT_EQ(request.m_column, "COLUMN1");
    EXPECT_TRUE(request.m_ifExists);
}

TEST(DDL, CreateIndex)
{
    // Parse statement and prepare request
    const std::string statement(
            "CREATE INDEX my_database.my_index ON my_table (\n"
            "  implicit_asc_column,\n"
            "  explicit_asc_column ASC,\n"
            "  explicit_desc_column DESC);\n");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kCreateIndex);

    // Check request
    const auto& request = dynamic_cast<const requests::CreateIndexRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");
    EXPECT_EQ(request.m_index, "MY_INDEX");
    EXPECT_FALSE(request.m_unique);
    EXPECT_FALSE(request.m_ifDoesntExist);

    // Check columns
    ASSERT_EQ(request.m_columns.size(), 3U);
    EXPECT_EQ(request.m_columns[0].m_name, "IMPLICIT_ASC_COLUMN");
    EXPECT_FALSE(request.m_columns[0].m_sortDescending);
    EXPECT_EQ(request.m_columns[1].m_name, "EXPLICIT_ASC_COLUMN");
    EXPECT_FALSE(request.m_columns[1].m_sortDescending);
    EXPECT_EQ(request.m_columns[2].m_name, "EXPLICIT_DESC_COLUMN");
    EXPECT_TRUE(request.m_columns[2].m_sortDescending);
}

TEST(DDL, CreateIndexIfNotExists)
{
    // Parse statement and prepare request
    const std::string statement(
            "CREATE INDEX IF NOT EXISTS my_database.my_index ON my_table (\n"
            "  implicit_asc_column,\n"
            "  explicit_asc_column ASC,\n"
            "  explicit_desc_column DESC);\n");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kCreateIndex);

    // Check request
    const auto& request = dynamic_cast<const requests::CreateIndexRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");
    EXPECT_EQ(request.m_index, "MY_INDEX");
    EXPECT_FALSE(request.m_unique);
    EXPECT_TRUE(request.m_ifDoesntExist);

    // Check columns
    ASSERT_EQ(request.m_columns.size(), 3U);
    EXPECT_EQ(request.m_columns[0].m_name, "IMPLICIT_ASC_COLUMN");
    EXPECT_FALSE(request.m_columns[0].m_sortDescending);
    EXPECT_EQ(request.m_columns[1].m_name, "EXPLICIT_ASC_COLUMN");
    EXPECT_FALSE(request.m_columns[1].m_sortDescending);
    EXPECT_EQ(request.m_columns[2].m_name, "EXPLICIT_DESC_COLUMN");
    EXPECT_TRUE(request.m_columns[2].m_sortDescending);
}

TEST(DDL, CreateUniqueIndex)
{
    // Parse statement and prepare request
    const std::string statement(
            "CREATE UNIQUE INDEX my_database.my_index ON my_table (\n"
            "  implicit_asc_column,\n"
            "  explicit_asc_column ASC,\n"
            "  explicit_desc_column DESC);\n");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kCreateIndex);

    // Check request
    const auto& request = dynamic_cast<const requests::CreateIndexRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_table, "MY_TABLE");
    EXPECT_EQ(request.m_index, "MY_INDEX");
    EXPECT_TRUE(request.m_unique);
    EXPECT_FALSE(request.m_ifDoesntExist);

    // Check columns
    ASSERT_EQ(request.m_columns.size(), 3U);
    EXPECT_EQ(request.m_columns[0].m_name, "IMPLICIT_ASC_COLUMN");
    EXPECT_FALSE(request.m_columns[0].m_sortDescending);
    EXPECT_EQ(request.m_columns[1].m_name, "EXPLICIT_ASC_COLUMN");
    EXPECT_FALSE(request.m_columns[1].m_sortDescending);
    EXPECT_EQ(request.m_columns[2].m_name, "EXPLICIT_DESC_COLUMN");
    EXPECT_TRUE(request.m_columns[2].m_sortDescending);
}

TEST(DDL, DropIndex)
{
    // Parse statement and prepare request
    const std::string statement("DROP INDEX my_database.my_index;");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDropIndex);

    // Check request
    const auto& request = dynamic_cast<const requests::DropIndexRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_index, "MY_INDEX");
    EXPECT_FALSE(request.m_ifExists);
}

TEST(DDL, DropIndexIfExists)
{
    // Parse statement and prepare request
    const std::string statement("DROP INDEX IF EXISTS my_database.my_index;");
    parser_ns::SqlParser parser(statement);
    parser.parse();
    const auto dbeRequest =
            parser_ns::DBEngineRequestFactory::createRequest(parser.findStatement(0));

    // Check request type
    ASSERT_EQ(dbeRequest->m_requestType, requests::DBEngineRequestType::kDropIndex);

    // Check request
    const auto& request = dynamic_cast<const requests::DropIndexRequest&>(*dbeRequest);
    EXPECT_EQ(request.m_database, "MY_DATABASE");
    EXPECT_EQ(request.m_index, "MY_INDEX");
    EXPECT_TRUE(request.m_ifExists);
}
