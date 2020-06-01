// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

#include <netdb.h>

namespace siodb::net {

/** Guard class to control list of address infos allocated by getaddrinfo() */
struct AddrInfosGuard {
    AddrInfosGuard(struct addrinfo* addrInfos, bool owned)
        : m_addrInfos(addrInfos)
        , m_owned(owned)
    {
    }

    ~AddrInfosGuard()
    {
        if (m_owned) {
            ::freeaddrinfo(m_addrInfos);
        }
    }

    struct addrinfo* m_addrInfos;
    const bool m_owned;
};

}  // namespace siodb::net
