// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BufferedOutputStream.h"

// Common project headers
#include "../stl_ext/buffer.h"
#include "../utils/HelperMacros.h"

namespace siodb::io {

/** Chunked output wrapper over another output stream. */
class ChunkedOutputStream : public BufferedOutputStream {
public:
    /**
     * Initializes object of class ChunkedOutputStream.
     * @param maxChunkSize Maximum chunk size.
     * @param stream Underlying stream.
     */
    ChunkedOutputStream(std::size_t maxChunkSize, OutputStream& stream);

    /** De-initialized object of class ChunkedOutputStream */
    ~ChunkedOutputStream();

    /**
     * Closes stream.
     * @return Zero on success, nonzero otherwise.
     */
    int close() override;

private:
    /**
     * Callback function which is called before flusing data from the internal buffer
     * or writing equivalent amount of unbuffered data.
     * @param size Data size.
     * @return 0 on success, -1 on failure.
     */
    int onFlush(std::size_t dataSize) override;

    /**
     * Writes chunk size to stream.
     * @param chunkSize Chunk size.
     * @return 0 on success, -1 on failure.
     */
    int writeChunkSize(std::size_t chunkSize);
};

}  // namespace siodb::io
