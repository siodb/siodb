// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "TlsConnection.h"
#include "openssl_wrappers/Ssl.h"
#include "openssl_wrappers/SslContext.h"

// STL headers
#include <memory>

namespace siodb::crypto {

/** TLS server class */
class TlsServer final {
public:
    /**
     * Initializes object of class TlsServer.
     * @throw OpenSslError in case of OpenSsl error.
     */
    TlsServer();

    DECLARE_NONCOPYABLE(TlsServer);

    /**
     * Loads certificate from a file and use it in server context.
     * @param certificateFile Certificate file path.
     * @throw OpenSslError in case of OpenSsl error.
     */
    void useCertificate(const char* certificateFile);

    /**
     * Loads certificate chain from a file and use it in server context.
     * @param certificateChainFile Certificate chain file path.
     * @throw OpenSslError in case of OpenSsl error.
     */
    void useCertificateChain(const char* certificateChainFile);

    /**
     * Loads private key file and use it in server context.
     * @param privateKeyFile Private key file path.
     * @throw OpenSslError in case of OpenSsl error.
     */
    void usePrivateKey(const char* privateKeyFile);

    /**
     * Loads CA list from a file and use it for connection with the client 
     * @param certificateChainFile Certificate chain file path.
     * @throw OpenSslError in case of OpenSsl error.
     */
    void setClientCAList(const char* certificateChainFile);

    /**
     * Accepts connection from client.
     * @param connectionFd Connection file descriptor.
     * @param autoCloseFd Indication to close connection file 
     * descriptor on TLS connection close.
     * @throw OpenSslError in case of OpenSsl error.
     */
    std::unique_ptr<TlsConnection> acceptConnection(int connectionFd, bool autoCloseFd);

private:
    /**
     * Returns TLS method.
     * @return OpenSsl TLS method.
     * @throw OpenSslError in case of OpenSsl error.
     */
    const ::SSL_METHOD* getSslMethod() const;

private:
    /** Open SSL context */
    SslContext m_sslContext;
};

}  // namespace siodb::crypto
