// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BlobStream.h"
#include "ClobStream.h"

// Common project headers
#include <siodb/common/utils/HelperMacros.h>

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

/** CLOB stream based on the BLOB stream */
class BlobWrapperClobStream : public ClobStream {
public:
    /**
     * Initializes object of class StringClobStream.
     * @param blobStream An underlying BLOB stream.
     * @param owner Take ownership of underlying CLOB stream.
     */
    BlobWrapperClobStream(BlobStream* blobStream, bool owner = false) noexcept;

    DECLARE_NONCOPYABLE(BlobWrapperClobStream);

    /**
     * Cloning of this stream is not supported.
     * @return Always nullptr.
     */
    BlobWrapperClobStream* clone() const override;

    /**
     * Reads data from stream up to given data size.
     * @param buffer Destinaton buffer.
     * @param bufferSize Buffer size.
     * @return -1 if error occurred, 0 if EOF reached, otherwise data size read actually.
     */
    std::ptrdiff_t read(void* buffer, std::size_t bufferSize) override;

    /**
     * Rewinds stream to beginning.
     * @return true if stream was rewind, false if underlying stream doesn't support rewinding.
     */
    bool rewind() override;

private:
    /** Updates current stream position */
    /**
     * Returns current stream position.
     * @return Stream position.
     */
    void updatePos() noexcept
    {
        m_pos = (m_blobStream->getPos() - m_initialPos) * 2 + (m_pendingChar ? 1 : 0);
    }

private:
    /** Underlying CLOB stream ownerhip facility */
    const std::unique_ptr<BlobStream> m_blobStreamHolder;

    /** An underlying BLOB stream */
    BlobStream* const m_blobStream;

    /** Initial position of an underlying Clob stream */
    const std::uint32_t m_initialPos;

    /** Pending character (if nonzero) */
    char m_pendingChar;

    /** Hex table character */
    static const char s_hexTable[16];
};

}  // namespace siodb::iomgr::dbengine
