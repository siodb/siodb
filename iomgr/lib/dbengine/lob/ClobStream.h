// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "LobStream.h"

// STL headers
#include <string>

namespace siodb::iomgr::dbengine {

/** Base class for all Character LOB streams. */
class ClobStream : virtual public LobStream {
protected:
    /**
     * Initializes object of class ClobStream.
     * @param size Stream size.
     */
    explicit ClobStream(std::uint32_t size)
        : LobStream(size)
    {
    }

public:
    /**
     * Creates copy of this stream.
     * @return copy of this stream ot nullptr of cloning of stream is not possible.
     */
    ClobStream* clone() const override;

    /**
     * Reads part of CLOB as into a string.
     * @param length Length of CLOB part to receive.
     * @return A string containing part of CLOB of size up to "length".
     * @throw runtime_error if CLOB reading error occurs
     */
    std::string readAsString(std::uint32_t length);
};

}  // namespace siodb::iomgr::dbengine
