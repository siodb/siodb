// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/stl_ext/lru_cache.h>

// STL headers
#include <iostream>

// Google Test
#include <gtest/gtest.h>

namespace {

class sample_lru_cache : public stdext::unordered_lru_cache<std::size_t, std::size_t> {
private:
    using Base = stdext::unordered_lru_cache<std::size_t, std::size_t>;

public:
    static constexpr std::size_t kCapacity = 10;

public:
    sample_lru_cache() noexcept
        : Base(kCapacity)
    {
    }
};

}  // anonymous namespace

TEST(LruCache, Create)
{
    sample_lru_cache cache;
    EXPECT_EQ(cache.capacity(), sample_lru_cache::kCapacity);
    EXPECT_TRUE(cache.empty());
    EXPECT_EQ(cache.size(), 0U);
}

TEST(LruCache, FillBelowCapacity)
{
    sample_lru_cache cache;

    for (std::size_t i = 0; i < sample_lru_cache::kCapacity - 1; ++i)

        cache.emplace(i, i + 1);
    EXPECT_EQ(cache.capacity(), sample_lru_cache::kCapacity);
    EXPECT_FALSE(cache.empty());
    EXPECT_EQ(cache.size(), sample_lru_cache::kCapacity - 1);
}

TEST(LruCache, FillToCapacity)
{
    sample_lru_cache cache;

    for (std::size_t i = 0; i < sample_lru_cache::kCapacity; ++i)
        cache.emplace(i, i + 1);

    EXPECT_EQ(cache.capacity(), sample_lru_cache::kCapacity);
    EXPECT_FALSE(cache.empty());
    EXPECT_EQ(cache.size(), sample_lru_cache::kCapacity);
}

TEST(LruCache, FillAboveCapacity)
{
    sample_lru_cache cache;

    for (std::size_t i = 0; i < sample_lru_cache::kCapacity * 2; ++i)
        cache.emplace(i, i + 1);

    EXPECT_EQ(cache.capacity(), sample_lru_cache::kCapacity);
    EXPECT_FALSE(cache.empty());
    EXPECT_EQ(cache.size(), sample_lru_cache::kCapacity);

    for (std::size_t i = sample_lru_cache::kCapacity; i < sample_lru_cache::kCapacity * 2; ++i) {
        ASSERT_TRUE(cache.contains(i));
    }
}

TEST(LruCache, Get)
{
    sample_lru_cache cache;

    for (std::size_t i = 0; i < sample_lru_cache::kCapacity - 1; ++i)
        cache.emplace(i, i + 1);

    for (std::size_t i = 0; i < sample_lru_cache::kCapacity - 1; ++i) {
        const auto cachedValue = cache.get(i);
        ASSERT_TRUE(cachedValue);
        EXPECT_EQ(*cachedValue, i + 1);
    }
}
