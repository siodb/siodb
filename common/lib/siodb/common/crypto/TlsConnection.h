// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "openssl_wrappers/Ssl.h"
#include "openssl_wrappers/SslContext.h"

// Common project headers
#include "../io/InputOutputStream.h"

namespace siodb::crypto {

/** Connection type */
enum class TlsConnectionType {
    kServer,
    kClient,
};

/** TLS connection */
class TlsConnection final : public io::InputOutputStream {
public:
    /**
     * Initializes object of class TlsConnection.
     * @param context OpenSsl context.
     * @param fd File descriptor.
     * @param connectionType Connection type.
     * @param autoCloseFd Indication that connection file descriptor must be closed
     *                    after closing secure connection.
     * @throw OpenSslError if OpenSSL error happens.
     */
    TlsConnection(SslContext& context, int fd, TlsConnectionType connectionType, bool autoCloseFd);

    /**
     * Initializes object of class TlsConnection.
     * @param src Other TLS connection object.
     */
    TlsConnection(TlsConnection&& src) noexcept;

    /**
     * De-initializes object of class TlsConnection.
     * Closes secure connection.
     */
    ~TlsConnection();

    /**
     * Returns SSL connection object.
     * @return SSL connection object.
     */
    const Ssl& getSsl() const noexcept
    {
        return m_ssl;
    }

    /**
     * Returns indication whether connection is valid or not.
     * @return true if connection is valid false otherwise.
     */
    bool isValid() const noexcept override;

    /**
     * Reads data from secure channel.
     * @param buffer Data buffer.
     * @param size Data size.
     * @return Number of read bytes. Negative values indicate error.
     */
    std::ptrdiff_t read(void* buffer, std::size_t size) override;

    /**
     * Writes data to secure channel.
     * @param buffer Data buffer.
     * @param size Data size.
     * @return Number of written bytes. Negative values indicate error.
     */
    std::ptrdiff_t write(const void* buffer, std::size_t size) override;

    /**
     * Skips data.
     * @param size Number of bytes to skip.
     * @return Number of bytes skipped. Negative value indicates error.
     */
    std::ptrdiff_t skip(std::size_t size) override;

    /**
     * Closes connection.
     * @return 0 on success, nonzero otherwise.
     */
    int close() override;

private:
    /** SSL connection object */
    Ssl m_ssl;

    /** Indication that file descriptor must be closed after closing secure connection. */
    bool m_autoCloseFd;
};

}  // namespace siodb::crypto
