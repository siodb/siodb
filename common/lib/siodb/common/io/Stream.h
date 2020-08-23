// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include "../utils/HelperMacros.h"

// CRT headers
#include <cstddef>

// System headers
#include <sys/types.h>

namespace siodb::io {

/** Common interface for the stream classes. */
class Stream {
protected:
    /** Initializes object of class Stream */
    Stream() noexcept = default;

public:
    /** De-initializes object of class Stream. */
    virtual ~Stream() = default;

    DECLARE_NONCOPYABLE(Stream);

    /**
     * Returns indication that stream is valid.
     * @return true stream if stream is valid, false otherwise.
     */
    virtual bool isValid() const = 0;

    /**
     * Closes stream.
     * @return Zero on success, nonzero otherwise.
     */
    virtual int close() = 0;
};

}  // namespace siodb::io
