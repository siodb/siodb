// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <stdexcept>

namespace stdext {

/** Thrown when waiting for some condition was interupted. */
class wait_interrupted_error : public std::runtime_error {
public:
    /** 
     * Initializes object of class wait_interrupted_error.
     * @param what Explanatory message.
     */
    explicit wait_interrupted_error(const std::string& what)
        : std::runtime_error(what)
    {
    }

    /**
     * Initializes object of class wait_interrupted_error.
     * @param msg Explanatory message.
     */
    explicit wait_interrupted_error(const char* what = k_default_message)
        : std::runtime_error(what)
    {
    }

private:
    /** Default explanatory message */
    static constexpr const char* k_default_message = "wait interrupted";
};

}  // namespace stdext
