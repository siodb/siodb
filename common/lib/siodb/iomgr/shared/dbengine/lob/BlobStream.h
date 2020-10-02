// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "LobStream.h"

// Common project headers
#include <siodb/common/utils/BinaryValue.h>

namespace siodb::iomgr::dbengine {

/** Base class for all Binary LOB streams. */
class BlobStream : virtual public LobStream {
protected:
    /**
     * Initializes object of class BlobStream.
     * @param size Stream size.
     */
    explicit BlobStream(std::uint32_t size) noexcept
        : LobStream(size)
    {
    }

public:
    /**
     * Throws exception because this stream can't be cloned.
     * @throw std::logic_error always.
     */
    BlobStream* clone() const override;

    /**
     * Reads part of BLOB as into a buffer.
     * @param length Length of BLOB part to receive.
     * @return A buffer containing part of BLOB of size up to "length".
     * @throw runtime_error if BLOB reading error occurs
     */
    BinaryValue readAsBinary(std::uint32_t length);
};

}  // namespace siodb::iomgr::dbengine
