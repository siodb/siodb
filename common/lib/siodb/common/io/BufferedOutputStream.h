// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "OutputStreamWrapperStream.h"

// Common project headers
#include "../stl_ext/buffer.h"

namespace siodb::io {

/** Chunked output wrapper over another output stream. */
class BufferedOutputStream : public OutputStreamWrapperStream {
public:
    /**
     * Initializes object of class BufferedChunkedOutputStream.
     * @param bufferSize Buffer size.
     * @param out Underlying output stream.
     */
    BufferedOutputStream(std::size_t bufferSize, OutputStream& out);

    /** De-initialized object of class BufferedOutputStream */
    ~BufferedOutputStream();

    DECLARE_NONCOPYABLE(BufferedOutputStream);

    /**
     * Returns current buffer size.
     * @return Current buffer size.
     */
    std::size_t getBufferSize() const noexcept
    {
        return m_buffer.size();
    }

    /**
     * Returns current data size in the buffer.
     * @return Current data size in the buffer.
     */
    std::size_t getDataSize() const noexcept
    {
        return m_dataSize;
    }

    /**
     * Closes stream.
     * @return Zero on success, nonzero otherwise.
     */
    int close() noexcept override;

    /**
     * Writes data to stream.
     * @param buffer Data buffer.
     * @param size Data size in bytes.
     * @return Number of written bytes. Negative value indicates error.
     */
    std::ptrdiff_t write(const void* buffer, std::size_t size) noexcept override;

    /**
     * Flushes buffer to an underlying media.
     * @return Number of written bytes on success, negative number otherwise.
     */
    virtual std::ptrdiff_t flush() noexcept;

protected:
    /**
     * Callback function which is called before flusing data from the internal buffer
     * or writing equivalent amount of unbuffered data.
     * @param size Data size.
     * @return 0 on success, -1 on failure.
     */
    virtual int onFlush(std::size_t dataSize) noexcept;

protected:
    /** Chunk buffer */
    stdext::buffer<std::uint8_t> m_buffer;

    /** Buffered data size */
    std::size_t m_dataSize;
};

}  // namespace siodb::io
