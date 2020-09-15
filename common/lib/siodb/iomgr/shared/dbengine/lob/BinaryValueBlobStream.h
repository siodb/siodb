// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BlobStream.h"

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

/** Vector-based BLOB stream */
class BinaryValueBlobStream : public BlobStream {
private:
    /**
     * Initializes object of class BinaryValueBlobStream.
     * @param src Source stream.
     */
    explicit BinaryValueBlobStream(const BinaryValueBlobStream& src) noexcept;

public:
    /**
     * Initializes object of class BinaryValueBlobStream.
     * @param v A vector.
     */
    explicit BinaryValueBlobStream(const BinaryValue& v);

    /**
     * Initializes object of class BinaryValueBlobStream.
     * @param v A vector.
     */
    explicit BinaryValueBlobStream(BinaryValue&& v) noexcept;

    /**
     * Creates copy of this stream.
     * @return Copy of this stream object.
     */
    BinaryValueBlobStream* clone() const override;

    /**
     * Reads data from stream up to given data size.
     * @param buffer Destinaton buffer.
     * @param bufferSize Buffer size.
     * @return -1 if error occurred, 0 if EOF reached, otherwise data size read actually.
     */
    std::ptrdiff_t read(void* buffer, std::size_t bufferSize) override;

    /**
     * Rewinds stream to beginning.
     * @return Always returns true.
     */
    bool rewind() override;

private:
    /** Content string */
    const std::shared_ptr<const BinaryValue> m_content;
};

}  // namespace siodb::iomgr::dbengine
