// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../../utils/HelperMacros.h"
#include "../OpenSslError.h"

// OpenSSL headers
#include <openssl/ssl.h>

namespace siodb::crypto {

/** Wrapper for the OpenSSL SSL object. */
class Ssl {
public:
    /**
     * Initializes object of class Ssl.
     * @param sslContext SSL context.
     * @throw OpenSslError if SSL object could not be created.
     */
    Ssl(SSL_CTX* sslContext)
        : m_ssl(SSL_new(sslContext))
    {
        if (!m_ssl) throw OpenSslError("SSL_new failed");
    }

    /**
     * Initializes object of class Ssl.
     * @param other Other ssl object.
     */
    Ssl(Ssl&& other) noexcept
        : m_ssl(other.m_ssl)
        , m_connectionActive(other.m_connectionActive)
    {
        other.m_ssl = nullptr;
        other.m_connectionActive = false;
    }

    DECLARE_NONCOPYABLE(Ssl);

    /**
     * Deinitializes object.
     */
    ~Ssl()
    {
        if (m_ssl != nullptr) {
            if (m_connectionActive) SSL_shutdown(m_ssl);

            SSL_free(m_ssl);
        }
    }

    /**
     * Converts class into SSL*.
     * @return SSL pointer.
     */
    operator SSL*() noexcept
    {
        return m_ssl;
    }

    /**
     * Converts class into const SSL*.
     * @return Const SSL pointer.
     */
    operator const SSL*() const noexcept
    {
        return m_ssl;
    }

    /**
     * Releases pointer without freeing memory
     * @return Released SSL.
     */
    SSL* release() noexcept
    {
        auto ssl = m_ssl;
        m_ssl = nullptr;
        return ssl;
    }

    /**
     * Accepts connection from client.
     */
    void accept()
    {
        if (SSL_accept(m_ssl) == 1)
            m_connectionActive = true;
        else
            throw OpenSslError("SSL_accept failed");
    }

    /**
     * Connects client to a server.
     */
    void connect()
    {
        if (SSL_connect(m_ssl) == 1)
            m_connectionActive = true;
        else
            throw OpenSslError("SSL_connect failed");
    }

    /**
     * Closes connection.
     * @return 0 in case of success, false otherwise.
     */
    int close() noexcept
    {
        auto result = SSL_shutdown(m_ssl);
        if (result == 1) m_connectionActive = false;
        return result;
    }

    /**
     * Returns indication whether connection active.
     * @return true in case of active connection, false othewise.
     */
    bool isConnected() const noexcept
    {
        return m_connectionActive;
    }

private:
    /** SSL object */
    SSL* m_ssl;

    /** Indication that connection is active */
    bool m_connectionActive;
};

}  // namespace siodb::crypto