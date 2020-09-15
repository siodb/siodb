// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/Align.h>
#include <siodb/common/utils/Debug.h>
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/AesCipher.h>
#include <siodb/iomgr/shared/dbengine/io/EncryptedFile.h>

// STL headers
#include <limits>
#include <random>
#include <sstream>

// CRT headers
#include <cstdio>
#include <cstring>

// Google Test
#include <gtest/gtest.h>
#include <siodb/common/unit_test/GTestOutput.h>

using CipherClass = siodb::iomgr::dbengine::crypto::Aes128;
constexpr int kFileCreationMode = 0644;

class TestEnvironment : public ::testing::Environment {
public:
    auto getEncryptionContext() const noexcept
    {
        return m_encryptionContext;
    }

    auto getDecryptionContext() const noexcept
    {
        return m_decryptionContext;
    }

    auto makeNewFilePath()
    {
        return m_testDir + "/f_" + std::to_string(++m_fileId);
    }

    void SetUp() override
    {
        std::ostringstream str;
        str << ::getenv("HOME") << "/tmp/encrypted_file_test_" << std::time(nullptr) << '_'
            << ::getpid();
        m_testDir = str.str();
        fs::create_directories(m_testDir);
        m_fileId = 0;

        const auto cipher = std::make_shared<CipherClass>();
        const auto keySizeBytes = cipher->getKeySizeInBits() / 8;
        siodb::BinaryValue cipherKey(keySizeBytes);
        for (std::size_t i = 0; i < keySizeBytes; ++i)
            cipherKey[i] = i;
        m_encryptionContext = cipher->createEncryptionContext(cipherKey);
        m_decryptionContext = cipher->createDecryptionContext(cipherKey);
    }

    void TearDown() override
    {
        // In case of failed test keep resources for debug.
        if (testing::UnitTest::GetInstance()->Passed() && fs::exists(m_testDir)) {
            fs::remove_all(m_testDir);
        }
    }

private:
    std::string m_testDir;
    std::atomic<unsigned> m_fileId;
    siodb::iomgr::dbengine::crypto::CipherContextPtr m_encryptionContext;
    siodb::iomgr::dbengine::crypto::CipherContextPtr m_decryptionContext;
};

// See https://stackoverflow.com/a/15341467/1540501
TestEnvironment* g_testEnv;

// Test does:
// 1) Creates a file
// 2) Encrypts and writes an encrypted data sequentially
// 3) Closes a file
// 4) Opens an existing file
// 5) Reads and decrypts data
TEST(EncryptedFile, SeparateFiles)
{
    using namespace siodb;
    using namespace siodb::iomgr::dbengine;

    const std::string aString = "abcasflh23439z123k,n d 30!2-23,4. 3=]-old,fnmd;fl<>nrw+0[-ik1['.l";
    const std::uint8_t anArray[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    const std::uint32_t singleValue1 = 100;
    const std::uint16_t singleValue2 = 200;
    const std::vector<uint8_t> largeBlock(4097, 12);

    const auto filePath = g_testEnv->makeNewFilePath();
    {
        // Write file
        io::EncryptedFile file(filePath, 0, kFileCreationMode, g_testEnv->getEncryptionContext(),
                g_testEnv->getDecryptionContext(), 0);

        off_t offset = 0;

        DEBUG_TEST_PUTS("Writing string");
        auto bytesWritten = file.write(
                reinterpret_cast<const std::uint8_t*>(aString.data()), aString.size(), offset);
        ASSERT_EQ(bytesWritten, aString.size());
        offset += bytesWritten;
        ASSERT_EQ(file.getFileSize(), offset);

        DEBUG_TEST_PUTS("Writing array");
        bytesWritten = file.write(anArray, sizeof(anArray), offset);
        ASSERT_EQ(bytesWritten, sizeof(anArray));
        offset += bytesWritten;
        ASSERT_EQ(file.getFileSize(), offset);

        DEBUG_TEST_PUTS("Writing singleValue1");
        bytesWritten = file.write(
                reinterpret_cast<const std::uint8_t*>(&singleValue1), sizeof(singleValue1), offset);
        ASSERT_EQ(bytesWritten, sizeof(singleValue1));
        offset += bytesWritten;
        ASSERT_EQ(file.getFileSize(), offset);

        DEBUG_TEST_PUTS("Writing singleValue2");
        bytesWritten = file.write(
                reinterpret_cast<const std::uint8_t*>(&singleValue2), sizeof(singleValue2), offset);
        ASSERT_EQ(bytesWritten, sizeof(singleValue2));
        offset += bytesWritten;
        ASSERT_EQ(file.getFileSize(), offset);

        DEBUG_TEST_PUTS("Writing largeBlock");
        bytesWritten = file.write(largeBlock.data(), largeBlock.size(), offset);
        ASSERT_EQ(bytesWritten, largeBlock.size());
        offset += bytesWritten;
        ASSERT_EQ(file.getFileSize(), offset);

        DEBUG_TEST_PUTS("Checking file size");
        struct stat st;
        ASSERT_TRUE(file.stat(st));
        EXPECT_EQ(file.getLastError(), 0);
        ASSERT_EQ(offset, st.st_size);
    }

    {
        // Read file
        io::EncryptedFile file(
                filePath, 0, g_testEnv->getEncryptionContext(), g_testEnv->getDecryptionContext());

        off_t offset = 0;
        DEBUG_TEST_PUTS("Reading string");
        std::string decryptedStr(aString.size(), '\1');
        auto bytesRead = file.read(
                reinterpret_cast<std::uint8_t*>(decryptedStr.data()), decryptedStr.size(), offset);
        ASSERT_EQ(bytesRead, aString.size());
        ASSERT_EQ(decryptedStr, aString);
        offset += bytesRead;

        DEBUG_TEST_PUTS("Reading array");
        std::uint8_t decryptedArray[10] = {};
        bytesRead = file.read(decryptedArray, sizeof(decryptedArray), offset);
        ASSERT_EQ(bytesRead, sizeof(decryptedArray));
        ASSERT_EQ(std::memcmp(decryptedArray, anArray, sizeof(decryptedArray)), 0);
        offset += bytesRead;

        DEBUG_TEST_PUTS("Reading decryptedSingleValue1");
        std::uint32_t decryptedSingleValue1 = 0;
        bytesRead = file.read(reinterpret_cast<std::uint8_t*>(&decryptedSingleValue1),
                sizeof(decryptedSingleValue1), offset);
        ASSERT_EQ(bytesRead, sizeof(decryptedSingleValue1));
        ASSERT_EQ(decryptedSingleValue1, singleValue1);
        offset += bytesRead;

        DEBUG_TEST_PUTS("Reading decryptedSingleValue1");
        std::uint16_t decryptedSingleValue2 = 0;
        bytesRead = file.read(reinterpret_cast<std::uint8_t*>(&decryptedSingleValue2),
                sizeof(decryptedSingleValue2), offset);
        ASSERT_EQ(bytesRead, sizeof(decryptedSingleValue2));
        ASSERT_EQ(decryptedSingleValue2, singleValue2);
        offset += bytesRead;

        DEBUG_TEST_PUTS("Reading largeblock");
        std::vector<uint8_t> decryptedLargeBlock(4097);
        bytesRead = file.read(&decryptedLargeBlock[0], decryptedLargeBlock.size(), offset);
        ASSERT_EQ(bytesRead, decryptedLargeBlock.size());
        ASSERT_EQ(decryptedLargeBlock, largeBlock);
        offset += bytesRead;

        DEBUG_TEST_PUTS("Checking file size");
        struct stat st;
        ASSERT_TRUE(file.stat(st));
        EXPECT_EQ(file.getLastError(), 0);
        ASSERT_EQ(offset, st.st_size);
    }
}

// Test does:
// 1) Creates single a file both for writing and reading
// 2) Encrypts and writes an encrypted data sequentially
// 3) Reads and decrypts data
// 4) Closes file
TEST(EncryptedFile, SingleFile)
{
    using namespace siodb;
    using namespace siodb::iomgr::dbengine;

    const std::int32_t singleValue1 = std::numeric_limits<std::int32_t>::min();
    const std::uint8_t anArray[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    const std::vector<uint8_t> largeBlock(4096, 0xFF);
    const std::int16_t singleValue2 = std::numeric_limits<std::int16_t>::max();
    const std::string aString = "?";

    // Create file
    io::EncryptedFile file(g_testEnv->makeNewFilePath(), 0, kFileCreationMode,
            g_testEnv->getEncryptionContext(), g_testEnv->getDecryptionContext(), 0);

    // Encrypt std::int32_t
    off_t offset = 0;
    DEBUG_TEST_PUTS("Writing singleValue1");
    auto bytesWritten = file.write(
            reinterpret_cast<const std::uint8_t*>(&singleValue1), sizeof(singleValue1), offset);
    ASSERT_EQ(bytesWritten, sizeof(singleValue1));
    off_t nextOffset = offset + bytesWritten;
    ASSERT_EQ(file.getFileSize(), nextOffset);

    // Decrypt std::int32_t
    DEBUG_TEST_PUTS("Checking singleValue1");
    std::int32_t decryptedSingleValue1 = 0;
    auto bytesRead = file.read(reinterpret_cast<std::uint8_t*>(&decryptedSingleValue1),
            sizeof(decryptedSingleValue1), offset);
    ASSERT_EQ(bytesRead, sizeof(decryptedSingleValue1));
    ASSERT_EQ(decryptedSingleValue1, singleValue1);

    // Encrypt array
    offset = nextOffset;
    DEBUG_TEST_PUTS("Writing array");
    bytesWritten = file.write(anArray, sizeof(anArray), offset);
    ASSERT_EQ(bytesWritten, sizeof(anArray));
    nextOffset = offset + bytesWritten;
    ASSERT_EQ(file.getFileSize(), nextOffset);

    // Decrypt array
    DEBUG_TEST_PUTS("Checking array");
    std::uint8_t decryptedArray[16] = {};
    bytesRead = file.read(decryptedArray, sizeof(decryptedArray), offset);
    ASSERT_EQ(bytesRead, sizeof(decryptedArray));
    ASSERT_EQ(std::memcmp(decryptedArray, anArray, sizeof(decryptedArray)), 0);

    // Encrypt std::vector data
    offset = nextOffset;
    DEBUG_TEST_PUTS("Writing large data");
    bytesWritten = file.write(largeBlock.data(), largeBlock.size(), offset);
    ASSERT_EQ(bytesWritten, largeBlock.size());
    nextOffset = offset + bytesWritten;
    ASSERT_EQ(file.getFileSize(), nextOffset);

    // Decrypt std::vector data
    DEBUG_TEST_PUTS("Checking large data");
    std::vector<uint8_t> decryptedLargeBlock(4096);
    bytesRead = file.read(&decryptedLargeBlock[0], decryptedLargeBlock.size(), offset);
    ASSERT_EQ(bytesRead, decryptedLargeBlock.size());

    // Encrypt std::int16_t data
    offset = nextOffset;
    DEBUG_TEST_PUTS("Writing signleValue2");
    bytesWritten = file.write(
            reinterpret_cast<const std::uint8_t*>(&singleValue2), sizeof(singleValue2), offset);
    ASSERT_EQ(bytesWritten, sizeof(singleValue2));
    nextOffset = offset + bytesWritten;
    ASSERT_EQ(file.getFileSize(), nextOffset);

    // Decrypt std::int16_t
    DEBUG_TEST_PUTS("Checking signleValue2");
    std::int16_t decryptedSingleValue2 = 0;
    bytesRead = file.read(reinterpret_cast<std::uint8_t*>(&decryptedSingleValue2),
            sizeof(decryptedSingleValue2), offset);
    ASSERT_EQ(bytesRead, sizeof(decryptedSingleValue2));
    ASSERT_EQ(decryptedSingleValue2, singleValue2);

    // Encrypt string
    offset = nextOffset;
    DEBUG_TEST_PUTS("Writing string");
    bytesWritten = file.write(
            reinterpret_cast<const std::uint8_t*>(aString.data()), aString.size(), offset);
    ASSERT_EQ(bytesWritten, aString.size());
    nextOffset = offset + bytesWritten;
    ASSERT_EQ(file.getFileSize(), nextOffset);

    // Decrypt string
    DEBUG_TEST_PUTS("Checking string");
    std::string decryptedStr(aString.size(), '\1');
    bytesRead = file.read(
            reinterpret_cast<std::uint8_t*>(decryptedStr.data()), decryptedStr.size(), offset);
    ASSERT_EQ(bytesRead, aString.size());
    ASSERT_EQ(decryptedStr, aString);

    DEBUG_TEST_PUTS("Checking file size");
    struct stat st;
    ASSERT_TRUE(file.stat(st));
    EXPECT_EQ(nextOffset, st.st_size);

    EXPECT_EQ(file.getLastError(), 0);
}

// Test does:
// 1) Creates single a file both for writing and reading
// 2) Encrypts and writes an encrypted data in different places of file
// 3) Reads and decrypts data
// 4) Closes file
TEST(EncryptedFile, SingleFileWithRandomOffsets)
{
    using namespace siodb;
    using namespace siodb::iomgr::dbengine;

    const std::vector<uint8_t> largeBlock(799, 0xFF);
    const std::int32_t singleValue1 = 454753;
    const std::string aString = "?";
    const std::int16_t singleValue2 = -4352;
    const std::uint8_t anArray[5] = {5, 1, 3, 2, 4};

    // Create file
    io::EncryptedFile file(g_testEnv->makeNewFilePath(), 0, kFileCreationMode,
            g_testEnv->getEncryptionContext(), g_testEnv->getDecryptionContext(), 4096);

    // Encrypt std::vector data
    DEBUG_TEST_PUTS("Writing vector");
    auto bytesWritten = file.write(largeBlock.data(), largeBlock.size(), 1);
    ASSERT_EQ(bytesWritten, largeBlock.size());

    // Decrypt std::vector data
    DEBUG_TEST_PUTS("Checking vector");
    std::vector<uint8_t> decryptedLargeBlock(799);
    auto bytesRead = file.read(&decryptedLargeBlock[0], decryptedLargeBlock.size(), 1);
    ASSERT_EQ(bytesRead, decryptedLargeBlock.size());

    // Encrypt std::int32_t
    DEBUG_TEST_PUTS("Writing std::int32_t");
    bytesWritten = file.write(
            reinterpret_cast<const std::uint8_t*>(&singleValue1), sizeof(singleValue1), 800);
    ASSERT_EQ(bytesWritten, sizeof(singleValue1));

    // Decrypt std::int32_t
    DEBUG_TEST_PUTS("Checking std::int32_t");
    std::int32_t decryptedSingleValue1 = 0;
    bytesRead = file.read(reinterpret_cast<std::uint8_t*>(&decryptedSingleValue1),
            sizeof(decryptedSingleValue1), 800);
    ASSERT_EQ(bytesRead, sizeof(decryptedSingleValue1));
    ASSERT_EQ(decryptedSingleValue1, singleValue1);

    // Encrypt string
    DEBUG_TEST_PUTS("Writing string");
    bytesWritten =
            file.write(reinterpret_cast<const std::uint8_t*>(aString.data()), aString.size(), 1400);
    ASSERT_EQ(bytesWritten, aString.size());

    // Decrypt string
    DEBUG_TEST_PUTS("Checking string");
    std::string decryptedStr(aString.size(), '\1');
    bytesRead = file.read(
            reinterpret_cast<std::uint8_t*>(decryptedStr.data()), decryptedStr.size(), 1400);
    ASSERT_EQ(bytesRead, aString.size());
    ASSERT_EQ(decryptedStr, aString);

    // Encrypt std::int16_t data
    DEBUG_TEST_PUTS("Writing int16");
    bytesWritten = file.write(
            reinterpret_cast<const std::uint8_t*>(&singleValue2), sizeof(singleValue2), 4000);
    ASSERT_EQ(bytesWritten, sizeof(singleValue2));

    // Decrypt std::int16_t
    DEBUG_TEST_PUTS("Checking int16");
    std::int16_t decryptedSingleValue2 = 0;
    bytesRead = file.read(reinterpret_cast<std::uint8_t*>(&decryptedSingleValue2),
            sizeof(decryptedSingleValue2), 4000);
    ASSERT_EQ(bytesRead, sizeof(decryptedSingleValue2));
    ASSERT_EQ(decryptedSingleValue2, singleValue2);

    // Encrypt array
    // The same offset as previous operation
    DEBUG_TEST_PUTS("Writing array");
    bytesWritten = file.write(anArray, sizeof(anArray), 4000);
    ASSERT_EQ(bytesWritten, sizeof(anArray));

    // Decrypt array
    DEBUG_TEST_PUTS("Checking array");
    std::uint8_t decryptedArray[5] = {};
    bytesRead = file.read(decryptedArray, sizeof(decryptedArray), 4000);
    ASSERT_EQ(bytesRead, sizeof(decryptedArray));
    ASSERT_EQ(std::memcmp(decryptedArray, anArray, sizeof(decryptedArray)), 0);
    EXPECT_EQ(file.getLastError(), 0);
}

// Test does:
// 1) Creates a file
// 2) Encrypts and attempts to do several writes into a single block
// 3) Closes a file
// 4) Opens an existing file
// 5) Reads and decrypts data
TEST(EncryptedFile, UpdateBlock)
{
    using namespace siodb;
    using namespace siodb::iomgr::dbengine;

    std::uint8_t anArray[256];
    for (std::size_t i = 0; i < 256; ++i)
        anArray[i] = i;

    // Write file
    const auto filePath = g_testEnv->makeNewFilePath();
    {
        DEBUG_TEST_COUT << "Creating new file " << filePath << std::endl;
        io::EncryptedFile file(filePath, 0, kFileCreationMode, g_testEnv->getEncryptionContext(),
                g_testEnv->getDecryptionContext(), 0);

        DEBUG_TEST_PUTS("Writing first 256 bytes");
        off_t offset = 0;
        for (std::size_t i = 0; i < 256 / 4; ++i) {
            auto bytesWritten = file.write(&anArray[i * 4], 4, offset);
            ASSERT_EQ(bytesWritten, 4U);
            offset += 4;
            ASSERT_EQ(file.getFileSize(), offset);
        }

        // Make offset odd
        DEBUG_TEST_PUTS("Skip forward 3 bytes");
        offset += 3;

        DEBUG_TEST_PUTS("Write next 256 bytes");
        for (std::size_t i = 0; i < 256 / 4; ++i) {
            auto bytesWritten = file.write(&anArray[i * 4], 4, offset);
            ASSERT_EQ(bytesWritten, 4U);
            offset += 4;
            ASSERT_EQ(file.getFileSize(), offset);
        }

        struct stat st;
        ASSERT_TRUE(file.stat(st));
        DEBUG_TEST_COUT << "Plaintext size=" << st.st_size << std::endl;
        ASSERT_EQ(offset, st.st_size);
    }

    // Read file
    {
        DEBUG_TEST_COUT << "Opening file for reading: " << filePath << std::endl;
        io::EncryptedFile file(
                filePath, 0, g_testEnv->getEncryptionContext(), g_testEnv->getDecryptionContext());

        off_t offset = 0;
        for (std::size_t i = 0; i < 256; ++i) {
            std::uint8_t byte = 0;
            auto bytesRead = file.read(&byte, 1, i);
            ASSERT_EQ(bytesRead, 1U);
            ASSERT_EQ(byte, i);
            ++offset;
        }

        offset += 3;
        for (std::size_t i = 0; i < 256; ++i) {
            std::uint8_t byte = 0;
            auto bytesRead = file.read(&byte, 1, i);
            ASSERT_EQ(bytesRead, 1U);
            ASSERT_EQ(byte, i);
            ++offset;
        }

        struct stat st;
        ASSERT_TRUE(file.stat(st));
        DEBUG_TEST_COUT << "Plaintext size=" << st.st_size << std::endl;
        ASSERT_EQ(offset, st.st_size);
    }
}

// Test does:
// 1) Creates a file with predefined size
// 2) Stat file
// 3) Extend file on a block size
// 4) Stat file
// 5) Extend file on 1 byte
// 6) Stat file
TEST(EncryptedFile, Extend)
{
    using namespace siodb;
    using namespace siodb::iomgr::dbengine;

    constexpr off_t kInitialSize = 1023;
    io::EncryptedFile file(g_testEnv->makeNewFilePath(), 0, kFileCreationMode,
            g_testEnv->getEncryptionContext(), g_testEnv->getDecryptionContext(), kInitialSize);

    const auto blockBytesSize = file.getBlockSize();

    struct stat st;
    ASSERT_TRUE(file.stat(st));
    ASSERT_EQ(kInitialSize, st.st_size);

    auto extendLength = blockBytesSize;
    ASSERT_TRUE(file.extend(extendLength));
    off_t expectedSize = kInitialSize + extendLength;
    ASSERT_TRUE(file.stat(st));
    ASSERT_EQ(expectedSize, st.st_size);

    extendLength = 1;
    ASSERT_TRUE(file.extend(extendLength));
    expectedSize += extendLength;
    ASSERT_TRUE(file.stat(st));
    ASSERT_EQ(expectedSize, st.st_size);
}

TEST(EncryptedFile, RandomReadWrite)
{
    using namespace siodb;
    using namespace siodb::iomgr::dbengine;

    constexpr off_t kFileSize = 1024 * 1024;
    siodb::BinaryValue buffer1(kFileSize), buffer2(kFileSize);

    // Create files
    const auto encryptedFilePath = g_testEnv->makeNewFilePath();
    io::EncryptedFile efile(encryptedFilePath, 0, kFileCreationMode,
            g_testEnv->getEncryptionContext(), g_testEnv->getDecryptionContext(), kFileSize);

    // Check size
    struct stat st;
    ASSERT_TRUE(efile.stat(st));
    ASSERT_EQ(kFileSize, st.st_size);

    // Write initial data (all zeros)
    ASSERT_EQ(efile.write(buffer1.data(), buffer1.size(), 0), buffer1.size());

    // Initialize random generator
    const auto seed = std::random_device()();
    TEST_COUT << "Seed=" << seed << std::endl;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<off_t> positionDist(0, kFileSize - 1);
    std::uniform_int_distribution<std::uint8_t> contnentDist(
            0, std::numeric_limits<std::uint8_t>::max());

    const int kRepeatCount = 200;
    for (int i = 0; i < kRepeatCount; ++i) {
        const auto pos = positionDist(gen);
        std::size_t len;
        do {
            len = positionDist(gen) % (kFileSize - pos);
        } while (len == 0);
        TEST_COUT << "Iteration #" << i << " pos " << pos << " len " << len << std::endl;
        for (off_t i = pos, n = pos + len; i < n; ++i)
            buffer1[i] = contnentDist(gen);
        ASSERT_EQ(efile.write(buffer1.data() + pos, len, pos), len);
        ASSERT_EQ(efile.read(buffer2.data() + pos, len, pos), len);
        ASSERT_EQ(std::memcmp(buffer1.data() + pos, buffer2.data() + pos, len), 0);
    }
}

int main(int argc, char** argv)
{
    DEBUG_SYSCALLS_LIBRARY_GUARD;
    testing::InitGoogleTest(&argc, argv);
    auto testEnv = new TestEnvironment();
    testing::AddGlobalTestEnvironment(testEnv);
    g_testEnv = testEnv;
    return RUN_ALL_TESTS();
}
