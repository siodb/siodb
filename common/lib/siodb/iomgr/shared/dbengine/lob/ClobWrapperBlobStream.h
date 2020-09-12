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

/** BLOB stream based on the CLOB stream */
class ClobWrapperBlobStream : public BlobStream {
public:
    /**
     * Initializes object of class ClobWrapperBlobStream.
     * @param clobStream An underlying CLOB stream.
     * @param owner Take ownership of underlying CLOB stream.
     */
    ClobWrapperBlobStream(ClobStream* clobStream, bool owner = false) noexcept;

    DECLARE_NONCOPYABLE(ClobWrapperBlobStream);

    /**
     * Throws exception because this stream cannot be cloned.
     * @throw std::logic_error always.
     */
    ClobWrapperBlobStream* clone() const override;

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
    /** Underlying CLOB stream ownerhip facility */
    const std::unique_ptr<ClobStream> m_clobStreamHolder;

    /** An underlying CLOB stream */
    ClobStream* const m_clobStream;

    /** Initial position of an underlying Clob stream */
    const std::uint32_t m_initialPos;
};

}  // namespace siodb::iomgr::dbengine
