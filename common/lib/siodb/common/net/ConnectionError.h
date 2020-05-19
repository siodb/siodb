// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <stdexcept>

namespace siodb::net {

/** Thrown when connection error happens. */
class ConnectionError : public std::runtime_error {
public:
    /**
     * Initializes object of class ConnectionError.
     * @param what Explanatory message.
     */
    explicit ConnectionError(const std::string& what)
        : std::runtime_error(what)
    {
    }

    /**
     * Initializes object of class ConnectionError.
     * @param what Explanatory message.
     */
    explicit ConnectionError(const char* what)
        : std::runtime_error(what)
    {
    }
};

}  // namespace siodb::net
