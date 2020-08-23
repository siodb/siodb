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

/** Wrapper for the OpenSSL EC_POINT. */
class EcPoint {
public:
    /**
     * Initializes object of class EcPoint.
     * @param group Group.
     * @throw OpenSslError if EC_POINT could not be created.
     */
    explicit EcPoint(const ::EC_GROUP* group)
        : m_ecPoint(::EC_POINT_new(group))
    {
        if (!m_ecPoint) throw OpenSslError("EC_POINT_new failed");
    }

    DECLARE_NONCOPYABLE(EcPoint);

    /** De-initializes object. */
    ~EcPoint()
    {
        ::EC_POINT_clear_free(m_ecPoint);
    }

    /**
     * Returns underlying mutable EC_POINT object.
     * @return EC_POINT pointer.
     */
    operator ::EC_POINT *() noexcept
    {
        return m_ecPoint;
    }

    /**
     * Returns underlying read-only EC_POINT object.
     * @return Const EC_POINT pointer.
     */
    operator const ::EC_POINT *() const noexcept
    {
        return m_ecPoint;
    }

    /**
     * Releases underlying object without freeing memory.
     * @return Released EC_POINT.
     */
    ::EC_POINT* release() noexcept
    {
        auto ecPoint = m_ecPoint;
        m_ecPoint = nullptr;
        return ecPoint;
    }

private:
    /** Ecliptic curve point */
    ::EC_POINT* m_ecPoint;
};

}  // namespace siodb::crypto
