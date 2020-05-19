// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/utils/StringScanner.h>

// Google Test
#include <gtest/gtest.h>

using namespace siodb::utils;

TEST(StringScannerTest, EmptyString)
{
    const char* data = "";
    StringScanner scanner(data, 0u);
    ASSERT_EQ(scanner.data(), data);
    ASSERT_EQ(scanner.current(), data);
    ASSERT_EQ(scanner.size(), 0u);
    ASSERT_EQ(scanner.remainingSize(), 0u);
    ASSERT_FALSE(scanner.hasMoreData());
    ASSERT_EQ(scanner.find(""), 0u);
    ASSERT_EQ(scanner.find("abc"), std::string::npos);
    ASSERT_EQ(scanner.findInLine(""), 0u);
    ASSERT_EQ(scanner.findInLine("abc"), std::string::npos);
    ASSERT_TRUE(scanner.advance(0u));
    ASSERT_EQ(scanner.pos(), 0u);
    ASSERT_FALSE(scanner.advance(1u));
    ASSERT_EQ(scanner.pos(), 0u);
    int testInt = 0;
    ASSERT_FALSE(scanner.read(&testInt, sizeof(testInt)));
    ASSERT_FALSE(scanner.skipUntilWhitespace());
    ASSERT_FALSE(scanner.skipWhitespaces());
    ASSERT_TRUE(scanner.startsWith(""));
    ASSERT_FALSE(scanner.startsWith("abc"));
}

TEST(StringScannerTest, NullptrString)
{
    StringScanner scanner(nullptr, 0u);
    ASSERT_EQ(scanner.data(), nullptr);
    ASSERT_EQ(scanner.current(), nullptr);
    ASSERT_EQ(scanner.size(), 0u);
    ASSERT_EQ(scanner.remainingSize(), 0u);
    ASSERT_FALSE(scanner.hasMoreData());
    ASSERT_EQ(scanner.find(""), 0u);
    ASSERT_EQ(scanner.find("abc"), std::string::npos);
    ASSERT_EQ(scanner.findInLine(""), 0u);
    ASSERT_EQ(scanner.findInLine("abc"), std::string::npos);
    ASSERT_TRUE(scanner.advance(0u));
    ASSERT_EQ(scanner.pos(), 0u);
    ASSERT_FALSE(scanner.advance(1));
    ASSERT_EQ(scanner.pos(), 0u);
    int testInt = 0;
    ASSERT_FALSE(scanner.read(&testInt, sizeof(testInt)));
    ASSERT_FALSE(scanner.skipUntilWhitespace());
    ASSERT_FALSE(scanner.skipWhitespaces());
    ASSERT_TRUE(scanner.startsWith(""));
    ASSERT_FALSE(scanner.startsWith("abc"));

    ASSERT_THROW(StringScanner(nullptr, 1), std::runtime_error);
}

TEST(StringScannerTest, ForwardTest)
{
    const char* data = "123456789";
    StringScanner scanner(data, strlen(data));
    ASSERT_EQ(scanner.data(), data);
    ASSERT_EQ(scanner.current(), data);
    ASSERT_EQ(scanner.size(), strlen(data));
    ASSERT_EQ(scanner.remainingSize(), strlen(data));
    ASSERT_TRUE(scanner.hasMoreData());
    ASSERT_EQ(*scanner.current(), '1');

    ASSERT_TRUE(scanner.advance(0u));
    ASSERT_EQ(*scanner.current(), '1');
    ASSERT_EQ(scanner.pos(), 0u);

    ASSERT_TRUE(scanner.advance(1u));
    ASSERT_EQ(*scanner.current(), '2');
    ASSERT_EQ(scanner.remainingSize(), strlen(data) - 1);
    ASSERT_EQ(scanner.pos(), 1u);
    ASSERT_TRUE(scanner.hasMoreData());

    ASSERT_TRUE(scanner.advance(7u));
    ASSERT_TRUE(scanner.hasMoreData());
    ASSERT_EQ(*scanner.current(), '9');
    ASSERT_EQ(scanner.pos(), 8u);

    ASSERT_TRUE(scanner.advance(1u));
    ASSERT_FALSE(scanner.hasMoreData());
    ASSERT_TRUE(scanner.advance(0u));
    ASSERT_FALSE(scanner.advance(1u));
    ASSERT_EQ(scanner.remainingSize(), 0u);
}

TEST(StringScannerTest, SkipWhiteSpaceTest)
{
    const char* data = " x\n\n\r\r \t\t \n y ";
    StringScanner scanner(data, strlen(data));
    ASSERT_TRUE(scanner.hasMoreData());
    ASSERT_EQ(scanner.pos(), 0u);
    ASSERT_TRUE(scanner.skipWhitespaces());
    ASSERT_EQ(scanner.pos(), 1u);
    ASSERT_TRUE(scanner.hasMoreData());

    ASSERT_EQ(*scanner.current(), 'x');
    ASSERT_TRUE(scanner.advance(1u));
    ASSERT_TRUE(scanner.skipWhitespaces());

    ASSERT_EQ(*scanner.current(), 'y');
    ASSERT_TRUE(scanner.advance(1u));
    ASSERT_TRUE(scanner.hasMoreData());

    ASSERT_EQ(scanner.remainingSize(), 1u);
    ASSERT_FALSE(scanner.skipWhitespaces());
    ASSERT_EQ(scanner.remainingSize(), 0u);
    ASSERT_FALSE(scanner.hasMoreData());
}

TEST(StringScannerTest, SkipUntilWhiteSpaceTest)
{
    const char* data = "aaa\taaa\naaa\raaa aaa";
    StringScanner scanner(data, strlen(data));
    ASSERT_TRUE(scanner.hasMoreData());
    ASSERT_EQ(scanner.pos(), 0u);
    ASSERT_TRUE(scanner.skipUntilWhitespace());
    ASSERT_EQ(scanner.pos(), 3u);
    ASSERT_TRUE(scanner.hasMoreData());
    ASSERT_EQ(*scanner.current(), '\t');
    ASSERT_TRUE(scanner.advance(1u));

    ASSERT_TRUE(scanner.skipUntilWhitespace());
    ASSERT_EQ(scanner.pos(), 7u);
    ASSERT_TRUE(scanner.hasMoreData());
    ASSERT_EQ(*scanner.current(), '\n');
    ASSERT_TRUE(scanner.advance(1u));

    ASSERT_TRUE(scanner.skipUntilWhitespace());
    ASSERT_EQ(scanner.pos(), 11u);
    ASSERT_TRUE(scanner.hasMoreData());
    ASSERT_EQ(*scanner.current(), '\r');
    ASSERT_TRUE(scanner.advance(1u));

    ASSERT_TRUE(scanner.skipUntilWhitespace());
    ASSERT_EQ(scanner.pos(), 15u);
    ASSERT_TRUE(scanner.hasMoreData());
    ASSERT_EQ(*scanner.current(), ' ');
    ASSERT_TRUE(scanner.advance(1u));

    ASSERT_FALSE(scanner.skipUntilWhitespace());
    ASSERT_EQ(scanner.pos(), strlen(data));
    ASSERT_FALSE(scanner.hasMoreData());
}

TEST(StringScannerTest, FindTest)
{
    const char* data = "xyz\nabc";
    StringScanner scanner(data, strlen(data));
    ASSERT_EQ(scanner.findPtr(""), data);
    ASSERT_EQ(scanner.find(""), 0u);
    ASSERT_EQ(scanner.find("abc"), 4u);
    ASSERT_EQ(scanner.findPtr("abc"), data + 4);
    ASSERT_EQ(scanner.find("xyz"), 0u);
    ASSERT_EQ(scanner.findInLine(""), 0u);
    ASSERT_EQ(scanner.findInLine("abc"), std::string::npos);
    ASSERT_EQ(scanner.findInLine("xyz"), 0u);
    ASSERT_EQ(scanner.findInLine("yz"), 1u);
    ASSERT_EQ(scanner.findInLinePtr("yz"), data + 1);
    ASSERT_EQ(scanner.pos(), 0u);

    ASSERT_TRUE(scanner.advance(4u));  // -> abc
    ASSERT_EQ(*scanner.current(), 'a');

    ASSERT_EQ(scanner.find(""), 0u);
    ASSERT_EQ(scanner.find("abc"), 0u);
    ASSERT_EQ(scanner.find("c"), 2u);
    ASSERT_EQ(scanner.findPtr("c"), scanner.current() + 2u);
    ASSERT_EQ(scanner.find("xyz"), std::string::npos);
    ASSERT_EQ(scanner.findPtr("xyz"), nullptr);
    ASSERT_EQ(scanner.findInLine(""), 0u);
    ASSERT_EQ(scanner.findInLine("abc"), 0u);
    ASSERT_EQ(scanner.findInLinePtr(""), scanner.current());
    ASSERT_EQ(scanner.findInLinePtr("abc"), scanner.current());
    ASSERT_EQ(scanner.findInLine("xyz"), std::string::npos);
    ASSERT_EQ(scanner.findInLinePtr("xyz"), nullptr);
}

TEST(StringScannerTest, StartsWithTest)
{
    const char* data = "xyz\nabc";
    StringScanner scanner(data, strlen(data));
    ASSERT_TRUE(scanner.startsWith(""));
    ASSERT_FALSE(scanner.startsWith("abc"));
    ASSERT_TRUE(scanner.startsWith("xyz"));
    ASSERT_EQ(scanner.pos(), 0u);

    ASSERT_TRUE(scanner.advance(4u));  // -> abc
    ASSERT_EQ(*scanner.current(), 'a');

    ASSERT_TRUE(scanner.startsWith(""));
    ASSERT_TRUE(scanner.startsWith("abc"));
    ASSERT_FALSE(scanner.startsWith("xyz"));
}

TEST(StringScannerTest, ReadTest)
{
    const int testInt = 2039487;
    const char testChar = 'Q';
    const double testDouble = 72376385901956.1237592;
    const std::uint16_t testUint16 = 65535u;

    char data[sizeof(testInt) + sizeof(testChar) + sizeof(testDouble) + sizeof(testUint16)
              + sizeof(testUint16)];

    auto tempData = data;
    memcpy(tempData, &testInt, sizeof(testInt));
    tempData += sizeof(testInt);
    memcpy(tempData, &testChar, sizeof(testChar));
    tempData += sizeof(testChar);
    memcpy(tempData, &testDouble, sizeof(testDouble));
    tempData += sizeof(testDouble);
    memcpy(tempData, &testUint16, sizeof(testUint16));
    tempData += sizeof(testUint16);
    memcpy(tempData, &testUint16, sizeof(testUint16));
    tempData += sizeof(testUint16);

    StringScanner scanner(data, sizeof(data));
    ASSERT_EQ(scanner.data(), data);
    ASSERT_EQ(scanner.current(), data);
    ASSERT_EQ(scanner.size(), sizeof(data));
    ASSERT_EQ(scanner.remainingSize(), sizeof(data));
    ASSERT_TRUE(scanner.hasMoreData());

    char biggerData[sizeof(data) + 1u];
    ASSERT_FALSE(scanner.read(biggerData, sizeof(biggerData)));
    ASSERT_EQ(scanner.pos(), 0u);

    int readInt = 0;
    ASSERT_TRUE(scanner.read(&readInt, sizeof(int)));
    ASSERT_EQ(scanner.remainingSize(), sizeof(data) - sizeof(int));
    ASSERT_EQ(scanner.pos(), sizeof(int));
    ASSERT_EQ(readInt, testInt);

    char readChar = 0;
    ASSERT_TRUE(scanner.read(&readChar, sizeof(readChar)));
    ASSERT_EQ(readChar, testChar);

    double readDouble = 0;
    ASSERT_TRUE(scanner.read(&readDouble, sizeof(readDouble)));
    ASSERT_EQ(readDouble, testDouble);

    std::uint16_t readUint16 = 0u;
    ASSERT_TRUE(scanner.read(&readUint16, sizeof(readUint16)));
    ASSERT_EQ(readUint16, testUint16);

    /// Only 2 last bytes remain in the remain data
    ASSERT_FALSE(scanner.read(&readInt, sizeof(int)));
    ASSERT_TRUE(scanner.hasMoreData());

    readUint16 = 0u;
    ASSERT_TRUE(scanner.read(&readUint16, sizeof(readUint16)));
    ASSERT_FALSE(scanner.hasMoreData());
    ASSERT_FALSE(scanner.read(&readUint16, sizeof(readUint16)));
    ASSERT_EQ(scanner.remainingSize(), 0u);
}

TEST(StringScannerTest, SetCurrentTest)
{
    const char* text = "test";
    const char* textEnd = text + 4;
    StringScanner scanner(text, strlen(text));

    scanner.setCurrent(scanner.current());
    scanner.setCurrent(text);
    scanner.setCurrent(text + 1);
    scanner.setCurrent(textEnd);
    ASSERT_FALSE(scanner.hasMoreData());
    ASSERT_THROW(scanner.setCurrent(textEnd + 1), std::out_of_range);
    ASSERT_THROW(scanner.setCurrent(scanner.data() - 1), std::out_of_range);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
