// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "KeyGenerator.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/utils/FdGuard.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/common/utils/RandomUtils.h>

// CRT headers
#include <ctime>

// STL headers
#include <stdexcept>

// OpenSSL
#include <openssl/sha.h>

// System headers
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace siodb::iomgr::dbengine::crypto {

namespace {
constexpr const char* kDefaultSeed = "siodb";
constexpr std::size_t kRandomSeedSize = 32;
}  // anonymous namespace

BinaryValue generateCipherKey(unsigned keyLength, const std::string& seed)
{
    if (keyLength == 0 || keyLength > 512 || keyLength % 8 != 0)
        throw std::invalid_argument("Invalid key length");

    // Collect entropy from system
    std::uint8_t rdata[kRandomSeedSize + 2];
    utils::getRandomBytes(rdata, sizeof(rdata));

    // Collect current time
    const auto t = std::time(nullptr);

    // Decode number of hashing rounds
    std::uint16_t n = 0;
    ::pbeDecodeUInt16(rdata + 32, &n);
    const unsigned hashRoundCount = n | 32768;

    // Perform hashing
    std::uint8_t hash[64];
    const char* seedData = seed.empty() ? kDefaultSeed : seed.data();
    const std::size_t seedLength = seed.empty() ? ct_strlen(kDefaultSeed) : seed.length();
    if (keyLength <= 256) {
        // Use SHA-256
        ::SHA256_CTX ctx;
        ::SHA256_Init(&ctx);
        ::SHA256_Update(&ctx, seedData, seedLength);
        ::SHA256_Update(&ctx, &t, sizeof(t));
        ::SHA256_Update(&ctx, rdata, kRandomSeedSize);
        ::SHA256_Final(hash, &ctx);
        for (unsigned i = 0; i < hashRoundCount; ++i) {
            ::SHA256_Init(&ctx);
            ::SHA256_Update(&ctx, hash, 32);
            ::SHA256_Final(hash, &ctx);
        }
    } else {
        // Use SHA-512
        ::SHA512_CTX ctx;
        ::SHA512_Init(&ctx);
        ::SHA512_Update(&ctx, seedData, seedLength);
        ::SHA512_Update(&ctx, &t, sizeof(t));
        ::SHA512_Update(&ctx, rdata, kRandomSeedSize);
        ::SHA512_Final(hash, &ctx);
        for (unsigned i = 0; i < hashRoundCount; ++i) {
            ::SHA512_Init(&ctx);
            ::SHA512_Update(&ctx, hash, 64);
            ::SHA512_Final(hash, &ctx);
        }
    }

    // Copy out the key
    return BinaryValue(hash, hash + keyLength / 8);
}

}  // namespace siodb::iomgr::dbengine::crypto
