// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TlsClient.h"

// Project headers
#include "OpenSslError.h"

namespace siodb::crypto {

const SSL_METHOD* TlsClient::getSslMethod() const
{
    const SSL_METHOD* method = TLS_client_method();
    if (method == nullptr) throw OpenSslError("TLS_client_method returned nullptr");
    return method;
}

}  // namespace siodb::crypto