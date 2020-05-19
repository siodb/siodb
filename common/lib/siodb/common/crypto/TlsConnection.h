// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "openssl_wrappers/Ssl.h"
#include "openssl_wrappers/SslContext.h"
#include "../io/IoBase.h"

namespace siodb::crypto {

/** Connection type */
enum class TlsConnectionType { kServer, kClient };

/** TLS connection */
class TlsConnection final : public io::IoBase {
public:
    /**
     * Initializes object of class TlsConnection.
     * @param context OpenSsl context.
     * @param fd File descriptor.
     * @param connectionType Connection type.
     * @param autoCloseFd Indication that connection file descriptor should be closed
     * if secure connection will be closed.
     * @throw OpenSslError in case of OpenSsl error.
     */
    TlsConnection(SslContext& context, int fd, TlsConnectionType connectionType, bool autoCloseFd);

    /**
     * Initializes object of class TlsConnection.
     * @param src Other TLS connection object.
     */
    TlsConnection(TlsConnection&& src) noexcept;

    /**
     * Deinitializes object of lcass TlsConnection.
     * Closes secure connection.
     */
    ~TlsConnection() override;

    DECLARE_NONCOPYABLE(TlsConnection);

    /**
     * Writes data to secure channel.
     * @param data Data.
     * @param size Size of data in bytes.
     * @return Count of written bytes.
     * @throw OpenSslError in case of error.
     */
    std::size_t write(const void* data, std::size_t size) override;

    /**
     * Reads data from secure channel.
     * @param data Data.
     * @param size Size of data in bytes.
     * @return Count of read bytes.
     * @throw OpenSslError in case of error.
     */
    std::size_t read(void* data, std::size_t size) override;

    /**
     * Skips data. Not supported for TLS connection
     * @param size Count of bytes to skip. Must be positive number
     * @return Count of bytes skipped.
     * @throw std::runtime_error if size is less than 0.
     */
    off_t skip(std::size_t size) override;

    /**
     * Closes secure connection.
     * @return 0 in case of success, other value means error.
     */
    int close() override;

    /**
     * Returns indication whether connection is valid or not.
     * @return true if connection is valid false otherwise.
     */
    bool isValid() const override;

    /**
     * Returns SSL connection object.
     * @return SSL connection object.
     */
    const Ssl& getSsl() const noexcept
    {
        return m_ssl;
    }

private:
    /** SSL connection object */
    Ssl m_ssl;

    /** 
     * Indication that connection file descriptor should be closed in case of 
     * closing secure connection
     */
    bool m_autoCloseFd;
};

}  // namespace siodb::crypto
