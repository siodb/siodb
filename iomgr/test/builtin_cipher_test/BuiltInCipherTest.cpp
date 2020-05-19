// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// CRT headers
#include <cstring>

// STL headers
#include <iomanip>
#include <iostream>
#include <random>

// Project headers
#include "dbengine/crypto/ciphers/AesCipher.h"
#include "dbengine/crypto/ciphers/CamelliaCipher.h"
#include "dbengine/crypto/ciphers/CipherContext.h"

// Google Test
#include <gtest/gtest.h>

namespace {

void printData(std::ostream& os, const char* title, const siodb::BinaryValue& data)
{
    if (title) os << title << ": \n";
    for (std::size_t i = 0, n = data.size(); i < n; ++i)
        os << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(data[i])
           << ' ';
    os << std::endl;
}

bool testCipher(const siodb::iomgr::dbengine::crypto::Cipher& cipher, unsigned maxBlockCount)
{
    const auto blockSize = cipher.getBlockSize() / 8;
    const auto keySize = cipher.getKeySize() / 8;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned> dis(0, 255);
    const auto blockCount = std::min(dis(gen) % 16U + 1U, maxBlockCount);
    const auto dataSize = blockCount * blockSize;

    siodb::BinaryValue data(dataSize);
    siodb::BinaryValue encryptedData(dataSize);
    siodb::BinaryValue decryptedData(dataSize);
    siodb::BinaryValue key(keySize);

    for (unsigned j = 0; j < keySize; ++j)
        key[j] = dis(gen);

    const auto encryptionContext = cipher.createEncryptionContext(key);
    const auto decryptionContext = cipher.createDecryptionContext(key);

    for (int i = 0; i < 256; ++i) {
        for (unsigned j = 0; j < dataSize; ++j)
            data[j] = dis(gen);
        encryptionContext->transform(data.data(), blockCount, encryptedData.data());
        decryptionContext->transform(encryptedData.data(), blockCount, decryptedData.data());
        if (std::memcmp(decryptedData.data(), data.data(), dataSize)) {
            std::cerr << "data size=" << dataSize << std::endl;
            printData(std::cerr, "Data", data);
            printData(std::cerr, "Encrypted Data", encryptedData);
            printData(std::cerr, "Decrypted Data", decryptedData);
            return false;
        }
    }
    return true;
}

}  // anonymous namespace

TEST(BuiltInCiphers, Aes128)
{
    const auto cipher = std::make_shared<siodb::iomgr::dbengine::crypto::Aes128>();
    ASSERT_TRUE(testCipher(*cipher, 1));
    ASSERT_TRUE(testCipher(*cipher, 16));
}

TEST(BuiltInCiphers, Aes192)
{
    const auto cipher = std::make_shared<siodb::iomgr::dbengine::crypto::Aes192>();
    ASSERT_TRUE(testCipher(*cipher, 1));
    ASSERT_TRUE(testCipher(*cipher, 16));
}

TEST(BuiltInCiphers, Aes256)
{
    const auto cipher = std::make_shared<siodb::iomgr::dbengine::crypto::Aes256>();
    ASSERT_TRUE(testCipher(*cipher, 1));
    ASSERT_TRUE(testCipher(*cipher, 16));
}

TEST(BuiltInCiphers, Camellia128)
{
    const auto cipher = std::make_shared<siodb::iomgr::dbengine::crypto::Camellia128>();
    ASSERT_TRUE(testCipher(*cipher, 1));
    ASSERT_TRUE(testCipher(*cipher, 16));
}

TEST(BuiltInCiphers, Camellia192)
{
    const auto cipher = std::make_shared<siodb::iomgr::dbengine::crypto::Camellia192>();
    ASSERT_TRUE(testCipher(*cipher, 1));
    ASSERT_TRUE(testCipher(*cipher, 16));
}

TEST(BuiltInCiphers, Camellia256)
{
    const auto cipher = std::make_shared<siodb::iomgr::dbengine::crypto::Camellia256>();
    ASSERT_TRUE(testCipher(*cipher, 1));
    ASSERT_TRUE(testCipher(*cipher, 16));
}

int main(int argc, char** argv)
{
    // Run tests
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
