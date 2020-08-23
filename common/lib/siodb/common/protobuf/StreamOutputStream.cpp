// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "StreamOutputStream.h"

// Common project headers
#include "../stl_ext/system_error_ext.h"

namespace siodb::protobuf {

StreamOutputStream::StreamOutputStream(
        io::OutputStream& stream, const utils::ErrorCodeChecker& errorCodeChecker, int blockSize)
    : m_copyingOutput(stream, errorCodeChecker)
    , m_impl(&m_copyingOutput, blockSize)
{
}

bool StreamOutputStream::Next(void** data, int* size)
{
    return m_impl.Next(data, size);
}

void StreamOutputStream::BackUp(int count)
{
    m_impl.BackUp(count);
}

google::protobuf::int64 StreamOutputStream::ByteCount() const
{
    return m_impl.ByteCount();
}

void StreamOutputStream::CheckNoError() const
{
    if (GetErrno() != 0) stdext::throw_system_error(GetErrno(), "Write error");
}

StreamOutputStream::CopyingOutputStream::CopyingOutputStream(
        io::OutputStream& stream, const utils::ErrorCodeChecker& errorCodeChecker)
    : m_errorCodeChecker(errorCodeChecker)
    , m_stream(stream)
    , m_errno(0)
{
}

bool StreamOutputStream::CopyingOutputStream::Write(const void* buffer, int size)
{
    int result = 0;
    do {
        result = m_stream.write(buffer, size);
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
