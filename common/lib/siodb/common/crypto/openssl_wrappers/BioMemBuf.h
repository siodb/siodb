// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../../utils/HelperMacros.h"
#include "../OpenSslError.h"

// OpenSSL headers
#include <openssl/bio.h>

namespace siodb::crypto {

/** Wrapper for the OpenSSL BIO memory buffer. */
class BioMemBuf {
public:
    /**
     * Initializes object of class BioMemBuf.
     * @param buffer Data buffer.
     * @param size Data buffer size.
     * @throw OpenSslError if context could not be created.
     */
    BioMemBuf(const void* buffer, std::size_t size)
        : m_bio(::BIO_new_mem_buf(buffer, size))
    {
        if (!m_bio) throw OpenSslError("BIO_new_mem_buf failed");
    }

    DECLARE_NONCOPYABLE(BioMemBuf);

    /** Deinitializes object. */
    ~BioMemBuf()
    {
        ::BIO_free(m_bio);
    }

    /**
     * Return mutable underlying BIO object.
     * @return BIO pointer.
     */
    operator ::BIO*() noexcept
    {
        return m_bio;
    }

    /**
     * Returns read-only underlying BIO object.
     * @return const BIO pointer.
     */
    operator const ::BIO*() const noexcept
    {
        return m_bio;
    }

private:
    /** BIO */
    ::BIO* m_bio;
};

}  // namespace siodb::crypto
