// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../../utils/HelperMacros.h"
#include "../OpenSslError.h"

// OpenSSL headers
#include <openssl/rsa.h>

namespace siodb::crypto {

/** Wrapper for the OpenSSL RSA. */
class RsaKey {
public:
    /**
     * Initializes object of class RsaKey.
     * @throw OpenSslError if RSA could not be created.
     */
    RsaKey()
        : m_rsaKey(RSA_new())
    {
        if (!m_rsaKey) throw OpenSslError("RSA_new failed");
    }

    DECLARE_NONCOPYABLE(RsaKey);

    /**
     * Deinitializes object.
     */
    ~RsaKey()
    {
        RSA_free(m_rsaKey);
    }

    /**
     * Converts class into RSA*.
     * @return RSA pointer.
     */
    operator RSA*() noexcept
    {
        return m_rsaKey;
    }

    /**
     * Converts class into const RSA*.
     * @return Const RSA pointer.
     */
    operator const RSA*() const noexcept
    {
        return m_rsaKey;
    }

    /**
     * Releases pointer without freeing memory
     * @return Released RSA.
     */
    RSA* release() noexcept
    {
        auto rsa = m_rsaKey;
        m_rsaKey = nullptr;
        return rsa;
    }

private:
    /** Key */
    RSA* m_rsaKey;
};

}  // namespace siodb::crypto