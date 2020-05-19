// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/utils/PlainBinaryEncoding.h>

// CRT headers
#include <cstring>

// STL headers
#include <iostream>

// Google Test
#include <gtest/gtest.h>

TEST(PlainBinaryEncoding, EncodeInt16)
{
    std::uint8_t buffer[16];
    std::memset(buffer, 0, sizeof(buffer));
    const std::int16_t originalValue = 0x1234;
    const auto res = ::pbeEncodeInt16(originalValue, buffer);
    ASSERT_EQ(res, buffer + 2);
    ASSERT_EQ(buffer[0], 0x34);
    ASSERT_EQ(buffer[1], 0x12);
}

TEST(PlainBinaryEncoding, EncodeUInt16)
{
    std::uint8_t buffer[16];
    std::memset(buffer, 0, sizeof(buffer));
    const std::uint16_t originalValue = 0x1234;
    const auto res = ::pbeEncodeUInt16(originalValue, buffer);
    ASSERT_EQ(res, buffer + 2);
    ASSERT_EQ(buffer[0], 0x34);
    ASSERT_EQ(buffer[1], 0x12);
}

TEST(PlainBinaryEncoding, EncodeInt32)
{
    std::uint8_t buffer[16];
    std::memset(buffer, 0, sizeof(buffer));
    const std::int32_t originalValue = 0x12345678;
    const auto res = ::pbeEncodeInt32(originalValue, buffer);
    ASSERT_EQ(res, buffer + 4);
    ASSERT_EQ(buffer[0], 0x78);
    ASSERT_EQ(buffer[1], 0x56);
    ASSERT_EQ(buffer[2], 0x34);
    ASSERT_EQ(buffer[3], 0x12);
}

TEST(PlainBinaryEncoding, EncodeUInt32)
{
    std::uint8_t buffer[16];
    std::memset(buffer, 0, sizeof(buffer));
    const std::uint32_t originalValue = 0x12345678;
    const auto res = ::pbeEncodeUInt32(originalValue, buffer);
    ASSERT_EQ(res, buffer + 4);
    ASSERT_EQ(buffer[0], 0x78);
    ASSERT_EQ(buffer[1], 0x56);
    ASSERT_EQ(buffer[2], 0x34);
    ASSERT_EQ(buffer[3], 0x12);
}

TEST(PlainBinaryEncoding, EncodeInt64)
{
    std::uint8_t buffer[16];
    std::memset(buffer, 0, sizeof(buffer));
    const std::int64_t originalValue = 0x123456789abcdef5;
    const auto res = ::pbeEncodeInt64(originalValue, buffer);
    ASSERT_EQ(res, buffer + 8);
    ASSERT_EQ(buffer[0], 0xf5);
    ASSERT_EQ(buffer[1], 0xde);
    ASSERT_EQ(buffer[2], 0xbc);
    ASSERT_EQ(buffer[3], 0x9a);
    ASSERT_EQ(buffer[4], 0x78);
    ASSERT_EQ(buffer[5], 0x56);
    ASSERT_EQ(buffer[6], 0x34);
    ASSERT_EQ(buffer[7], 0x12);
}

TEST(PlainBinaryEncoding, EncodeUInt64)
{
    std::uint8_t buffer[16];
    std::memset(buffer, 0, sizeof(buffer));
    const std::uint64_t originalValue = 0x123456789abcdef5;
    const auto res = ::pbeEncodeUInt64(originalValue, buffer);
    ASSERT_EQ(res, buffer + 8);
    ASSERT_EQ(buffer[0], 0xf5);
    ASSERT_EQ(buffer[1], 0xde);
    ASSERT_EQ(buffer[2], 0xbc);
    ASSERT_EQ(buffer[3], 0x9a);
    ASSERT_EQ(buffer[4], 0x78);
    ASSERT_EQ(buffer[5], 0x56);
    ASSERT_EQ(buffer[6], 0x34);
    ASSERT_EQ(buffer[7], 0x12);
}

TEST(PlainBinaryEncoding, DecodeInt16)
{
    const int16_t originalValue = 0x1234;
    const std::uint8_t buffer[16] {0x34, 0x12};
    std::int16_t decodedValue = 0;
    const auto res = ::pbeDecodeInt16(buffer, &decodedValue);
    ASSERT_EQ(res, buffer + 2);
    ASSERT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, DecodeUInt16)
{
    const uint16_t originalValue = 0x1234;
    const std::uint8_t buffer[16] {0x34, 0x12};
    std::uint16_t decodedValue = 0;
    const auto res = ::pbeDecodeUInt16(buffer, &decodedValue);
    ASSERT_EQ(res, buffer + 2);
    ASSERT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, DecodeInt32)
{
    const int32_t originalValue = 0x12345678;
    const std::uint8_t buffer[16] {0x78, 0x56, 0x34, 0x12};
    std::int32_t decodedValue = 0;
    const auto res = ::pbeDecodeInt32(buffer, &decodedValue);
    ASSERT_EQ(res, buffer + 4);
    ASSERT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, DecodeUInt32)
{
    const uint32_t originalValue = 0x12345678;
    const std::uint8_t buffer[16] {0x78, 0x56, 0x34, 0x12};
    std::uint32_t decodedValue = 0;
    const auto res = ::pbeDecodeUInt32(buffer, &decodedValue);
    ASSERT_EQ(res, buffer + 4);
    ASSERT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, DecodeInt64)
{
    const int64_t originalValue = 0x123456789abcdef5;
    const std::uint8_t buffer[16] {0xf5, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12};
    std::int64_t decodedValue = 0;
    const auto res = ::pbeDecodeInt64(buffer, &decodedValue);
    std::cout << std::hex << decodedValue << std::endl;
    ASSERT_EQ(res, buffer + 8);
    ASSERT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, DecodeUInt64)
{
    const uint64_t originalValue = 0x123456789abcdef5;
    const std::uint8_t buffer[16] {0xf5, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12};
    std::uint64_t decodedValue = 0;
    const auto res = ::pbeDecodeUInt64(buffer, &decodedValue);
    std::cout << std::hex << decodedValue << std::endl;
    ASSERT_EQ(res, buffer + 8);
    ASSERT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, EncodeDecodeInt16)
{
    const std::int16_t originalValue = 0x1234;
    std::uint8_t buffer[16];
    ::pbeEncodeInt16(originalValue, buffer);
    std::int16_t decodedValue = 0;
    ::pbeDecodeInt16(buffer, &decodedValue);
    EXPECT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, EncodeDecodeUInt16)
{
    const std::uint16_t originalValue = 0x1234;
    std::uint8_t buffer[16];
    ::pbeEncodeUInt16(originalValue, buffer);
    std::uint16_t decodedValue = 0;
    ::pbeDecodeUInt16(buffer, &decodedValue);
    EXPECT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, EncodeDecodeInt32)
{
    const std::int32_t originalValue = 0x12345678;
    std::uint8_t buffer[16];
    ::pbeEncodeInt32(originalValue, buffer);
    std::int32_t decodedValue = 0;
    ::pbeDecodeInt32(buffer, &decodedValue);
    EXPECT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, EncodeDecodeUInt32)
{
    const std::uint32_t originalValue = 0x12345678;
    std::uint8_t buffer[16];
    ::pbeEncodeUInt32(originalValue, buffer);
    std::uint32_t decodedValue = 0;
    ::pbeDecodeUInt32(buffer, &decodedValue);
    EXPECT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, EncodeDecodeInt64)
{
    const std::int64_t originalValue = 0x123456789abcdef5;
    std::uint8_t buffer[16];
    ::pbeEncodeInt64(originalValue, buffer);
    std::int64_t decodedValue = 0;
    ::pbeDecodeInt64(buffer, &decodedValue);
    EXPECT_EQ(decodedValue, originalValue);
}

TEST(PlainBinaryEncoding, EncodeDecodeUInt64)
{
    const std::uint64_t originalValue = 0x123456789abcdef5;
    std::uint8_t buffer[16];
    ::pbeEncodeUInt64(originalValue, buffer);
    std::uint64_t decodedValue = 0;
    ::pbeDecodeUInt64(buffer, &decodedValue);
    EXPECT_EQ(decodedValue, originalValue);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
