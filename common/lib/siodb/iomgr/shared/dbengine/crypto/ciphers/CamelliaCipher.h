// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Cipher.h"

namespace siodb::iomgr::dbengine::crypto {

/** Base class for all Camellia ciphers. */
class CamelliaCipher : public Cipher {
public:
    /**
     * Returns cipher block size in bits.
     * @return Block size in bits.
     */
    unsigned getBlockSizeInBits() const noexcept override final;

protected:
    /**
     * Creates encryption context with a specified key. Assumes key length is valid.
     * @param key A key.
     * @return Cipher context object.
     */
    CipherContextPtr doCreateEncryptionContext(const BinaryValue& key) const override;

    /**
     * Creates decryption context with a specified key. Assumes key length is valid.
     * @param key A key.
     * @return Cipher context object.
     */
    CipherContextPtr doCreateDecryptionContext(const BinaryValue& key) const override;
};

/** Implementation of the Camellia with 128 bit key. */
class Camellia128 final : public CamelliaCipher {
public:
    /**
     * Returns cipher identification string.
     * @return Cipher identification string.
     */
    const char* getCipherId() const noexcept override;

    /**
     * Returns cipher key size in bits.
     * @return Key size in bits.
     */
    unsigned getKeySizeInBits() const noexcept override;
};

/** Implementation of the Camellia with 192 bit key. */
class Camellia192 final : public CamelliaCipher {
public:
    /**
     * Returns cipher identification string.
     * @return Cipher identification string.
     */
    const char* getCipherId() const noexcept override;

    /**
     * Returns cipher key size in bits.
     * @return Key size in bits.
     */
    unsigned getKeySizeInBits() const noexcept override;
};

/** Implementation of the Camellia with 256 bit key. */
class Camellia256 final : public CamelliaCipher {
public:
    /**
     * Returns cipher identification string.
     * @return Cipher identification string.
     */
    const char* getCipherId() const noexcept override;

    /**
     * Returns cipher key size in bits.
     * @return Key size in bits.
     */
    unsigned getKeySizeInBits() const noexcept override;
};

}  // namespace siodb::iomgr::dbengine::crypto
