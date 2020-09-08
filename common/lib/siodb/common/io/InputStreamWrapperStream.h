// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "InputStream.h"

namespace siodb::io {

/** Base class for the input stream classes which wrap another input stream. */
class InputStreamWrapperStream : public InputStream {
protected:
    /** 
     * Initialized object of class InputStreamWrapperStream.
     * @param in Underlying input stream.
     */
    explicit InputStreamWrapperStream(InputStream& in) noexcept
        : m_in(&in)
    {
    }

public:
    /** De-initialized object of class InputStreamWrapperStream */
    ~InputStreamWrapperStream();

    /**
     * Returns indication that stream is valid.
     * @return true stream if stream is valid, false otherwise.
     */
    bool isValid() const noexcept override;

    /**
     * Closes stream.
     * @return Zero on success, nonzero otherwise.
     */
    int close() noexcept override;

protected:
    /** Underlying input stream */
    InputStream* m_in;
};

}  // namespace siodb::io
