// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TlsServer.h"

// Project headers
#include "OpenSslError.h"
#include "../log/Log.h"
#include "../utils/FdGuard.h"

// STL headers
#include <algorithm>

// CRT headers
#include <cstring>

namespace siodb::crypto {

namespace {
const std::array<const char*, 2> kAsn1SertExtensions = {".der", ".crt"};

int getFileTypeFromFileName(const char* fileName) noexcept
{
    const auto extension = std::strrchr(fileName, '.');

    if (extension != nullptr) {
        const auto dsaIt = std::find_if(kAsn1SertExtensions.begin(), kAsn1SertExtensions.end(),
                [extension](const char* str) noexcept { return std::strcmp(extension, str) == 0; });
        if (dsaIt != kAsn1SertExtensions.end()) return X509_FILETYPE_ASN1;
    }

    return X509_FILETYPE_PEM;
}

int errorPasswordCallback([[maybe_unused]] char* buf, [[maybe_unused]] int size,
        [[maybe_unused]] int rwflag, [[maybe_unused]] void* u)
{
    // -1 means error
    return -1;
}

}  // namespace

TlsServer::TlsServer()
    : m_sslContext(getSslMethod())
{
    ::SSL_CTX_set_default_passwd_cb(m_sslContext, errorPasswordCallback);
}

void TlsServer::useCertificate(const char* certificateFile)
{
    if (::SSL_CTX_use_certificate_file(
                m_sslContext, certificateFile, getFileTypeFromFileName(certificateFile))
            <= 0)
        throw OpenSslError("SSL_CTX_use_certificate_file failed");
}

void TlsServer::useCertificateChain(const char* certificateChainFile)
{
    if (::SSL_CTX_use_certificate_chain_file(m_sslContext, certificateChainFile) <= 0)
        throw OpenSslError("SSL_CTX_use_certificate_chain_file failed");
}

void TlsServer::usePrivateKey(const char* privateKeyFile)
{
    if (::SSL_CTX_use_PrivateKey_file(
                m_sslContext, privateKeyFile, getFileTypeFromFileName(privateKeyFile))
            <= 0) {
        throw OpenSslError("SSL_CTX_use_PrivateKey_file failed");
    }
    if (!::SSL_CTX_check_private_key(m_sslContext))
        throw OpenSslError("OpenSsl private key file is invalid");
}

void TlsServer::setClientCAList(const char* certificateChainFile)
{
    auto clientCAs = ::SSL_load_client_CA_file(certificateChainFile);
    if (clientCAs == nullptr) throw OpenSslError("SSL_load_client_CA_file failed");
    ::SSL_CTX_set_client_CA_list(m_sslContext, clientCAs);
}

std::unique_ptr<TlsConnection> TlsServer::acceptConnection(int fd, bool autoCloseFd)
{
    FdGuard guard(autoCloseFd ? fd : -1);
    auto result = std::make_unique<TlsConnection>(
            m_sslContext, fd, TlsConnectionType::kServer, autoCloseFd);
    guard.release();
    return result;
}

const ::SSL_METHOD* TlsServer::getSslMethod() const
{
    const ::SSL_METHOD* method = ::TLS_server_method();
    if (method == nullptr) throw OpenSslError("TLS_server_method returned nullptr");
    return method;
}

}  // namespace siodb::crypto
