// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "CipherContext.h"

// OpenSSL headers
#include <openssl/camellia.h>

namespace siodb::iomgr::dbengine::crypto {

/** Base class for all Camellia cipher contexts */
class CamelliaCipherContext : public CipherContext {
protected:
    /**
     * Initializes instance of class CamelliaCipherContext.
     * @param cipher Cipher instance.
     * @param key A key.
     */
    explicit CamelliaCipherContext(ConstCipherPtr&& cipher, const BinaryValue& key) noexcept
        : CipherContext(std::move(cipher))
    {
        Camellia_set_key(key.data(), key.size() * 8, &m_preparedKey);
    }

protected:
    /** Prepared encryption or decryption key */
    CAMELLIA_KEY m_preparedKey;
};

/** Encryption context for all Camellia ciphers */
class CamelliaEncryptionContext : public CamelliaCipherContext {
public:
    /**
     * Initializes instance of class AesEncryptionContext.
     * @param cipher Cipher instance.
     * @param key A key.
     */
    explicit CamelliaEncryptionContext(ConstCipherPtr&& cipher, const BinaryValue& key) noexcept
        : CamelliaCipherContext(std::move(cipher), key)
    {
    }

    /**
     * Transforms (i.e. encrypts or decrypts) given number of blocks.
     * @param in Input data.
     * @param blockCount Number of data blocks to process.
     * @param out Output buffer.
     */
    void transform(const std::uint8_t* in, unsigned blockCount, std::uint8_t* out) const
            noexcept override final;
};

/** Decryption context for all Camellia ciphers */
class CamelliaDecryptionContext : public CamelliaCipherContext {
public:
    /**
     * Initializes instance of class AesDecryptionContext.
     * @param cipher Cipher instance.
     * @param key A key.
     */
    explicit CamelliaDecryptionContext(ConstCipherPtr&& cipher, const BinaryValue& key) noexcept
        : CamelliaCipherContext(std::move(cipher), key)
    {
    }

    /**
     * Transforms (i.e. encrypts or decrypts) given number of blocks.
     * @param in Input data.
     * @param blockCount Number of data blocks to process.
     * @param out Output buffer.
     */
    void transform(const std::uint8_t* in, unsigned blockCount, std::uint8_t* out) const
            noexcept override final;
};

}  // namespace siodb::iomgr::dbengine::crypto
