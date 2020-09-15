// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "OutputStream.h"

// CRT headers
#include <cstdint>

namespace siodb::io {

/** Base class for the output stream classes which wrap another output stream. */
class OutputStreamWrapperStream : public OutputStream {
protected:
    /** 
     * Initialized object of class OutputStreamWrapperStream.
     * @param out Underlying output stream.
     */
    explicit OutputStreamWrapperStream(OutputStream& out) noexcept
        : m_out(&out)
    {
    }

public:
    /** De-initialized object of class OutputStreamWrapperStream */
    ~OutputStreamWrapperStream();

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
    /**
     * Writes chunk size to stream.
     * @param chunkSize Chunk size.
     * @return 0 on success, -1 on failure.
     */
    int writeChunkSize(std::uint32_t chunkSize) noexcept;

protected:
    /** Underlying output stream */
    OutputStream* m_out;
};

}  // namespace siodb::io
