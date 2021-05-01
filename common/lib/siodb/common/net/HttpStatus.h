// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

namespace siodb::net {

/**
 * HTTP status codes enumeration.
 *
 * NOTE: We want this to be assignable to "int" without an explicit typecast,
 * that's why we don't use "enum class" here.
 */
struct HttpStatus {
    enum {
        kOk = 200,
        kCreated = 201,
        kBadRequest = 400,
        kUnauthorized = 401,
        kForbidden = 403,
        kNotFound = 404,
        kInternalServerError = 500
    };
};

}  // namespace siodb::net
