// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "CipherContextPtr.h"
#include "CipherPtr.h"
#include <siodb/common/utils/BinaryValue.h>

// STL headers
#include <optional>

namespace siodb::config {
struct ExternalCipherOptions;
}  // namespace siodb::config

namespace siodb::iomgr::dbengine::crypto {

class CipherContext;

/** Base class for all ciphers. */
class Cipher : public std::enable_shared_from_this<Cipher> {
public:
    /** De-initialzes object */
    virtual ~Cipher() = default;

    /**
     * Returns cipher identification string.
     * @return Cipher identification string.
     */
    virtual const char* getCipherId() const noexcept = 0;

    /**
     * Returns cipher block size in bits.
     * @return Block size in bits.
     */
    virtual unsigned getBlockSizeInBits() const noexcept = 0;

    /**
     * Returns cipher key size in bits.
     * @return Key size in bits.
     */
    virtual unsigned getKeySizeInBits() const noexcept = 0;

    /**
     * Creates encryption context with a specified key.
     * @param key A key.
     * @return Cipher context object.
     */
    CipherContextPtr createEncryptionContext(const BinaryValue& key) const;

    /**
     * Creates decryption context with a specified key.
     * @param key A key.
     * @return Cipher context object.
     */
    CipherContextPtr createDecryptionContext(const BinaryValue& key) const;

protected:
    /**
     * Validates key length.
     * @throw DatabaseError if key length doesn't match.
     */
    void validateKeyLength(const BinaryValue& key) const;

    /**
     * Creates encryption context with a specified key. Assumes key length is valid.
     * @param key A key.
     * @return Cipher context object.
     */
    virtual CipherContextPtr doCreateEncryptionContext(const BinaryValue& key) const = 0;

    /**
     * Creates decryption context with a specified key. Assumes key length is valid.
     * @param key A key.
     * @return Cipher context object.
     */
    virtual CipherContextPtr doCreateDecryptionContext(const BinaryValue& key) const = 0;
};

/** Initializes all built-in ciphers. */
void initializeBuiltInCiphers();

/**
 * Initializes all external ciphers.
 * @param externalCipherOptions External cipher options.
 */
void initializeExternalCiphers(const config::ExternalCipherOptions& externaCipherOptions);

/**
 * Returns specified cipher object.
 * @param cipherId Cipher identification string.
 * @return Cipher object, nullptr if cipher is "none", empty object if cipher doesn't exist.
 */
std::optional<CipherPtr> getCipher0(const std::string& cipherId);

/** No cipher ID */
constexpr const char* kNoCipherId = "none";

}  // namespace siodb::iomgr::dbengine::crypto
