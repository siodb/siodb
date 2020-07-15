// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <stdexcept>

namespace siodb::config {

/** Thrown to indicate that database configuration contains option with invalid value */
class InvalidConfigurationError : public std::logic_error {
public:
    /**
     * Initializes object of class InvalidConfigurationError.
     * @param what Explanatory message.
     */
    explicit InvalidConfigurationError(const char* what)
        : std::logic_error(what)
    {
    }

    /**
     * Initializes object of class InvalidConfigurationError.
     * @param what Explanatory message.
     */
    explicit InvalidConfigurationError(const std::string& what)
        : std::logic_error(what)
    {
    }
};

}  // namespace siodb::config
