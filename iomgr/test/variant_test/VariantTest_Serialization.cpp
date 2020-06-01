// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/Variant.h"
#include "dbengine/lob/BinaryValueBlobStream.h"
#include "dbengine/lob/StringClobStream.h"

// Common project headers
#include <siodb/common/utils/Debug.h>
#include <siodb/common/utils/StringUtils.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;

constexpr std::size_t kExtraBufferSize = 0x10000U;

TEST(Serialization, Null)
{
    constexpr std::size_t kSerializedSize = 1;
    const dbengine::Variant src;
    const auto size = src.getSerializedSize();
    ASSERT_EQ(size, kSerializedSize);

    stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
    const auto end = src.serializeUnchecked(buffer.data());
    const auto actualSize = static_cast<std::size_t>(end - buffer.data());
    ASSERT_EQ(actualSize, size);

    dbengine::Variant dest;
    const auto consumed = dest.deserialize(buffer.data(), buffer.size());
    ASSERT_EQ(consumed, size);
    ASSERT_EQ(dest, src);
}

TEST(Serialization, Int8)
{
    constexpr std::size_t kSerializedSize = 2;

    using VSP = std::pair<std::int8_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kSerializedSize},
            VSP {0xFF, kSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, UInt8)
{
    constexpr std::size_t kSerializedSize = 2;

    using VSP = std::pair<std::uint8_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kSerializedSize},
            VSP {0xFF, kSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, Int16)
{
    constexpr std::size_t kMinSerializedSize = 2;
    constexpr std::size_t kMaxSerializedSize = 4;

    using VSP = std::pair<std::int16_t, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {-1, kMinSerializedSize},
            VSP {0x7FFF, kMaxSerializedSize},
            VSP {0x8000, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, UInt16)
{
    constexpr std::size_t kMinSerializedSize = 2;
    constexpr std::size_t kMaxSerializedSize = 4;

    using VSP = std::pair<std::uint16_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {0xFFFF, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, Int32)
{
    constexpr std::size_t kMinSerializedSize = 2;
    constexpr std::size_t kMaxSerializedSize = 6;

    using VSP = std::pair<std::int32_t, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {-1, kMinSerializedSize},
            VSP {0x7FFFFFFF, kMaxSerializedSize},
            VSP {0x80000000, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, UInt32)
{
    constexpr std::size_t kMinSerializedSize = 2;
    constexpr std::size_t kMaxSerializedSize = 6;

    using VSP = std::pair<std::uint32_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {0xFFFFFFFF, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, Int64)
{
    constexpr std::size_t kMinSerializedSize = 2;
    constexpr std::size_t kMaxSerializedSize = 11;

    using VSP = std::pair<std::int64_t, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {-1, kMinSerializedSize},
            VSP {0x7FFFFFFFFFFFFFFFULL, kMaxSerializedSize},
            VSP {0x8000000000000000ULL, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, UInt64)
{
    constexpr std::size_t kMinSerializedSize = 2;
    constexpr std::size_t kMaxSerializedSize = 11;

    using VSP = std::pair<std::uint64_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {0xFFFFFFFFFFFFFFFFULL, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, Float)
{
    constexpr std::size_t kSerializedSize = 5;

    using VSP = std::pair<float, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {std::numeric_limits<float>::min(), kSerializedSize},
            VSP {-std::numeric_limits<float>::min(), kSerializedSize},
            VSP {std::numeric_limits<float>::max(), kSerializedSize},
            VSP {-std::numeric_limits<float>::max(), kSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, Double)
{
    constexpr std::size_t kSerializedSize = 9;

    using VSP = std::pair<double, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {std::numeric_limits<double>::min(), kSerializedSize},
            VSP {-std::numeric_limits<double>::min(), kSerializedSize},
            VSP {std::numeric_limits<double>::max(), kSerializedSize},
            VSP {-std::numeric_limits<double>::max(), kSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, DateTime)
{
    constexpr std::size_t kDateOnlySerializedSize = 5;

    constexpr std::size_t kDateTimeSerializedSize = 11;
    using VSP = std::pair<siodb::RawDateTime, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::RawDateTime("2020-01-01", "%Y-%m-%d"), kDateOnlySerializedSize},
            VSP {siodb::RawDateTime("2020-01-01 00:00:00", "%Y-%m-%d %H:%M:%S"),
                    kDateOnlySerializedSize},
            VSP {siodb::RawDateTime("2020-01-01 01:01:01", "%Y-%m-%d %H:%M:%S"),
                    kDateTimeSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, String)
{
    using VSP = std::pair<std::string, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::utils::createString(0), 2},
            VSP {siodb::utils::createString(3), 5},
            VSP {siodb::utils::createString(0xFFFF), 0xFFFF + 4},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, Binary)
{
    using VSP = std::pair<siodb::BinaryValue, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::BinaryValue(), 2},
            VSP {siodb::BinaryValue(3), 5},
            VSP {siodb::BinaryValue(0xFFFF), 0xFFFF + 4},
    };

    for (const auto& vsp : valueSizePairs) {
        const dbengine::Variant src(vsp.first);
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest, src);
    }
}

TEST(Serialization, Clob)
{
    using VSP = std::pair<std::string, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::utils::createString(0), 2},
            VSP {siodb::utils::createString(3), 5},
            VSP {siodb::utils::createString(0xFFFF), 0xFFFF + 4},
    };

    for (const auto& vsp : valueSizePairs) {
        dbengine::Variant src(new dbengine::StringClobStream(vsp.first));
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest.getValueType(), src.getValueType());

        const auto srcStr = src.getClob().readAsString(src.getClob().getSize());
        const auto destStr = dest.getClob().readAsString(dest.getClob().getSize());
        ASSERT_EQ(srcStr, destStr);
    }
}

TEST(Serialization, TooLargeClob)
{
    const dbengine::Variant src(new dbengine::StringClobStream(
            siodb::utils::createString(dbengine::Variant::kMaxStringValueLength * 2)));
    const auto size = src.getSerializedSize();
    ASSERT_EQ(size, 2U);

    stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
    EXPECT_THROW(src.serializeUnchecked(buffer.data()), dbengine::VariantSerializationError);
}

TEST(Serialization, Blob)
{
    using VSP = std::pair<siodb::BinaryValue, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::BinaryValue(), 2},
            VSP {siodb::BinaryValue(3), 5},
            VSP {siodb::BinaryValue(0xFFFF), 0xFFFF + 4},
    };

    for (const auto& vsp : valueSizePairs) {
        dbengine::Variant src(new dbengine::BinaryValueBlobStream(vsp.first));
        const auto size = src.getSerializedSize();
        ASSERT_EQ(size, vsp.second);

        stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
        const auto end = src.serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, size);

        dbengine::Variant dest;
        const auto consumed = dest.deserialize(buffer.data(), buffer.size());
        ASSERT_EQ(consumed, size);
        ASSERT_EQ(dest.getValueType(), src.getValueType());

        const auto srcVec = src.getBlob().readAsBinary(src.getBlob().getSize());
        const auto destVec = dest.getBlob().readAsBinary(dest.getBlob().getSize());
        ASSERT_EQ(srcVec, destVec);
    }
}

TEST(Serialization, TooLargeBlob)
{
    const dbengine::Variant src(new dbengine::BinaryValueBlobStream(
            siodb::BinaryValue(dbengine::Variant::kMaxBinaryValueLength * 2)));
    const auto size = src.getSerializedSize();
    ASSERT_EQ(size, 2U);

    stdext::buffer<std::uint8_t> buffer(size + kExtraBufferSize);
    EXPECT_THROW(src.serializeUnchecked(buffer.data()), dbengine::VariantSerializationError);
}

TEST(Serialization, EmptyStringSize)
{
    const std::string s;
    const auto serializedSize = ::getSerializedSize(s);
    ASSERT_EQ(serializedSize, 1U);
}

TEST(Serialization, StringSize)
{
    const std::string s("Hello");
    const auto serializedSize = ::getSerializedSize(s);
    ASSERT_EQ(serializedSize, 6U);
}

TEST(Serialization, EmptyBinarySize)
{
    const siodb::BinaryValue bv;
    const auto serializedSize = ::getSerializedSize(bv);
    ASSERT_EQ(serializedSize, 1U);
}

TEST(Serialization, BinarySize)
{
    const siodb::BinaryValue bv {1, 2, 3, 4};
    const auto serializedSize = ::getSerializedSize(bv);
    ASSERT_EQ(serializedSize, 5U);
}
