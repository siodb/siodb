// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BufferedOutputStream.h"

// Common project headers
#include "../stl_ext/buffer.h"

namespace siodb::io {

/** Chunked output wrapper over another output stream. */
class BufferedChunkedOutputStream : public BufferedOutputStream {
public:
    /**
     * Initializes object of class BufferedChunkedOutputStream.
     * @param maxChunkSize Maximum chunk size.
     * @param out Underlying output stream.
     */
    BufferedChunkedOutputStream(std::size_t maxChunkSize, OutputStream& out);

    /** De-initialized object of class BufferedChunkedOutputStream */
    ~BufferedChunkedOutputStream();

    /**
     * Closes stream.
     * @return Zero on success, nonzero otherwise.
     */
    int close() noexcept override;

private:
    /**
     * Callback function which is called before flusing data from the internal buffer
     * or writing equivalent amount of unbuffered data.
     * @param size Data size.
     * @return 0 on success, -1 on failure.
     */
    int onFlush(std::size_t dataSize) noexcept override;

    /**
     * Validates max. chunk size.
     * @param maxChunkSize Max chunk size value.
     * @return maxChunkSize if it is valid
     * @throw std::invalid_argument if maxChunkSize is not valid.
     */
    static std::size_t validateMaxChunkSize(std::size_t maxChunkSize);

private:
    /** Minimum allowed max chunk size */
    static constexpr std::size_t kMinMaxChunkSize = 1;
    /** Maximum allowed max chunk size */
    static constexpr std::size_t kMaxMaxChunkSize = 1024 * 1024 * 1024;
};

}  // namespace siodb::io
