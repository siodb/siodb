// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../../utils/HelperMacros.h"
#include "../OpenSslError.h"

// OpenSSL headers
#include <openssl/dsa.h>

namespace siodb::crypto {

/** Wrapper for the OpenSSL DSA. */
class DsaKey {
public:
    /**
     * Initializes object of class DsaKey.
     * @throw OpenSslError if DSA could not be created.
     */
    DsaKey()
        : m_dsaKey(::DSA_new())
    {
        if (!m_dsaKey) throw OpenSslError("DSA_new failed");
    }

    DECLARE_NONCOPYABLE(DsaKey);

    /** Deinitializes object. */
    ~DsaKey()
    {
        ::DSA_free(m_dsaKey);
    }

    /**
     * Converts class into DSA*.
     * @return DSA pointer.
     */
    operator ::DSA*() noexcept
    {
        return m_dsaKey;
    }

    /**
     * Converts class into const DSA*.
     * @return Const DSA pointer.
     */
    operator const ::DSA*() const noexcept
    {
        return m_dsaKey;
    }

    /**
     * Releases pointer without freeing memory
     * @return Released DSA.
     */
    ::DSA* release() noexcept
    {
        auto dsa = m_dsaKey;
        m_dsaKey = nullptr;
        return dsa;
    }

private:
    /** Key */
    ::DSA* m_dsaKey;
};

}  // namespace siodb::crypto
