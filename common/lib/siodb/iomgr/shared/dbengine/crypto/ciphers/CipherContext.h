// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Cipher.h"

namespace siodb::iomgr::dbengine::crypto {

/** Base class for all cipher contexts. */
class CipherContext {
protected:
    /**
     * Initializes object of class CipherContext.
     * @param cipher Cipher instance.
     */
    explicit CipherContext(ConstCipherPtr&& cipher) noexcept
        : m_cipher(std::move(cipher))
        , m_blockSizeInBytes(m_cipher->getBlockSizeInBits() / 8)
    {
    }

public:
    /** De-initialzes object */
    virtual ~CipherContext() = default;

    /**
     * Returns underlying cipher.
     * @return Cipher object.
     */
    const auto& getCipher() const noexcept
    {
        return *m_cipher;
    }

    /**
     * Returns block size in bytes.
     * @return Block size in bytes.
     */
    auto getBlockSizeInBytes() const noexcept
    {
        return m_blockSizeInBytes;
    }

    /**
     * Transforms (i.e. encrypts or decrypts) given number of blocks.
     * @param in Input data.
     * @param blockCount Number of data blocks to process.
     * @param out Output buffer.
     */
    virtual void transform(const void* in, std::size_t blockCount, void* out) const noexcept = 0;

protected:
    /** Cipher which this context belongs to */
    const ConstCipherPtr m_cipher;

    /** Block size in bytes */
    const unsigned m_blockSizeInBytes;
};

}  // namespace siodb::iomgr::dbengine::crypto
