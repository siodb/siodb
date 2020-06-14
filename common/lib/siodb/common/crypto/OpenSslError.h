// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../stl_ext/string_builder.h"

// STL headers
#include <stdexcept>

// OpenSSL headers
#include <openssl/err.h>

namespace siodb::crypto {

/** OpenSSL error. */
class OpenSslError : public std::runtime_error {
public:
    /**
     * Initializes object of class OpenSslError.
     * @param what Explanatory message.
     */
    explicit OpenSslError(const char* what)
        : std::runtime_error(OpenSslError::createErrorText(what, ::ERR_get_error()))
        , m_error(ERR_get_error())
    {
    }

    /**
     * Initializes object of class OpenSslError.
     * @param what Explanatory message.
     * @param errorCode Error code.
     */
    OpenSslError(const char* what, int errorCode)
        : std::runtime_error(OpenSslError::createErrorText(what, errorCode))
        , m_error(errorCode)
    {
    }

    /**
     * Returns error code.
     * @return Error code.
     */
    int getErrorCode() const noexcept
    {
        return m_error;
    }

private:
    /**
     * Creates an error text.
     * @param str User string.
     * @param errorCode OpenSSL error code.
     * @param errorText A buffer for error.
     * @return Created string with an error text.
     */
    static std::string createErrorText(const char* str, int errorCode)
    {
        constexpr std::size_t kMinOpenSSLErrorBufferLength = 120;
        char errorText[kMinOpenSSLErrorBufferLength];
        return stdext::string_builder() << str << ": " << ::ERR_error_string(errorCode, errorText);
    }

private:
    /** OpenSSL error code */
    const int m_error;
};

}  // namespace siodb::crypto
