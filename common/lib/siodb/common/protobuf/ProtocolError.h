// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// STL headers
#include <stdexcept>

namespace siodb {

/** Thrown to indicate communication protocol error */
class ProtocolError : public std::runtime_error {
public:
    /**
     * Initializes object of the class ProtocolError.
     * @param what Explanatory message.
     */
    explicit ProtocolError(const char* what)
        : std::runtime_error(what)
    {
    }

    /**
     * Initializes object of the class ProtocolError.
     * @param what Explanatory message.
     */
    explicit ProtocolError(const std::string& what)
        : std::runtime_error(what)
    {
    }
};

}  // namespace siodb
