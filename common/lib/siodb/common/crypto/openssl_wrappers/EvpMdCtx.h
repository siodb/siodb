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

/** Wrapper class for the OpenSSL EVP_MD_CTX. */
class EvpMdCtx {
public:
    /**
     * Initializes object of class EvpMdCtx.
     * @throw OpenSslError if context could not be created
     */
    EvpMdCtx()
        : m_ctx(::EVP_MD_CTX_create())
    {
        if (!m_ctx) throw OpenSslError("EVP_MD_CTX_create failed");
    }

    DECLARE_NONCOPYABLE(EvpMdCtx);

    /** Deinitializes object. */
    ~EvpMdCtx()
    {
        ::EVP_MD_CTX_destroy(m_ctx);
    }

    /**
     * Converts class into EVP_MD_CTX*.
     * @return Context.
     */
    operator ::EVP_MD_CTX*() noexcept
    {
        return m_ctx;
    }

    /**
     * Converts class into const EVP_MD_CTX*.
     * @return Const context.
     */
    operator const ::EVP_MD_CTX*() const noexcept
    {
        return m_ctx;
    }

private:
    /** Context */
    ::EVP_MD_CTX* m_ctx;
};

}  // namespace siodb::crypto
