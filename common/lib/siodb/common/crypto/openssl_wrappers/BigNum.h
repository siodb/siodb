// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../../utils/HelperMacros.h"
#include "../OpenSslError.h"

// OpenSSL headers
#include <openssl/bn.h>

namespace siodb::crypto {

/** Wrapper for the OpenSSL BIGNUM. */
class BigNum {
public:
    /**
     * Initializes object of class BigNum.
     * @throw OpenSslError if BIGNUM could not be created
     */
    BigNum()
        : m_bigNum(BN_new())
    {
        if (!m_bigNum) throw OpenSslError("BN_new failed");
    }

    /**
     * Initializes object of class BigNum.
     * @throw OpenSslError if BIGNUM could not be created
     */
    BigNum(BigNum&& other) noexcept
        : m_bigNum(other.release())
    {
    }

    DECLARE_NONCOPYABLE(BigNum);

    /**
     * Initializes object of class BigNum.
     * @param bigNumStr Big number string.
     * @param len Length of big number string. 
     * @throw OpenSslError if BIGNUM could not be created
     */
    BigNum(const unsigned char* bigNumStr, int len)
        : m_bigNum(BN_bin2bn(bigNumStr, len, nullptr))
    {
        if (!m_bigNum) throw OpenSslError("BN_bin2bn failed");
    }

    /**
     * Move assignment.
     * @param src Source object.
     */
    BigNum& operator=(BigNum&& src) noexcept
    {
        if (&src != this) {
            m_bigNum = src.m_bigNum;
            src.m_bigNum = nullptr;
        }
        return *this;
    }

    /**
     * Deinitializes object.
     */
    ~BigNum()
    {
        BN_clear_free(m_bigNum);
    }

    /**
     * Converts class into BIGNUM*.
     * @return BIGNUM pointer.
     */
    operator BIGNUM*() noexcept
    {
        return m_bigNum;
    }

    /**
     * Converts class into const BIGNUM*.
     * @return Const BIGNUM pointer.
     */
    operator const BIGNUM*() const noexcept
    {
        return m_bigNum;
    }

    /**
     * Releases pointer without freeing memory
     * @return Released BIGNUM.
     */
    BIGNUM* release() noexcept
    {
        auto bn = m_bigNum;
        m_bigNum = nullptr;
        return bn;
    }

private:
    /** Big number */
    BIGNUM* m_bigNum;
};

}  // namespace siodb::crypto
