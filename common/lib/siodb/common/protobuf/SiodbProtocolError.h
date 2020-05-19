// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// STL headers
#include <stdexcept>

namespace siodb {

/** Thrown to indicate communication protocol error */
class SiodbProtocolError : public std::runtime_error {
public:
    /**
     * Initializes object of the class SiodbProtocolError.
     * @param what Explanatory message.
     */
    explicit SiodbProtocolError(const char* what)
        : std::runtime_error(what)
    {
    }

    /**
     * Initializes object of the class SiodbProtocolError.
     * @param what Explanatory message.
     */
    explicit SiodbProtocolError(const std::string& what)
        : std::runtime_error(what)
    {
    }
};

}  // namespace siodb
