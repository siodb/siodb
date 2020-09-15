// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/reg/CipherKeyRecord.h"
#include "dbengine/reg/ColumnDefinitionRecord.h"
#include "dbengine/reg/ColumnSetRecord.h"
#include "dbengine/reg/ConstraintRecord.h"
#include "dbengine/reg/DatabaseRecord.h"
#include "dbengine/reg/IndexRecord.h"
#include "dbengine/reg/RegistryRecordUuidChecker.h"
#include "dbengine/reg/TableRecord.h"
#include "dbengine/reg/UserPermissionRecord.h"
#include "dbengine/reg/UserRecord.h"

// Boost headers
#include <boost/lexical_cast.hpp>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;

namespace {

constexpr std::size_t kExtraBufferSize = 0x10000U;

template<class Record>
void checkRecord(
        const Record& src, std::size_t expectedSize, unsigned version = Record::kClassVersion)
{
    const auto computedSize = src.getSerializedSize(version);
    ASSERT_EQ(computedSize, expectedSize);

    stdext::buffer<std::uint8_t> buffer(computedSize + kExtraBufferSize);
    const auto end = src.serializeUnchecked(buffer.data(), version);
    const auto actualSize = static_cast<std::size_t>(end - buffer.data());
    ASSERT_EQ(actualSize, computedSize);

    Record dest;
    dest.deserialize(buffer.data(), computedSize);
    ASSERT_EQ(dest, src);
}

template<class Record>
void checkEmptyRecord(std::size_t expectedSize, unsigned version = Record::kClassVersion)
{
    const Record r;
    checkRecord(r, expectedSize, version);
}

}  // namespace

TEST(Serialization, CheckUuidUniqueness)
{
    dbengine::checkRegistryRecordUuids();
}

TEST(Serialization, ColumnDefinitionConstraintRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 20;
    checkEmptyRecord<dbengine::ColumnDefinitionConstraintRecord>(kSerializedSize);
}

TEST(Serialization, Filled_ColumnDefinitionConstraintRecord)
{
    constexpr std::size_t kSerializedSize = 26;
    const dbengine::ColumnDefinitionConstraintRecord record(0x100, 0x10000, 0x1000000);
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, ColumnDefinitionRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 20;
    checkEmptyRecord<dbengine::ColumnDefinitionRecord>(kSerializedSize);
}

TEST(Serialization, Filled1_ColumnDefinitionRecord)
{
    constexpr std::size_t kSerializedSize = 23;
    const dbengine::ColumnDefinitionRecord record(0x100, 0x10000);
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, Filled2_ColumnDefinitionRecord)
{
    constexpr std::size_t kSerializedSize = 23;
    const dbengine::ColumnDefinitionRecord record(0x100, 0x10000);
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, ColumnSetColumnRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 21;
    checkEmptyRecord<dbengine::ColumnSetColumnRecord>(kSerializedSize);
}

TEST(Serialization, ColumnSetColumnRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 31;
    const dbengine::ColumnSetColumnRecord record(0x100, 0x10000, 0x1000000, 0x100000000ULL);
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, ColumnSetRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 20;
    checkEmptyRecord<dbengine::ColumnSetRecord>(kSerializedSize);
}

TEST(Serialization, ColumnSetRecord_Filled1)
{
    constexpr std::size_t kSerializedSize = 23;
    const dbengine::ColumnSetRecord record(0x100, 0x10000);
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, ColumnSetRecord_Filled2)
{
    constexpr std::size_t kSerializedSize = 100;
    dbengine::ColumnSetColumnRegistry columns;
    columns.emplace(0x1, 0x1, 0x1, 0x1);
    columns.emplace(0x100, 0x100, 0x100, 0x100);
    columns.emplace(0x101, 0x10000, 0x1000000, 0x100000000ULL);
    const dbengine::ColumnSetRecord record(0x100, 0x10000, std::move(columns));
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, ConstraintRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 24;
    checkEmptyRecord<dbengine::ConstraintRecord>(kSerializedSize);
}

TEST(Serialization, ConstraintRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 60;
    const dbengine::ConstraintRecord record(0x100, "some_name", dbengine::ConstraintState::kActive,
            0x10000, 0x1000000, 0x100000000ULL, "some_description");
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, DatabaseRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 37;
    checkEmptyRecord<dbengine::DatabaseRecord>(kSerializedSize);
}

TEST(Serialization, DatabaseRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 59;
    const dbengine::DatabaseRecord record(0x100,
            boost::lexical_cast<siodb::Uuid>("0dfee496-6700-4c73-abab-13ac0a154306"), "db1",
            "aes128", "my database");
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, IndexColumnRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 21;
    checkEmptyRecord<dbengine::IndexColumnRecord>(kSerializedSize);
}

TEST(Serialization, IndexColumnRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 27;
    const dbengine::IndexColumnRecord record(0x100, 0x10000, 0x1000000, true);
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, IndexRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 28;
    checkEmptyRecord<dbengine::IndexRecord>(kSerializedSize);
}

TEST(Serialization, IndexRecord_Filled1)
{
    constexpr std::size_t kSerializedSize = 47;
    dbengine::IndexColumnRegistry columns;
    const dbengine::IndexRecord record(0x100, dbengine::IndexType::kLinearIndexU64, 0x10000, true,
            "index1", std::move(columns), siodb::kDefaultDataFileSize << 8, "my index");
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, IndexRecord_Filled2)
{
    constexpr std::size_t kSerializedSize = 119;
    dbengine::IndexColumnRegistry columns;
    columns.emplace(0x1, 0x1, 0x1, false);
    columns.emplace(0x100, 0x100, 0x100, true);
    columns.emplace(0x10000, 0x10000, 0x10000, true);
    const dbengine::IndexRecord record(0x100, dbengine::IndexType::kLinearIndexU64, 0x10000, true,
            "index1", std::move(columns), siodb::kDefaultDataFileSize << 8, "my index");
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, TableRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 23;
    checkEmptyRecord<dbengine::TableRecord>(kSerializedSize);
}

TEST(Serialization, TableRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 44;
    const dbengine::TableRecord record(
            0x100, dbengine::TableType::kMemory, "table1", 0x10000, 0x1000000, "my table");
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, UserAccessKeyRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 23;
    checkEmptyRecord<dbengine::UserAccessKeyRecord>(kSerializedSize);
}

TEST(Serialization, UserAccessKeyRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 138;
    const dbengine::UserAccessKeyRecord record(0x100, 0x10000, "user1-key1",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9UkQ "
            "user1@host",
            "my ssh key", true);
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, UserTokenRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 23;
    checkEmptyRecord<dbengine::UserTokenRecord>(kSerializedSize);
}

TEST(Serialization, UserTokenRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 64;
    const dbengine::UserTokenRecord record(0x100, 0x10000, "user1-token1",
            siodb::BinaryValue {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}, 1,
            "my token");
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, UserPermissionRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 24;
    checkEmptyRecord<dbengine::UserPermissionRecord>(kSerializedSize);
}

TEST(Serialization, UserPermissionRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 36;
    const dbengine::UserPermissionRecord record(0x100, 0x10000, 0x1000000,
            dbengine::DatabaseObjectType::kTable, 0x100000000ULL, 0x1fff, 0x1fff);
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, UserRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 24;
    checkEmptyRecord<dbengine::UserRecord>(kSerializedSize);
}

TEST(Serialization, UserRecord_Filled1)
{
    constexpr std::size_t kSerializedSize = 50;
    const dbengine::UserRecord record(0x100, "user1", "John Doe", "first user", true, {}, {});
    checkRecord(record, kSerializedSize);
}

TEST(Serialization, UserRecord_Filled2)
{
    constexpr std::size_t kSerializedSize = 484;

    dbengine::UserAccessKeyRegistry userAccessKeys;
    userAccessKeys.emplace(0x100, 0x10000, "user1-key1",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9UkQ "
            "user1@host",
            "my ssh key 1 xx", true);
    userAccessKeys.emplace(0x101, 0x10000, "user1-key2",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9UkX "
            "user1@host2",
            "my ssh key 2 yyy", true);
    userAccessKeys.emplace(0x102, 0x10000, "user1-key3",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9Uke "
            "user1@host3",
            "my ssh key 3 zzzz", true);

    dbengine::UserTokenRegistry userTokens;

    const dbengine::UserRecord record(0x100, "user1", "John Doe", "first user", true,
            std::move(userAccessKeys), std::move(userTokens));

    checkRecord(record, kSerializedSize);
}

TEST(Serialization, UserRecord_Filled3)
{
    constexpr std::size_t kSerializedSize = 547;

    dbengine::UserAccessKeyRegistry userAccessKeys;
    userAccessKeys.emplace(0x100, 0x10000, "user1-key1",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9UkQ "
            "user1@host",
            "my ssh key 1 xx", true);
    userAccessKeys.emplace(0x101, 0x10000, "user1-key2",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9UkX "
            "user1@host2",
            "my ssh key 2 yyy", true);
    userAccessKeys.emplace(0x102, 0x10000, "user1-key3",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9Uke "
            "user1@host3",
            "my ssh key 3 zzzz", true);

    dbengine::UserTokenRegistry userTokens;
    userTokens.emplace(0x100, 0x10000, "user1-token1",
            siodb::BinaryValue {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
            std::nullopt, "my token");

    const dbengine::UserRecord record(0x100, "user1", "John Doe", "first user", true,
            std::move(userAccessKeys), std::move(userTokens));

    checkRecord(record, kSerializedSize);
}

TEST(Serialization, UserRecord_Filled4)
{
    constexpr std::size_t kSerializedSize = 629;

    dbengine::UserAccessKeyRegistry userAccessKeys;
    userAccessKeys.emplace(0x100, 0x10000, "user1-key1",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9UkQ "
            "user1@host",
            "my ssh key 1 xx", true);
    userAccessKeys.emplace(0x101, 0x10000, "user1-key2",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9UkX "
            "user1@host2",
            "my ssh key 2 yyy", true);
    userAccessKeys.emplace(0x102, 0x10000, "user1-key3",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9Uke "
            "user1@host3",
            "my ssh key 3 zzzz", true);

    dbengine::UserTokenRegistry userTokens;
    userTokens.emplace(0x100, 0x10000, "user1-token1",
            siodb::BinaryValue {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
            std::nullopt, "my token");
    userTokens.emplace(0x101, 0x10000, "user2-token2",
            siodb::BinaryValue {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 1, 2, 3, 4,
                    5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
            1, "my token 2");

    const dbengine::UserRecord record(0x100, "user1", "John Doe", "first user", true,
            std::move(userAccessKeys), std::move(userTokens));

    checkRecord(record, kSerializedSize);
}

TEST(Serialization, CipherKeyRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 20;
    checkEmptyRecord<dbengine::CipherKeyRecord>(kSerializedSize);
}

TEST(Serialization, CipherKeyRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 43;
    const dbengine::CipherKeyRecord record(128U, "aes128",
            siodb::BinaryValue {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
    checkRecord(record, kSerializedSize);
}
