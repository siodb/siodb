// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../../utils/HelperMacros.h"
#include "../OpenSslError.h"

// OpenSSL headers
#include <openssl/ec.h>

namespace siodb::crypto {

/** Wrapper for the OpenSSL EC_KEY. */
class EcKey {
public:
    /**
     * Initializes object of class EcKey.
     * @param curveId Curve ID.
     * @throw OpenSslError if EC_KEY could not be created.
     */
    explicit EcKey(int curveId)
        : m_ecKey(::EC_KEY_new_by_curve_name(curveId))
    {
        if (!m_ecKey) throw OpenSslError("EC_KEY_new_by_curve_name failed");
    }

    DECLARE_NONCOPYABLE(EcKey);

    /** De-initializes object. */
    ~EcKey()
    {
        ::EC_KEY_free(m_ecKey);
    }

    /**
     * Returns underlying read-only EC_KEY object.
     * @return EC_KEY pointer.
     */
    operator ::EC_KEY *() noexcept
    {
        return m_ecKey;
    }

    /**
     * Returns underlying read-only EC_KEY object.
     * @return Const EC_KEY pointer.
     */
    operator const ::EC_KEY *() const noexcept
    {
        return m_ecKey;
    }

    /**
     * Releases underlying object without freeing memory.
     * @return Released EC_KEY.
     */
    ::EC_KEY* release() noexcept
    {
        auto ecKey = m_ecKey;
        m_ecKey = nullptr;
        return ecKey;
    }

private:
    /** Ecliptic curve key */
    ::EC_KEY* m_ecKey;
};

}  // namespace siodb::crypto
