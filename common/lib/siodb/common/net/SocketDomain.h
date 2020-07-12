// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::net {

/**
 * Validates listener socket domain.
 * @param socketDomain TCP/IP socket domain type.
 * @return socketDomain if it is valid.
 * @throw std::invalid_argument if socketDomain is invalid.
 */
int checkSocketDomain(int socketDomain);

/**
 * Validates listener socket domain.
 * @param socketDomain TCP/IP socket domain type.
 * @return socketDomain if it is valid.
 * @throw std::invalid_argument if socketDomain is invalid.
 */
int checkSocketDomainIpOnly(int socketDomain);

/**
 * Returns socket domain name.
 * @param socketDomain TCP/IP socket domain type.
 * @return Socket doman name, if socketDomain is valid.
 * @throw std::invalid_argument if socketDomain is invalid.
 */
const char* getSocketDomainName(int socketDomain);

}  // namespace siodb::net
