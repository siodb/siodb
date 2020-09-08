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
    , m_isOpen(true)
{
}

bool StreamOutputStream::Next(void** data, int* size)
{
    if (m_isOpen) return m_impl.Next(data, size);
    return false;
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

bool StreamOutputStream::isValid() const noexcept
{
    return m_copyingOutput.GetErrno() == 0;
}

int StreamOutputStream::close() noexcept
{
    if (m_isOpen) {
        bool result;
        try {
            result = Flush();
        } catch (...) {
            // Ignore exceptions, but indicate failure
            m_isOpen = false;
            errno = ENOMEM;
            return -1;
        }
        m_isOpen = false;
        if (!result) errno = m_copyingOutput.GetErrno();
        return result ? 0 : -1;
    }
    errno = EIO;
    return -1;
}

std::ptrdiff_t StreamOutputStream::write(const void* buffer, std::size_t size) noexcept
{
    if (!m_isOpen) {
        errno = EIO;
        return -1;
    }
    void* streamBuffer = nullptr;
    int streamBufferSize = 0;
    auto remaining = size;
    while (true) {
        bool hasNext;
        try {
            hasNext = Next(&streamBuffer, &streamBufferSize);
        } catch (...) {
            // Ignore exceptions but indicate failure
            errno = ENOMEM;
            break;
        }
        if (hasNext) {
            if (streamBufferSize > 0) {
                const auto streamBufferSizeU = static_cast<std::size_t>(streamBufferSize);
                const auto bytesToWrite = std::min(remaining, streamBufferSizeU);
                std::memcpy(streamBuffer, buffer, bytesToWrite);
                remaining -= bytesToWrite;
                if (remaining == 0) {
                    if (bytesToWrite < streamBufferSizeU) {
                        // Hopefully, will not throw
                        BackUp(static_cast<int>(streamBufferSizeU - bytesToWrite));
                    }
                    break;
                }
                buffer = static_cast<const std::uint8_t*>(buffer) + bytesToWrite;
            }
        } else {
            errno = m_copyingOutput.GetErrno();
            break;
        }
    }
    return size - remaining;
}

//// ---- class StreamOutputStream::CopyingOutputStream -----

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
        // Write error (not EOF).
        m_errno = errno;
    }

    return result;
}

}  // namespace siodb::protobuf
