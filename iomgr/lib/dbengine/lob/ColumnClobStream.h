// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnLobStream.h"

namespace siodb::iomgr::dbengine {

/** Column-based CLOB stream */
class ColumnClobStream : public ClobStream, public ColumnLobStream {
private:
    /**
     * Initializes object of class ColumnClobStream.
     * @param src Source stream.
     */
    ColumnClobStream(const ColumnClobStream& src);

public:
    /**
     * Initializes object of class ColumnClobStream.
     * @param column Column object.
     * @param addr CLOB address.
     * @param holdSource Flag indicates that data source must be hold by this object.
     */
    ColumnClobStream(Column& column, const ColumnDataAddress& addr, bool holdSource);

    /**
     * Creates copy of this stream.
     * @return copy of this stream ot nullptr of cloning of stream is not possible.
     */
    ColumnClobStream* clone() const override;

    /**
     * Reads data from stream up to given data size.
     * @param buffer Destinaton buffer.
     * @param bufferSize Buffer size.
     * @return -1 if error occurred, 0 if EOF reached, otherwise data size read actually.
     */
    std::ptrdiff_t read(void* buffer, std::size_t bufferSize) override;

    /**
     * Rewinds stream to beginning.
     * @return true if stream was rewind, false if stream doesn't support rewinding.
     */
    bool rewind() override;
};

}  // namespace siodb::iomgr::dbengine
