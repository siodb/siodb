// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RandomUtils.h"

// Project headers
#include "FdGuard.h"
#include "../io/FileIO.h"

// CRT headers
#include <cstring>

// STL headers
#include <sstream>
#include <stdexcept>

namespace siodb::utils {

namespace {

void getRandomBytesImpl(const char* device, std::uint8_t* buffer, std::size_t length)
{
    FdGuard fd(::open(device, O_RDONLY));
    if (!fd.isValidFd()) {
        int errorCode = errno;
        std::ostringstream err;
        err << "Can't open " << device << " for reading: " << std::strerror(errorCode);
        throw std::runtime_error(err.str());
    }
    if (::readExact(fd.getFd(), buffer, length, kIgnoreSignals) != length) {
        int errorCode = errno;
        std::ostringstream err;
        err << "Can't read from the " << device << ": " << std::strerror(errorCode);
        throw std::runtime_error(err.str());
    }
}

}  // namespace

void getRandomBytes(std::uint8_t* buffer, std::size_t length)
{
    getRandomBytesImpl("/dev/urandom", buffer, length);
}

void getStrongRandomBytes(std::uint8_t* buffer, std::size_t length)
{
    getRandomBytesImpl("/dev/random", buffer, length);
}

}  // namespace siodb::utils
