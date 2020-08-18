// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SiodbProtobufOutputStream.h"

namespace siodb::protobuf {

SiodbProtobufOutputStream::SiodbProtobufOutputStream(
        io::IODevice& device, const utils::ErrorCodeChecker& errorCodeChecker, int blockSize)
    : m_copyingOutput(device, errorCodeChecker)
    , m_impl(&m_copyingOutput, blockSize)
{
}

bool SiodbProtobufOutputStream::Next(void** data, int* size)
{
    return m_impl.Next(data, size);
}

void SiodbProtobufOutputStream::BackUp(int count)
{
    m_impl.BackUp(count);
}

google::protobuf::int64 SiodbProtobufOutputStream::ByteCount() const
{
    return m_impl.ByteCount();
}

SiodbProtobufOutputStream::CopyingOutputStream::CopyingOutputStream(
        io::IODevice& device, const utils::ErrorCodeChecker& errorCodeChecker)
    : m_errorCodeChecker(errorCodeChecker)
    , m_device(device)
    , m_errno(0)
{
}

bool SiodbProtobufOutputStream::CopyingOutputStream::Write(const void* buffer, int size)
{
    int result = 0;
    do {
        result = m_device.write(buffer, size);
    } while (
            result < 0
            // Here is our custom logic comes into the game.
            // Now, depending on external condition, we, for example, can treat EINTR as real error.
            && !m_errorCodeChecker.isError(errno));

    if (result < 0) {
        // Read error (not EOF).
        m_errno = errno;
    }

    return result;
}

}  // namespace siodb::protobuf
