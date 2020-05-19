// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <stdexcept>

namespace siodb::utils {

/** Thrown when deserialization fails. */
class DeserializationError : public std::runtime_error {
public:
    /**
     * Initializes object of class DeserializartionError.
     * @param what Explanatory message.
     */
    explicit DeserializationError(const char* what)
        : std::runtime_error(what)
    {
    }

    /**
     * Initializes object of class VariantDeserializartionError.
     * @param what Explanatory message.
     */
    explicit DeserializationError(const std::string& what)
        : std::runtime_error(what)
    {
    }
};

}  // namespace siodb::utils
