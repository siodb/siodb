// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/utils/UnorderedLruCache.h>

// STL headers
#include <iostream>

// Google Test
#include <gtest/gtest.h>

namespace {

class SampleLruCache : public siodb::utils::unordered_lru_cache<std::size_t, std::size_t> {
private:
    using Base = siodb::utils::unordered_lru_cache<std::size_t, std::size_t>;

public:
    static constexpr std::size_t kCapacity = 10;

    SampleLruCache()
        : Base(kCapacity)
    {
    }
};

}  // anonymous namespace

TEST(unordered_lru_cache, Create)
{
    SampleLruCache cache;
    EXPECT_EQ(cache.capacity(), SampleLruCache::kCapacity);
    EXPECT_TRUE(cache.empty());
    EXPECT_EQ(cache.size(), 0U);
}

TEST(unordered_lru_cache, FillBelowCapacity)
{
    SampleLruCache cache;
    for (std::size_t i = 0; i < SampleLruCache::kCapacity - 1; ++i)
        cache.emplace(i, i + 1);
    EXPECT_EQ(cache.capacity(), SampleLruCache::kCapacity);
    EXPECT_FALSE(cache.empty());
    EXPECT_EQ(cache.size(), SampleLruCache::kCapacity - 1);
}

TEST(unordered_lru_cache, FillToCapacity)
{
    SampleLruCache cache;
    for (std::size_t i = 0; i < SampleLruCache::kCapacity; ++i)
        cache.emplace(i, i + 1);
    EXPECT_EQ(cache.capacity(), SampleLruCache::kCapacity);
    EXPECT_FALSE(cache.empty());
    EXPECT_EQ(cache.size(), SampleLruCache::kCapacity);
}

TEST(unordered_lru_cache, FillAboveCapacity)
{
    SampleLruCache cache;
    for (std::size_t i = 0; i < SampleLruCache::kCapacity * 2; ++i)
        cache.emplace(i, i + 1);
    EXPECT_EQ(cache.capacity(), SampleLruCache::kCapacity);
    EXPECT_FALSE(cache.empty());
    EXPECT_EQ(cache.size(), SampleLruCache::kCapacity);
    for (std::size_t i = SampleLruCache::kCapacity; i < SampleLruCache::kCapacity * 2; ++i) {
        ASSERT_TRUE(cache.contains(i));
    }
}

TEST(unordered_lru_cache, Get)
{
    SampleLruCache cache;
    for (std::size_t i = 0; i < SampleLruCache::kCapacity - 1; ++i)
        cache.emplace(i, i + 1);
    for (std::size_t i = 0; i < SampleLruCache::kCapacity - 1; ++i) {
        const auto cachedValue = cache.get(i);
        ASSERT_TRUE(cachedValue);
        EXPECT_EQ(*cachedValue, i + 1);
    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
