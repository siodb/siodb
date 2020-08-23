// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Stream.h"

namespace siodb::io {

/** Common interface for the output stream classes. */
class OutputStream : virtual public Stream {
public:
    /**
     * Writes data to stream.
     * @param buffer Data buffer.
     * @param size Data size in bytes.
     * @return Number of written bytes. Negative value indicates error.
     */
    virtual std::ptrdiff_t write(const void* buffer, std::size_t size) = 0;
};

}  // namespace siodb::io
