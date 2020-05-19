// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <stdexcept>

namespace siodb::utils {

/**
 * Defines a type of object to be thrown as exception when waiting for some condition was interupted.
 */
class WaitInterruptedException : public std::runtime_error {
public:
    /** 
     * Initializes object of class WaitInterruptedException.
     * @param what Explanatory message.
     */
    explicit WaitInterruptedException(const std::string& what)
        : std::runtime_error(what)
    {
    }

    /**
     * Initializes object of class WaitInterruptedException.
     * @param msg Explanatory message.
     */
    explicit WaitInterruptedException(const char* what)
        : std::runtime_error(what)
    {
    }
};

}  // namespace siodb::utils
