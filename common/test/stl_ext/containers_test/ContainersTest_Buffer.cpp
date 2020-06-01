// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/stl_ext/buffer.h>

// CRT headers
#include <cstring>

// STL headers
#include <iostream>

// Google Test
#include <gtest/gtest.h>

namespace {

using Element = int;
using Buffer = stdext::buffer<Element>;

}  // namespace

TEST(Buffer, CreateEmpty)
{
    const Buffer buffer;
    ASSERT_EQ(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 0U);
}

TEST(Buffer, CreateNonEmpty)
{
    constexpr std::size_t kSize = 10;

    const Buffer buffer(kSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kSize);
}

TEST(Buffer, CreateInitialized)
{
    constexpr std::size_t kSize = 10;
    constexpr Element kFill = 0xAB;

    const Buffer buffer(kSize, kFill);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kSize);
    const std::vector<Element> requiredData(kSize, kFill);
    const int res = std::memcmp(buffer.data(), requiredData.data(), sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, CreateFromMemoryRegion)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer(data, kSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kSize);
    const int res = std::memcmp(buffer.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, CreateFromMemoryRegionNoAttach)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};
    std::unique_ptr<Element[]> mem(new Element[kSize]);
    std::memcpy(mem.get(), data, sizeof(Element) * kSize);

    auto p = mem.get();
    const Buffer buffer(p, kSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kSize);
    ASSERT_NE(buffer.data(), mem.get());
    const int res = std::memcmp(buffer.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, CreateFromMemoryRegionAttach)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};
    std::unique_ptr<Element[]> mem(new Element[kSize]);
    std::memcpy(mem.get(), data, sizeof(Element) * kSize);

    auto p = mem.release();
    const Buffer buffer(p, kSize, true);
    ASSERT_EQ(buffer.data(), p);
    ASSERT_EQ(buffer.size(), kSize);
}

TEST(Buffer, CreateFromRange)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer(data, data + kSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kSize);
    const int res = std::memcmp(buffer.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, CreateFromInitializerList)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer {0, 1, 2, 3, 4, 5, 0, 0, 0, 0};
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kSize);
    const int res = std::memcmp(buffer.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, CreateCopyOfEmpty)
{
    const Buffer buffer1;
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);

    const Buffer buffer2(buffer1);
    ASSERT_EQ(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 0U);
}

TEST(Buffer, CreateCopyOfNonEmpty)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer1(data, data + kSize);
    ASSERT_NE(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), kSize);
    int res = std::memcmp(buffer1.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    const Buffer buffer2(buffer1);
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_NE(buffer2.data(), buffer1.data());
    ASSERT_EQ(buffer2.size(), buffer1.size());
    res = std::memcmp(buffer1.data(), buffer2.data(), sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, CreateMoveEmpty)
{
    Buffer buffer1;
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);

    const Buffer buffer2(std::move(buffer1));
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);
    ASSERT_EQ(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 0U);
}

TEST(Buffer, CreateMoveNonEmpty)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    Buffer buffer1(data, data + kSize);
    ASSERT_NE(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), kSize);
    int res = std::memcmp(buffer1.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    const auto* b1data = buffer1.data();
    const auto b1size = buffer1.size();
    const Buffer buffer2(std::move(buffer1));
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);
    ASSERT_EQ(buffer2.data(), b1data);
    ASSERT_EQ(buffer2.size(), b1size);
}

TEST(Buffer, CopyAssignEmpty)
{
    const Buffer buffer1;
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);

    Buffer buffer2 {1, 2};
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 2U);

    buffer2 = buffer1;
    ASSERT_EQ(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 0U);
}

TEST(Buffer, CopyAssignOfNonEmpty)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer1(data, data + kSize);
    ASSERT_NE(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), kSize);
    int res = std::memcmp(buffer1.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    Buffer buffer2 {1, 2};
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 2U);

    buffer2 = buffer1;
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_NE(buffer2.data(), buffer1.data());
    ASSERT_EQ(buffer2.size(), buffer1.size());
    res = std::memcmp(buffer1.data(), buffer2.data(), sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, MoveAssignEmpty)
{
    Buffer buffer1;
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);

    Buffer buffer2 {1, 2};
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 2U);

    buffer2 = std::move(buffer1);
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);
    ASSERT_EQ(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 0U);
}

TEST(Buffer, MoveAssignNonEmpty)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    Buffer buffer1(data, data + kSize);
    ASSERT_NE(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), kSize);
    int res = std::memcmp(buffer1.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    Buffer buffer2 {1, 2};
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 2U);

    const auto* b1data = buffer1.data();
    const auto b1size = buffer1.size();
    buffer2 = std::move(buffer1);
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);
    ASSERT_EQ(buffer2.data(), b1data);
    ASSERT_EQ(buffer2.size(), b1size);
}

TEST(Buffer, EmptyEqual)
{
    const Buffer buffer1;
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);

    Buffer buffer2 {1, 2};
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 2U);

    buffer2 = buffer1;
    ASSERT_EQ(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 0U);

    ASSERT_TRUE(buffer2 == buffer1);
}

TEST(Buffer, NonEmptySameSizeEqual)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer1(data, data + kSize);
    ASSERT_NE(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), kSize);
    int res = std::memcmp(buffer1.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    Buffer buffer2 {1, 2};
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 2U);

    buffer2 = buffer1;
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_NE(buffer2.data(), buffer1.data());
    ASSERT_EQ(buffer2.size(), buffer1.size());
    res = std::memcmp(buffer1.data(), buffer2.data(), sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    ASSERT_TRUE(buffer2 == buffer1);
}

TEST(Buffer, EmptyNotNonEqual)
{
    const Buffer buffer1;
    ASSERT_EQ(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), 0U);

    Buffer buffer2 {1, 2};
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 2U);

    buffer2 = buffer1;
    ASSERT_EQ(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 0U);

    ASSERT_FALSE(buffer2 != buffer1);
}

TEST(Buffer, NonEmptySameSizeNotNonEqual)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer1(data, data + kSize);
    ASSERT_NE(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), kSize);
    int res = std::memcmp(buffer1.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    Buffer buffer2 {1, 2};
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 2U);

    buffer2 = buffer1;
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_NE(buffer2.data(), buffer1.data());
    ASSERT_EQ(buffer2.size(), buffer1.size());
    res = std::memcmp(buffer1.data(), buffer2.data(), sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    ASSERT_FALSE(buffer2 != buffer1);
}

TEST(Buffer, NonEmptyDifferentSizeNonEqual)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer1(data, data + kSize);
    ASSERT_NE(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), kSize);
    int res = std::memcmp(buffer1.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    const Buffer buffer2 {1, 2};
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), 2U);

    ASSERT_TRUE(buffer2 != buffer1);
}

TEST(Buffer, NonEmptySameSizeNonEqual)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer1(data, data + kSize);
    ASSERT_NE(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), kSize);
    int res = std::memcmp(buffer1.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    Buffer buffer2(buffer1);
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_NE(buffer2.data(), buffer1.data());
    ASSERT_EQ(buffer2.size(), buffer1.size());

    buffer2[0] = 0xFF;
    res = std::memcmp(buffer1.data(), buffer2.data(), sizeof(Element) * kSize);
    ASSERT_NE(res, 0);

    ASSERT_TRUE(buffer2 != buffer1);
}

TEST(Buffer, CompareSameSize)
{
    constexpr std::size_t kSize = 10;
    const int data[kSize] = {0, 1, 2, 3, 4, 5};

    const Buffer buffer1(data, data + kSize);
    ASSERT_NE(buffer1.data(), nullptr);
    ASSERT_EQ(buffer1.size(), kSize);
    int res = std::memcmp(buffer1.data(), data, sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    Buffer buffer2(buffer1);
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_EQ(buffer2.size(), buffer1.size());

    buffer2[0] = 0xFF;
    ASSERT_NE(buffer2.data(), nullptr);
    ASSERT_NE(buffer2.data(), buffer1.data());
    ASSERT_EQ(buffer2.size(), buffer1.size());
    res = std::memcmp(buffer1.data(), buffer2.data(), sizeof(Element) * kSize);
    ASSERT_NE(res, 0);

    ASSERT_TRUE(buffer1 < buffer2);
    ASSERT_FALSE(buffer2 < buffer1);

    ASSERT_TRUE(buffer1 <= buffer2);
    ASSERT_FALSE(buffer2 <= buffer1);

    ASSERT_TRUE(buffer2 > buffer1);
    ASSERT_FALSE(buffer1 > buffer2);

    ASSERT_TRUE(buffer2 >= buffer1);
    ASSERT_FALSE(buffer1 >= buffer2);

    buffer2[0] = buffer1[0];

    ASSERT_FALSE(buffer1 < buffer2);
    ASSERT_FALSE(buffer2 < buffer1);

    ASSERT_TRUE(buffer1 <= buffer2);
    ASSERT_TRUE(buffer2 <= buffer1);

    ASSERT_FALSE(buffer2 > buffer1);
    ASSERT_FALSE(buffer1 > buffer2);

    ASSERT_TRUE(buffer2 >= buffer1);
    ASSERT_TRUE(buffer1 >= buffer2);
}

TEST(Buffer, CompareDifferentSize)
{
    constexpr std::size_t kSize1 = 5;
    const int data1[kSize1] = {1, 2, 3, 4, 5};
    constexpr std::size_t kSize2 = 10;
    const int data2[kSize2] = {1, 2, 3, 4, 5};

    const Buffer buffer1(data1, kSize1);
    const Buffer buffer2(data2, kSize2);

    ASSERT_FALSE(buffer1 == buffer2);
    ASSERT_TRUE(buffer1 != buffer2);

    ASSERT_TRUE(buffer1 < buffer2);
    ASSERT_FALSE(buffer2 < buffer1);

    ASSERT_TRUE(buffer2 > buffer1);
    ASSERT_FALSE(buffer1 > buffer2);

    ASSERT_TRUE(buffer1 <= buffer2);
    ASSERT_FALSE(buffer2 <= buffer1);

    ASSERT_TRUE(buffer2 >= buffer1);
    ASSERT_FALSE(buffer1 >= buffer2);
}

TEST(Buffer, SelfComparison)
{
    const Buffer buffer {1, 2};
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 2U);

    ASSERT_TRUE(buffer == buffer);
    ASSERT_FALSE(buffer != buffer);
    ASSERT_FALSE(buffer > buffer);
    ASSERT_TRUE(buffer >= buffer);
    ASSERT_FALSE(buffer < buffer);
    ASSERT_TRUE(buffer <= buffer);
}

TEST(Buffer, Fill)
{
    Buffer buffer {1, 2, 3, 4, 5};
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 5U);

    constexpr Element kFill = 0xCAFEBABE;
    const std::vector<Element> requiredData(buffer.size(), kFill);

    buffer.fill(kFill);
    const int res =
            std::memcmp(buffer.data(), requiredData.data(), sizeof(Element) * buffer.size());
    ASSERT_EQ(res, 0);
}

TEST(Buffer, Clear)
{
    Buffer buffer {1, 2, 3, 4, 5};
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 5U);

    buffer.clear();
    ASSERT_EQ(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 0U);
}

TEST(Buffer, ResizeFromEmptyToNonEmptyUninitialized)
{
    Buffer buffer;
    ASSERT_EQ(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 0U);

    constexpr std::size_t kSize = 10;
    buffer.resize(kSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kSize);
}

TEST(Buffer, ResizeFromEmptyToNonEmptyInitialized)
{
    Buffer buffer;
    ASSERT_EQ(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 0U);

    constexpr std::size_t kSize = 10;
    constexpr Element kFill = 0xCAFEBABE;
    buffer.resize(kSize, kFill);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kSize);

    const std::vector<Element> requiredData(kSize, kFill);
    const int res =
            std::memcmp(buffer.data(), requiredData.data(), sizeof(Element) * buffer.size());
    ASSERT_EQ(res, 0);
}

TEST(Buffer, ResizeFromNonEmptyToEmpty)
{
    Buffer buffer {1, 2, 3, 4, 5};
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 5U);

    buffer.resize(0);
    ASSERT_EQ(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 0U);
}

TEST(Buffer, ResizeFromNonEmptyToLessSize)
{
    constexpr std::size_t kSize = 5;
    const Element data[kSize] = {1, 2, 3, 4, 5};
    Buffer buffer(data, data + kSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 5U);

    constexpr auto kNewSize = kSize - 2;
    buffer.resize(kNewSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kNewSize);

    const int res = std::memcmp(data, buffer.data(), sizeof(Element) * kNewSize);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, ResizeFromNonEmptyToGreaterSize)
{
    constexpr std::size_t kSize = 5;
    const Element data[kSize] = {1, 2, 3, 4, 5};
    Buffer buffer(data, data + kSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 5U);

    constexpr auto kNewSize = kSize + 2;
    buffer.resize(kNewSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kNewSize);

    const int res = std::memcmp(data, buffer.data(), sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, ResizeFromNonEmptyToGreaterSizeWithFill)
{
    constexpr std::size_t kSize = 5;
    const Element data[kSize] = {1, 2, 3, 4, 5};
    Buffer buffer(data, data + kSize);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 5U);

    constexpr std::size_t kGrowth = 5;
    constexpr auto kNewSize = kSize + kGrowth;
    constexpr Element kFill = 0xCAFEBABE;
    buffer.resize(kNewSize, kFill);
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), kNewSize);

    int res = std::memcmp(data, buffer.data(), sizeof(Element) * kSize);
    ASSERT_EQ(res, 0);

    const std::vector<Element> requiredData(kGrowth, kFill);
    res = std::memcmp(requiredData.data(), buffer.data() + kSize, sizeof(Element) * kGrowth);
    ASSERT_EQ(res, 0);
}

TEST(Buffer, MutableIterators)
{
    Buffer buffer {1, 2, 3, 4, 5};
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 5U);

    const auto itBegin = buffer.begin();
    ASSERT_EQ(itBegin, buffer.data());

    const auto itEnd = buffer.end();
    ASSERT_EQ(itEnd, buffer.data() + buffer.size());
}

TEST(Buffer, ConstIterators)
{
    const Buffer buffer {1, 2, 3, 4, 5};
    ASSERT_NE(buffer.data(), nullptr);
    ASSERT_EQ(buffer.size(), 5U);

    const auto itBegin1 = buffer.begin();
    ASSERT_EQ(itBegin1, buffer.data());

    const auto itBegin2 = buffer.cbegin();
    ASSERT_EQ(itBegin2, buffer.data());

    const auto itEnd1 = buffer.end();
    ASSERT_EQ(itEnd1, buffer.data() + buffer.size());

    const auto itEnd2 = buffer.cend();
    ASSERT_EQ(itEnd2, buffer.data() + buffer.size());
}

TEST(Buffer, Swap)
{
    Buffer buffer1 {1, 2, 3, 4, 5};
    Buffer buffer2 {1, 2, 3, 4, 5};

    const auto b1data = buffer1.data();
    const auto b2data = buffer2.data();

    buffer1.swap(buffer2);
    ASSERT_EQ(buffer1.data(), b2data);
    ASSERT_EQ(buffer2.data(), b1data);
}

TEST(Buffer, ExternalSwap)
{
    Buffer buffer1 {1, 2, 3, 4, 5};
    Buffer buffer2 {1, 2, 3, 4, 5};

    const auto b1data = buffer1.data();
    const auto b2data = buffer2.data();

    std::swap(buffer1, buffer2);
    ASSERT_EQ(buffer1.data(), b2data);
    ASSERT_EQ(buffer2.data(), b1data);
}
