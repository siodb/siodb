// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/reg/ColumnDefinitionRecord.h"
#include "dbengine/reg/ColumnSetRecord.h"
#include "dbengine/reg/ConstraintRecord.h"
#include "dbengine/reg/DatabaseRecord.h"
#include "dbengine/reg/IndexRecord.h"
#include "dbengine/reg/TableRecord.h"
#include "dbengine/reg/UserAccessKeyRecord.h"
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
    const auto size = src.getSerializedSize(version);
    ASSERT_EQ(size, expectedSize);

    stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
    const auto end = src.serializeUnchecked(buffer.data(), version);
    const auto actualSize = static_cast<std::size_t>(end - buffer.data());
    ASSERT_EQ(actualSize, size);

    Record dest;
    dest.deserialize(buffer.data(), size);
    ASSERT_EQ(dest, src);
}

template<class Record>
void checkEmptyRecord(std::size_t expectedSize, unsigned version = Record::kClassVersion)
{
    const Record r;
    checkRecord(r, expectedSize, version);
}

}  // namespace

TEST(Serialization, ColumnDefinitionConstraintRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 20;
    checkEmptyRecord<dbengine::ColumnDefinitionConstraintRecord>(kSerializedSize);
}

TEST(Serialization, Filled_ColumnDefinitionConstraintRecord)
{
    constexpr std::size_t kSerializedSize = 26;
    const dbengine::ColumnDefinitionConstraintRecord src(0x100, 0x10000, 0x1000000);
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, ColumnDefinitionRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 20;
    checkEmptyRecord<dbengine::ColumnDefinitionRecord>(kSerializedSize);
}

TEST(Serialization, Filled1_ColumnDefinitionRecord)
{
    constexpr std::size_t kSerializedSize = 23;
    const dbengine::ColumnDefinitionRecord src(0x100, 0x10000);
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, Filled2_ColumnDefinitionRecord)
{
    constexpr std::size_t kSerializedSize = 23;
    const dbengine::ColumnDefinitionRecord src(0x100, 0x10000);
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, ColumnSetColumnRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 21;
    checkEmptyRecord<dbengine::ColumnSetColumnRecord>(kSerializedSize);
}

TEST(Serialization, ColumnSetColumnRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 31;
    const dbengine::ColumnSetColumnRecord src(0x100, 0x10000, 0x1000000, 0x100000000ULL);
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, ColumnSetRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 20;
    checkEmptyRecord<dbengine::ColumnSetRecord>(kSerializedSize);
}

TEST(Serialization, ColumnSetRecord_Filled1)
{
    constexpr std::size_t kSerializedSize = 23;
    const dbengine::ColumnSetRecord src(0x100, 0x10000);
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, ColumnSetRecord_Filled2)
{
    constexpr std::size_t kSerializedSize = 100;
    dbengine::ColumnSetColumnRegistry columns;
    columns.emplace(0x1, 0x1, 0x1, 0x1);
    columns.emplace(0x100, 0x100, 0x100, 0x100);
    columns.emplace(0x101, 0x10000, 0x1000000, 0x100000000ULL);
    const dbengine::ColumnSetRecord src(0x100, 0x10000, std::move(columns));
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, ConstraintRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 24;
    checkEmptyRecord<dbengine::ConstraintRecord>(kSerializedSize);
}

TEST(Serialization, ConstraintRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 60;
    const dbengine::ConstraintRecord src(0x100, "some_name", dbengine::ConstraintState::kActive,
            0x10000, 0x1000000, 0x100000000ULL, "some_description");
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, DatabaseRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 38;
    checkEmptyRecord<dbengine::DatabaseRecord>(kSerializedSize);
}

TEST(Serialization, DatabaseRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 76;
    const dbengine::DatabaseRecord src(0x100,
            boost::lexical_cast<siodb::Uuid>("0dfee496-6700-4c73-abab-13ac0a154306"), "db1",
            "aes128",
            siodb::BinaryValue {
                    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf},
            "my database");
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, IndexColumnRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 21;
    checkEmptyRecord<dbengine::IndexColumnRecord>(kSerializedSize);
}

TEST(Serialization, IndexColumnRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 27;
    const dbengine::IndexColumnRecord src(0x100, 0x10000, 0x1000000, true);
    checkRecord(src, kSerializedSize);
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
    const dbengine::IndexRecord src(0x100, dbengine::IndexType::kLinearIndexU64, 0x10000, true,
            "index1", std::move(columns), siodb::kDefaultDataFileSize << 8, "my index");
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, IndexRecord_Filled2)
{
    constexpr std::size_t kSerializedSize = 119;
    dbengine::IndexColumnRegistry columns;
    columns.emplace(0x1, 0x1, 0x1, false);
    columns.emplace(0x100, 0x100, 0x100, true);
    columns.emplace(0x10000, 0x10000, 0x10000, true);
    const dbengine::IndexRecord src(0x100, dbengine::IndexType::kLinearIndexU64, 0x10000, true,
            "index1", std::move(columns), siodb::kDefaultDataFileSize << 8, "my index");
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, TableRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 23;
    checkEmptyRecord<dbengine::TableRecord>(kSerializedSize);
}

TEST(Serialization, TableRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 44;
    const dbengine::TableRecord src(
            0x100, dbengine::TableType::kMemory, "table1", 0x10000, 0x1000000, "my table");
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, UserAccessKeyRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 23;
    checkEmptyRecord<dbengine::UserAccessKeyRecord>(kSerializedSize);
}

TEST(Serialization, UserAccessKeyRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 138;
    const dbengine::UserAccessKeyRecord src(0x100, 0x10000, "user1-key1",
            "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICl9Vdr42N1wUoNbKO4EfnWi9os98aVe59RZjozI9UkQ "
            "user1@host",
            "my ssh key", true);
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, UserPermissionRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 24;
    checkEmptyRecord<dbengine::UserPermissionRecord>(kSerializedSize);
}

TEST(Serialization, UserPermissionRecord_Filled)
{
    constexpr std::size_t kSerializedSize = 36;
    const dbengine::UserPermissionRecord src(0x100, 0x10000, 0x1000000,
            dbengine::DatabaseObjectType::kTable, 0x100000000ULL, 0x1fff, 0x1fff);
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, UserRecord_Empty)
{
    constexpr std::size_t kSerializedSize = 23;
    checkEmptyRecord<dbengine::UserRecord>(kSerializedSize);
}

TEST(Serialization, UserRecord_Filled1)
{
    constexpr std::size_t kSerializedSize = 49;
    const dbengine::UserRecord src(0x100, "user1", "John Doe", "first user", true, {});
    checkRecord(src, kSerializedSize);
}

TEST(Serialization, UserRecord_Filled2)
{
    constexpr std::size_t kSerializedSize = 483;
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
    const dbengine::UserRecord src(
            0x100, "user1", "John Doe", "first user", true, std::move(userAccessKeys));
    checkRecord(src, kSerializedSize);
}
