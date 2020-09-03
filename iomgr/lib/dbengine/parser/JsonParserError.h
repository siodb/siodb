// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <stdexcept>

namespace siodb::iomgr::dbengine::parser {

/** JSON parsing error exception */
class JsonParserError : public std::runtime_error {
public:
    /**
     * Initializes object of class JsonParserError.
     * @param what Explanatory string.
     */
    explicit JsonParserError(const char* what)
        : std::runtime_error(what)
    {
    }

    /**
     * Initializes object of class JsonParserError.
     * @param what Explanatory string.
     */
    explicit JsonParserError(const std::string& what)
        : std::runtime_error(what)
    {
    }
};

}  // namespace siodb::iomgr::dbengine::parser
