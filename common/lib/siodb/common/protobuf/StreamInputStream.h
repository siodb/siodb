// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../io/InputStream.h"
#include "../utils/ErrorCodeChecker.h"

// STL headers
#include <memory>

// Protobuf headers
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

namespace siodb::protobuf {

/**
 * Siodb version of the protobuf output stream which operates over an InputStream and
 * allows to handling of read errors, including EINTR.
 * A ZeroCopyInputStream which reads from a file descriptor.
 * SignalAwareFileInputStream is preferred over using an ifstream with IstreamInputStream.
 * The latter will introduce an extra layer of buffering, harming performance.
 * Also, it's conceivable that SignalAwareFileInputStream could someday be enhanced
 * to use zero-copy file descriptors on OSs which support them.
 */
class StreamInputStream : public google::protobuf::io::ZeroCopyInputStream, public io::InputStream {
public:
    /**
     * Creates a stream that reads from the given InputStream.
     * If a block_size is given, it specifies the number of bytes that
     * should be read and returned with each call to Next(). Otherwise,
     * a reasonable default is used.
     * @param stream I/O stream.
     * @param errorChecker Error checker.
     * @param blockSize Block size.
     */
    StreamInputStream(io::InputStream& stream, const utils::ErrorCodeChecker& errorCodeChecker,
            int blockSize = -1);

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
     * If an I/O error has occurred on this InputStream, this is the
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

    /**
     * Checks that there is no error.
     * @throw std::system_error if error happened.
     */
    void CheckNoError() const;

    ////////////// siodb::io::InputStream implementation //////////////////////

    /**
     * Returns indication that stream is valid.
     * @return true stream if stream is valid, false otherwise.
     */
    bool isValid() const noexcept override;

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
     * @return Number of bytes skipped. Negative value indicates error.
     */
    std::ptrdiff_t skip(std::size_t size) override;

private:
    class CopyingInputStream : public google::protobuf::io::CopyingInputStream {
    public:
        CopyingInputStream(
                io::InputStream& stream, const utils::ErrorCodeChecker& errorCodeChecker);
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

        bool IsClosed() const noexcept
        {
            return m_closed;
        }

        // implements CopyingInputStream
        int Read(void* buffer, int size);
        int Skip(int count);

    private:
        /** Error code checker */
        const utils::ErrorCodeChecker& m_errorCodeChecker;
        /** Input stream */
        io::InputStream& m_stream;
        /** Indicaiton that stream should be closed in the destructor */
        bool m_closeOnDelete;
        /** Indication of the closed stream */
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

    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(StreamInputStream);
};

}  // namespace siodb::protobuf
