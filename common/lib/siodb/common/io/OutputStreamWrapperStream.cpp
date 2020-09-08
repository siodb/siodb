// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "OutputStreamWrapperStream.h"

// Common project headers
#include "../utils/Base128VariantEncoding.h"

namespace siodb::io {

OutputStreamWrapperStream::~OutputStreamWrapperStream()
{
    if (isValid()) close();
}

bool OutputStreamWrapperStream::isValid() const noexcept
{
    return m_out && m_out->isValid();
}

int OutputStreamWrapperStream::close() noexcept
{
    if (m_out) {
        m_out = nullptr;
        return 0;
    }
    errno = EIO;
    return -1;
}

// ----- internals -----

int OutputStreamWrapperStream::writeChunkSize(std::size_t chunkSize) noexcept
{
    if (!isValid()) {
        errno = EIO;
        return -1;
    }
    std::uint8_t buffer[9];
    const int n = ::encodeVarUInt64(chunkSize, buffer) - buffer;
    if (m_out->write(buffer, n) != n) {
        m_out = nullptr;
        return -1;
    }
    return 0;
}

}  // namespace siodb::io
