// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TlsConnection.h"

// Project headers
#include "../utils/FDGuard.h"

// System headers
#include <unistd.h>

namespace siodb::crypto {

TlsConnection::TlsConnection(
        SslContext& context, int fd, TlsConnectionType connectionType, bool autoCloseFd)
    : m_ssl(context)
    , m_autoCloseFd(autoCloseFd)
{
    FDGuard guard(m_autoCloseFd ? fd : -1);
    if (::SSL_set_fd(m_ssl, fd) != 1) throw OpenSslError("SSL_set_fd failed");
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
    close();
}

bool TlsConnection::isValid() const noexcept
{
    return m_ssl.isConnected();
}

int TlsConnection::close() noexcept
{
    if (m_ssl.isConnected()) {
        const auto fd = ::SSL_get_fd(m_ssl);
        const auto result = m_ssl.close();
        if (m_autoCloseFd) return ::close(fd);
        if (result < 0) errno = EIO;
        return result;
    }
    errno = EIO;
    return -1;
}

std::ptrdiff_t TlsConnection::read(void* data, std::size_t size) noexcept
{
    const auto n = ::SSL_read(m_ssl, data, size);
    if (n < 0) errno = EIO;
    return n;
}

std::ptrdiff_t TlsConnection::write(const void* data, std::size_t size) noexcept
{
    const auto n = ::SSL_write(m_ssl, data, size);
    if (n < 0) errno = EIO;
    return n;
}

}  // namespace siodb::crypto
