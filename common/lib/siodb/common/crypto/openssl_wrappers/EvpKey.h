// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../../utils/HelperMacros.h"
#include "../OpenSslError.h"

// OpenSSL headers
#include <openssl/evp.h>

namespace siodb::crypto {

/** Wrapper class for the OpenSSL EVP_PKEY. */
class EvpKey {
public:
    /**
     * Initializes object of class EvpKey.
     * @throw OpenSslError if EVP_PKEY could not be created.
     */
    EvpKey()
        : m_evpKey(::EVP_PKEY_new())
    {
        if (!m_evpKey) throw OpenSslError("EVP_PKEY_new failed");
    }

    /**
     * Initializes object of class EvpKey.
     * @param evpKey Key.
     */
    explicit EvpKey(::EVP_PKEY* evpKey) noexcept
        : m_evpKey(evpKey)
    {
    }

    DECLARE_NONCOPYABLE(EvpKey);

    /** Deinitializes object. */
    ~EvpKey()
    {
        ::EVP_PKEY_free(m_evpKey);
    }

    /**
     * Returns underlying mutable EVP_PKEY object.
     * @return EVP_PKEY ponter.
     */
    operator ::EVP_PKEY*() noexcept
    {
        return m_evpKey;
    }

    /**
     * Returns underlying read-only EVP_PKEY object.
     * @return Const EVP_PKEY pointer.
     */
    operator const ::EVP_PKEY*() const noexcept
    {
        return m_evpKey;
    }

    /**
     * Releases underlying object without freeing memory.
     * @return Released EVP_PKEY.
     */
    ::EVP_PKEY* release() noexcept
    {
        auto key = m_evpKey;
        m_evpKey = nullptr;
        return key;
    }

private:
    /** Key */
    ::EVP_PKEY* m_evpKey;
};

}  // namespace siodb::crypto
