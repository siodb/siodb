// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TlsConnection.h"

// Project headers
#include "../utils/FileDescriptorGuard.h"

// System headers
#include <unistd.h>

namespace siodb::crypto {

TlsConnection::TlsConnection(
        SslContext& context, int fd, TlsConnectionType connectionType, bool autoCloseFd)
    : m_ssl(context)
    , m_autoCloseFd(autoCloseFd)
{
    FileDescriptorGuard guard(m_autoCloseFd ? fd : -1);
    if (SSL_set_fd(m_ssl, fd) != 1) throw OpenSslError("SSL_set_fd failed");
    guard.release();

    if (connectionType == TlsConnectionType::kServer)
        m_ssl.accept();
    else
        m_ssl.connect();
}

TlsConnection::TlsConnection(TlsConnection&& src) noexcept
    : m_ssl(std::move(src.m_ssl))
    , m_autoCloseFd(src.m_autoCloseFd)
{
    src.m_autoCloseFd = false;
}

TlsConnection::~TlsConnection()
{
    if (m_ssl.isConnected()) {
        const auto fd = SSL_get_fd(m_ssl);
        m_ssl.close();
        if (m_autoCloseFd) ::close(fd);
    }
}

std::size_t TlsConnection::write(const void* data, std::size_t size)
{
    int result = SSL_write(m_ssl, data, size);
    if (result < 0) OpenSslError("SSL_write failed");
    return result;
}

std::size_t TlsConnection::read(void* data, std::size_t size)
{
    int result = SSL_read(m_ssl, data, size);
    if (result < 0) OpenSslError("SSL_read failed");
    return result;
}

off_t TlsConnection::skip(std::size_t size)
{
    char buffer[4096];
    std::size_t remainBytes = size;
    while (remainBytes > 0) {
        const auto n = std::min(remainBytes, sizeof(buffer));
        remainBytes -= read(buffer, n);
    }
    return size;
}

int TlsConnection::close()
{
    const auto fd = SSL_get_fd(m_ssl);
    const auto result = m_ssl.close();
    if (result != 1) OpenSslError("SSL_close failed");
    if (m_autoCloseFd) return ::close(fd);
    return 0;
}

bool TlsConnection::isValid() const
{
    return m_ssl.isConnected();
}

}  // namespace siodb::crypto
