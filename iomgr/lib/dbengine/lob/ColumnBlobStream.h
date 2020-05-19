// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnLobStream.h"

namespace siodb::iomgr::dbengine {

/** Column-based BLOB stream */
class ColumnBlobStream : public BlobStream, public ColumnLobStream {
    /**
     * Initializes object of class ColumnClobStream.
     * @param src Source stream.
     */
    ColumnBlobStream(const ColumnBlobStream& src);

public:
    /**
     * Initializes object of class ColumnClobStream.
     * @param column Column object.
     * @param addr BLOB address.
     * @param holdSource Flag indicates that data source must be hold by this object.
     */
    ColumnBlobStream(Column& column, const ColumnDataAddress& addr, bool holdSource);

    /**
     * Creates copy of this stream.
     * @return copy of this stream ot nullptr of cloning of stream is not possible.
     */
    ColumnBlobStream* clone() const override;

    /**
     * Reads data from stream up to given data size.
     * @param buffer Destinaton buffer.
     * @param bufferSize Buffer size.
     * @return -1 if error occurred, 0 if EOF reached, otherwise data size read actually.
     */
    std::ptrdiff_t read(void* buffer, std::size_t bufferSize) override;

    /**
     * Rewinds stream.
     * @return true is underlying stream is rewinded, false otherwise.
     */
    bool rewind() override;
};

}  // namespace siodb::iomgr::dbengine
