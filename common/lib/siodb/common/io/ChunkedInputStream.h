// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "InputStream.h"

namespace siodb::io {

/** Chunked input wrapper over another input stream. */
class ChunkedInputStream : public InputStream {
public:
    /**
     * Initializes object of class ChunkedInputStream.
     * @param stream Underlying stream.
     */
    explicit ChunkedInputStream(InputStream& stream);

    DECLARE_NONCOPYABLE(ChunkedInputStream);

    /**
     * Returns indication that stream is valid.
     * @return true stream if stream is valid, false otherwise.
     */
    bool isValid() const override;

    /**
     * Closes stream.
     * @return Zero on success, nonzero otherwise.
     */
    int close() override;

    /**
     * Reads data from stream.
     * @param buffer Data buffer.
     * @param size Size of data in bytes.
     * @return Number of read bytes. Negative value indicates error.
     */
    std::ptrdiff_t read(void* buffer, std::size_t size) override;

    /**
     * Skips data.
     * @param size Number of bytes to skip.
     * @return Offset from the beginning of stream. Negative value indicates error.
     */
    off_t skip(std::size_t size) override;

private:
    /**
     * Reads chunk size.
     * @return 1 on success, 0 on the end of file, -1 on error.
     */
    int readChunkSize();

private:
    /** Underlying stream */
    InputStream* m_stream;

    /** Position in chunk */
    std::size_t m_pos;

    /** Current chunk size */
    std::size_t m_chunkSize;
};

}  // namespace siodb::io
