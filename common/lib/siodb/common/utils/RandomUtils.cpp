// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RandomUtils.h"

// Project headers
#include "FDGuard.h"
#include "../io/FileIO.h"

// CRT headers
#include <cstring>

// STL headers
#include <sstream>
#include <stdexcept>

namespace siodb::utils {

namespace {

void getRandomBytesImpl(const char* devicePath, std::uint8_t* buffer, std::size_t length)
{
    FDGuard fd(::open(devicePath, O_RDONLY));
    if (!fd.isValidFd()) {
        int errorCode = errno;
        std::ostringstream err;
        err << "Can't open " << devicePath << " for reading: " << std::strerror(errorCode);
        throw std::runtime_error(err.str());
    }
    const auto n = ::readExact(fd.getFD(), buffer, length, kIgnoreSignals);
    if (n != length) {
        int errorCode = errno;
        std::ostringstream err;
        err << "Can't read from the " << devicePath << ": " << std::strerror(errorCode)
            << ", was about to read " << length << ", but received " << n;
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
