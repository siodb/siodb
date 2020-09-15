// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "CipherContext.h"

// OpenSSL headers
#include <openssl/aes.h>

namespace siodb::iomgr::dbengine::crypto {

/** Base class for all AES cipher contexts */
class AesCipherContext : public CipherContext {
protected:
    /**
     * Initializes object of class AesCipherContext.
     * @param cipher Cipher instance.
     */
    explicit AesCipherContext(ConstCipherPtr&& cipher) noexcept
        : CipherContext(std::move(cipher))
    {
    }

protected:
    /** Prepared encryption or decryption key */
    AES_KEY m_preparedKey;
};

/** Encryption context for all AES ciphers */
class AesEncryptionContext : public AesCipherContext {
public:
    /**
     * Initializes instance of class AesEncryptionContext.
     * @param cipher Cipher instance.
     * @param key A key.
     */
    explicit AesEncryptionContext(ConstCipherPtr&& cipher, const BinaryValue& key) noexcept
        : AesCipherContext(std::move(cipher))
    {
        AES_set_encrypt_key(key.data(), key.size() * 8, &m_preparedKey);
    }

    /**
     * Transforms (i.e. encrypts or decrypts) given number of blocks.
     * @param in Input data.
     * @param blockCount Number of data blocks to process.
     * @param out Output buffer.
     */
    void transform(const void* in, std::size_t blockCount, void* out) const noexcept override final;
};

/** Decryption context for all AES ciphers */
class AesDecryptionContext : public AesCipherContext {
public:
    /**
     * Initializes instance of class AesDecryptionContext.
     * @param cipher Cipher instance.
     * @param key A key.
     */
    explicit AesDecryptionContext(ConstCipherPtr&& cipher, const BinaryValue& key) noexcept
        : AesCipherContext(std::move(cipher))
    {
        AES_set_decrypt_key(key.data(), key.size() * 8, &m_preparedKey);
    }

    /**
     * Transforms (i.e. encrypts or decrypts) given number of blocks.
     * @param in Input data.
     * @param blockCount Number of data blocks to process.
     * @param out Output buffer.
     */
    void transform(const void* in, std::size_t blockCount, void* out) const noexcept override final;
};

}  // namespace siodb::iomgr::dbengine::crypto
