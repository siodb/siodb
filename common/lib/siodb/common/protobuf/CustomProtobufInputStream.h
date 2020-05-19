// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../io/IoBase.h"
#include "../utils/ErrorCodeChecker.h"

// STL headers
#include <memory>

// Protobuf headers
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

namespace siodb::protobuf {

/**
 * Our own version of protobuf file input stream, that allows to handle read errors,
 * including EINTR.
 * A ZeroCopyInputStream which reads from a file descriptor.
 * SignalAwareFileInputStream is preferred over using an ifstream with IstreamInputStream.
 * The latter will introduce an extra layer of buffering, harming performance.
 * Also, it's conceivable that SignalAwareFileInputStream could someday be enhanced
 * to use zero-copy file descriptors on OSs which support them.
 */
class CustomProtobufInputStream : public google::protobuf::io::ZeroCopyInputStream {
public:
    /**
     * Creates a stream that reads from the given Unix file descriptor.
     * If a block_size is given, it specifies the number of bytes that
     * should be read and returned with each call to Next().  Otherwise,
     * a reasonable default is used.
     * @param fd File descriptor.
     * @param errorChecker Error checker.
     * @param blockSize Block size.
     */
    CustomProtobufInputStream(
            io::IoBase& io, const utils::ErrorCodeChecker& errorCodeChecker, int blockSize = -1);

    /**
     * Flushes any buffers and closes the underlying file.  Returns false if
     * an error occurs during the process; use GetErrno() to examine the error.
     * Even if an error occurs, the file descriptor is closed when this returns.
     * @return true on success, false otherwise.
     */
    bool Close();

    /**
     * By default, the file descriptor is not closed when the stream is
     * destroyed.  Call SetCloseOnDelete(true) to change that.  WARNING:
     * This leaves no way for the caller to detect if close() fails.  If
     * detecting close() errors is important to you, you should arrange
     * to close the descriptor yourself.
     * @param value Flag value.
     */
    void SetCloseOnDelete(bool value) noexcept
    {
        m_copyingInput.SetCloseOnDelete(value);
    }

    /**
     * If an I/O error has occurred on this file descriptor, this is the
     * errno from that error.  Otherwise, this is zero.  Once an error
     * occurs, the stream is broken and all subsequent operations will fail.
     * @return Error code.
     */
    int GetErrno() const noexcept
    {
        return m_copyingInput.GetErrno();
    }

    // implements ZeroCopyInputStream
    bool Next(const void** data, int* size);
    void BackUp(int count);
    bool Skip(int count);
    google::protobuf::int64 ByteCount() const;

private:
    class CopyingInputStream : public google::protobuf::io::CopyingInputStream {
    public:
        CopyingInputStream(io::IoBase& io, const utils::ErrorCodeChecker& errorCodeChecker);
        ~CopyingInputStream();

        bool Close();

        void SetCloseOnDelete(bool value) noexcept
        {
            m_closeOnDelete = value;
        }

        int GetErrno() const noexcept
        {
            return m_errno;
        }

        // implements CopyingInputStream
        int Read(void* buffer, int size);
        int Skip(int count);

    private:
        const utils::ErrorCodeChecker& m_errorCodeChecker;
        io::IoBase& m_io;
        bool m_closeOnDelete;
        bool m_closed;

        /** The errno of the I/O error, if one has occurred. Otherwise, zero */
        int m_errno;

        /**
         *  Did we try to seek once and fail?  If so, we assume this file descriptor
         * doesn't support seeking and won't try again.
         */
        bool m_prevSeekFailed;

        GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(CopyingInputStream);
    };

    CopyingInputStream m_copyingInput;
    google::protobuf::io::CopyingInputStreamAdaptor m_impl;

    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(CustomProtobufInputStream);
};

}  // namespace siodb::protobuf
