// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RandomGenerator.h"

// Project headers
#include "OpenSslError.h"
#include "../io/FileIO.h"
#include "../utils/FileDescriptorGuard.h"
#include "../utils/SystemError.h"

// OpenSSL headers
#include <openssl/rand.h>

namespace siodb::crypto {

RandomGenerator::RandomGenerator()
{
    while (!RAND_status()) {
        FileDescriptorGuard fd(::open("/dev/urandom", O_RDONLY));
        if (!fd.isValidFd()) throw std::runtime_error("Can't open /dev/urandom");
        std::uint8_t rdata[32];
        if (::readExact(fd.getFd(), rdata, sizeof(rdata), kIgnoreSignals) != sizeof(rdata))
            throw std::runtime_error("Can't read data from /dev/urandom");
        RAND_seed(&rdata, sizeof(rdata));
    }
}

void RandomGenerator::getRandomBytes(unsigned char* data, std::size_t size) const
{
    if (RAND_bytes(data, size) != 1) throw OpenSslError("RAND_bytes failed");
}

}  // namespace siodb::crypto
