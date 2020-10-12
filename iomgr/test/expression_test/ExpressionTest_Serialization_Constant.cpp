// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "ExpressionFactories.h"
#include "ExpressionSerializationTest.h"
#include "TestContext.h"

// Common project headers
#include <siodb/common/utils/DebugMacros.h>
#include <siodb/common/utils/StringUtils.h>
#include <siodb/iomgr/shared/dbengine/lob/BinaryValueBlobStream.h>
#include <siodb/iomgr/shared/dbengine/lob/StringClobStream.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace requests = dbengine::requests;

constexpr std::size_t kExtraBufferSize = 16;

TEST(Serialization_Constant, Null)
{
    constexpr std::size_t kExpectedSerializedSize = 2;
    const dbengine::Variant srcValue;
    const auto src = makeConstant(srcValue);
    testExpressionSerialization(*src, kExpectedSerializedSize);
}

TEST(Serialization_Constant, Int8)
{
    constexpr std::size_t kExpectedSerializedSize = 3;

    using VSP = std::pair<std::int8_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kExpectedSerializedSize},
            VSP {0xFF, kExpectedSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, kExpectedSerializedSize);
    }
}

TEST(Serialization_Constant, UInt8)
{
    constexpr std::size_t kExpectedSerializedSize = 3;

    using VSP = std::pair<std::uint8_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kExpectedSerializedSize},
            VSP {0xFF, kExpectedSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, kExpectedSerializedSize);
    }
}

TEST(Serialization_Constant, Int16)
{
    constexpr std::size_t kMinSerializedSize = 3;
    constexpr std::size_t kMaxSerializedSize = 5;

    using VSP = std::pair<std::int16_t, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {-1, kMinSerializedSize},
            VSP {0x7FFF, kMaxSerializedSize},
            VSP {0x8000, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, vsp.second);
    }
}

TEST(Serialization_Constant, UInt16)
{
    constexpr std::size_t kMinSerializedSize = 3;
    constexpr std::size_t kMaxSerializedSize = 5;

    using VSP = std::pair<std::uint16_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {0xFFFF, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, vsp.second);
    }
}

TEST(Serialization_Constant, Int32)
{
    constexpr std::size_t kMinSerializedSize = 3;
    constexpr std::size_t kMaxSerializedSize = 7;

    using VSP = std::pair<std::int32_t, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {-1, kMinSerializedSize},
            VSP {0x7FFFFFFF, kMaxSerializedSize},
            VSP {0x80000000, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, vsp.second);
    }
}

TEST(Serialization_Constant, UInt32)
{
    constexpr std::size_t kMinSerializedSize = 3;
    constexpr std::size_t kMaxSerializedSize = 7;

    using VSP = std::pair<std::uint32_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {0xFFFFFFFF, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, vsp.second);
    }
}

TEST(Serialization_Constant, Int64)
{
    constexpr std::size_t kMinSerializedSize = 3;
    constexpr std::size_t kMaxSerializedSize = 12;

    using VSP = std::pair<std::int64_t, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {-1, kMinSerializedSize},
            VSP {0x7FFFFFFFFFFFFFFFULL, kMaxSerializedSize},
            VSP {0x8000000000000000ULL, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, vsp.second);
    }
}

TEST(Serialization_Constant, UInt64)
{
    constexpr std::size_t kMinSerializedSize = 3;
    constexpr std::size_t kMaxSerializedSize = 12;

    using VSP = std::pair<std::uint64_t, std::size_t>;
    const std::array<VSP, 2> valueSizePairs {
            VSP {1, kMinSerializedSize},
            VSP {0xFFFFFFFFFFFFFFFFULL, kMaxSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, vsp.second);
    }
}

TEST(Serialization_Constant, Float)
{
    constexpr std::size_t kExpectedSerializedSize = 6;

    using VSP = std::pair<float, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {std::numeric_limits<float>::min(), kExpectedSerializedSize},
            VSP {-std::numeric_limits<float>::min(), kExpectedSerializedSize},
            VSP {std::numeric_limits<float>::max(), kExpectedSerializedSize},
            VSP {-std::numeric_limits<float>::max(), kExpectedSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, kExpectedSerializedSize);
    }
}

TEST(Serialization_Constant, Double)
{
    constexpr std::size_t kExpectedSerializedSize = 10;

    using VSP = std::pair<double, std::size_t>;
    const std::array<VSP, 4> valueSizePairs {
            VSP {std::numeric_limits<double>::min(), kExpectedSerializedSize},
            VSP {-std::numeric_limits<double>::min(), kExpectedSerializedSize},
            VSP {std::numeric_limits<double>::max(), kExpectedSerializedSize},
            VSP {-std::numeric_limits<double>::max(), kExpectedSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, kExpectedSerializedSize);
    }
}

TEST(Serialization_Constant, DateTime)
{
    constexpr std::size_t kDateOnlySerializedSize = 6;
    constexpr std::size_t kDateTimeSerializedSize = 12;

    using VSP = std::pair<siodb::RawDateTime, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::RawDateTime("2020-01-01", "%Y-%m-%d"), kDateOnlySerializedSize},
            VSP {siodb::RawDateTime("2020-01-01 00:00:00", "%Y-%m-%d %H:%M:%S"),
                    kDateOnlySerializedSize},
            VSP {siodb::RawDateTime("2020-01-01 01:01:01", "%Y-%m-%d %H:%M:%S"),
                    kDateTimeSerializedSize},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, vsp.second);
    }
}

TEST(Serialization_Constant, String)
{
    using VSP = std::pair<std::string, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::utils::createString(0), 3},
            VSP {siodb::utils::createString(3), 6},
            VSP {siodb::utils::createString(0xFFFF), 0xFFFF + 5},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, vsp.second);
    }
}

TEST(Serialization_Constant, Binary)
{
    using VSP = std::pair<siodb::BinaryValue, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::BinaryValue(), 3},
            VSP {siodb::BinaryValue(3), 6},
            VSP {siodb::BinaryValue(0xFFFF), 0xFFFF + 5},
    };

    for (const auto& vsp : valueSizePairs) {
        const auto src = makeConstant(vsp.first);
        testExpressionSerialization(*src, vsp.second);
    }
}

TEST(Serialization_Constant, Clob)
{
    TestContext context;

    using VSP = std::pair<std::string, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::utils::createString(0), 3},
            VSP {siodb::utils::createString(3), 6},
            VSP {siodb::utils::createString(0xFFFF), 0xFFFF + 5},
    };

    for (const auto& vsp : valueSizePairs) {
        dbengine::Variant srcValue(new dbengine::StringClobStream(vsp.first));
        const auto src = makeConstant(std::move(srcValue));
        const auto serializedSize = src->getSerializedSize();
        ASSERT_EQ(serializedSize, vsp.second);

        stdext::buffer<std::uint8_t> buffer(serializedSize + kExtraBufferSize, 0xCD);
        const auto end = src->serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, serializedSize);

        requests::ExpressionPtr dest;
        const auto consumed = requests::Expression::deserialize(buffer.data(), buffer.size(), dest);
        ASSERT_EQ(consumed, serializedSize);

        auto destValue = dest->evaluate(context);
        ASSERT_EQ(destValue.getValueType(), srcValue.getValueType());

        const auto srcStr = srcValue.getClob().readAsString(srcValue.getClob().getSize());
        const auto destStr = destValue.getClob().readAsString(destValue.getClob().getSize());
        ASSERT_EQ(srcStr, destStr);
    }
}

TEST(Serialization_Constant, TooLargeClob)
{
    const dbengine::Variant srcValue(new dbengine::StringClobStream(
            siodb::utils::createString(dbengine::Variant::kMaxStringValueLength * 2)));
    const auto src = makeConstant(std::move(srcValue));
    const auto serializedSize = src->getSerializedSize();
    ASSERT_EQ(serializedSize, 3U);

    stdext::buffer<std::uint8_t> buffer(serializedSize + kExtraBufferSize, 0xCD);
    EXPECT_THROW(src->serializeUnchecked(buffer.data()), dbengine::VariantSerializationError);
}

TEST(Serialization_Constant, Blob)
{
    TestContext context;

    using VSP = std::pair<siodb::BinaryValue, std::size_t>;
    const std::array<VSP, 3> valueSizePairs {
            VSP {siodb::BinaryValue(), 3},
            VSP {siodb::BinaryValue(3), 6},
            VSP {siodb::BinaryValue(0xFFFF), 0xFFFF + 5},
    };

    for (const auto& vsp : valueSizePairs) {
        dbengine::Variant srcValue(new dbengine::BinaryValueBlobStream(vsp.first));
        const auto src = makeConstant(std::move(srcValue));
        const auto serializedSize = src->getSerializedSize();
        ASSERT_EQ(serializedSize, vsp.second);

        stdext::buffer<std::uint8_t> buffer(serializedSize + kExtraBufferSize, 0xCD);
        const auto end = src->serializeUnchecked(buffer.data());
        const auto actualSize = static_cast<std::size_t>(end - buffer.data());
        ASSERT_EQ(actualSize, serializedSize);

        requests::ExpressionPtr dest;
        const auto consumed = requests::Expression::deserialize(buffer.data(), buffer.size(), dest);
        ASSERT_EQ(consumed, serializedSize);

        auto destValue = dest->evaluate(context);
        ASSERT_EQ(destValue.getValueType(), srcValue.getValueType());

        const auto srcStr = srcValue.getBlob().readAsBinary(srcValue.getBlob().getSize());
        const auto destStr = destValue.getBlob().readAsBinary(destValue.getBlob().getSize());
        ASSERT_EQ(srcStr, destStr);
    }
}

TEST(Serialization_Constant, TooLargeBlob)
{
    const dbengine::Variant srcValue(new dbengine::BinaryValueBlobStream(
            siodb::BinaryValue(dbengine::Variant::kMaxBinaryValueLength * 2)));
    const auto src = makeConstant(std::move(srcValue));
    const auto serializedSize = src->getSerializedSize();
    ASSERT_EQ(serializedSize, 3U);

    stdext::buffer<std::uint8_t> buffer(serializedSize + kExtraBufferSize, 0xCD);
    EXPECT_THROW(src->serializeUnchecked(buffer.data()), dbengine::VariantSerializationError);
}
