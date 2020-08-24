// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Stream.h"

namespace siodb::io {

/** Common interface for the input stream classes. */
class InputStream : virtual public Stream {
public:
    /**
     * Reads data from stream.
     * @param buffer Data buffer.
     * @param size Size of data in bytes.
     * @return Number of read bytes. Negative value indicates error.
     */
    virtual std::ptrdiff_t read(void* buffer, std::size_t size) = 0;

    /**
     * Skips data.
     * @param size Number of bytes to skip.
     * @return Number of bytes skipped. Negative value indicates error.
     */
    virtual std::ptrdiff_t skip(std::size_t size) = 0;
};

}  // namespace siodb::io
