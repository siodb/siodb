// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "TlsConnection.h"

// STL headers
#include <memory>

namespace siodb::crypto {

/**
 * TLS client
 */
class TlsClient final {
public:
    /**
     * Initializes object of class TlsClient.
     * @throw OpenSslError in case of OpenSsl error.
     */
    TlsClient()
        : m_sslContext(getSslMethod())
    {
    }

    DECLARE_NONCOPYABLE(TlsClient);

    /**
     * Connects to the server.
     * @param fd File descriptor.
     * @return Client connection.
     * @throw OpenSslError in case of SSL error.
     */
    std::unique_ptr<TlsConnection> connectToServer(int fd)
    {
        return std::make_unique<TlsConnection>(m_sslContext, fd, TlsConnectionType::kClient, true);
    }

    /**
     * Enables server certificate verification.
     */
    void enableCertificateVerification() noexcept
    {
        SSL_CTX_set_verify(m_sslContext, SSL_VERIFY_PEER, NULL);
    }

private:
    /**
     * Returns TLS method.
     * @return OpenSsl TLS method.
     * @throw OpenSslError in case of OpenSsl error.
     */
    const SSL_METHOD* getSslMethod() const;

private:
    /** OpenSsl context */
    SslContext m_sslContext;
};

}  // namespace siodb::crypto
