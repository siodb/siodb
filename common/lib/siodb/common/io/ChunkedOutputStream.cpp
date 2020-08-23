// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ChunkedOutputStream.h"

// Common project headers
#include "../utils/Base128VariantEncoding.h"

namespace siodb::io {

ChunkedOutputStream::ChunkedOutputStream(std::size_t maxChunkSize, OutputStream& stream)
    : BufferedOutputStream(maxChunkSize, stream)
{
}

ChunkedOutputStream::~ChunkedOutputStream()
{
    close();
}

int ChunkedOutputStream::close()
{
    int res = -1;
    if (m_stream) {
        res = flush();
        if (res == 0) res = writeChunkSize(0);
        m_stream = nullptr;
    }
    return res;
}

// ---- internals -----

int ChunkedOutputStream::onFlush(std::size_t dataSize)
{
    return writeChunkSize(dataSize);
}

int ChunkedOutputStream::writeChunkSize(std::size_t chunkSize)
{
    if (!isValid()) return -1;
    std::uint8_t buffer[9];
    const int n = ::encodeVarUInt64(chunkSize, buffer) - buffer;
    if (writeRawData(buffer, n) != n) {
        m_stream = nullptr;
        return -1;
    }
    return 0;
}

}  // namespace siodb::io
