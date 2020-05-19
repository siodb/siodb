// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnBlobStream.h"

namespace siodb::iomgr::dbengine {

ColumnBlobStream::ColumnBlobStream(const ColumnBlobStream& src)
    : LobStream(0)
    , BlobStream(0)
    , ColumnLobStream(src.m_column, src.m_startingAddress, static_cast<bool>(src.m_columnHolder))
{
}

ColumnBlobStream::ColumnBlobStream(Column& column, const ColumnDataAddress& addr, bool holdSource)
    : LobStream(0)
    , BlobStream(0)
    , ColumnLobStream(column, addr, holdSource)
{
}

ColumnBlobStream* ColumnBlobStream::clone() const
{
    return new ColumnBlobStream(*this);
}

std::ptrdiff_t ColumnBlobStream::read(void* buffer, std::size_t bufferSize)
{
    return readInternal(buffer, bufferSize);
}

bool ColumnBlobStream::rewind()
{
    doRewind();
    return true;
}

}  // namespace siodb::iomgr::dbengine
