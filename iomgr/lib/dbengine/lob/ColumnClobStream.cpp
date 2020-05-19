// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnClobStream.h"

namespace siodb::iomgr::dbengine {

ColumnClobStream::ColumnClobStream(const ColumnClobStream& src)
    : LobStream(0)
    , ClobStream(0)
    , ColumnLobStream(src.m_column, src.m_startingAddress, static_cast<bool>(src.m_columnHolder))
{
}

ColumnClobStream::ColumnClobStream(Column& column, const ColumnDataAddress& addr, bool holdSource)
    : LobStream(0)
    , ClobStream(0)
    , ColumnLobStream(column, addr, holdSource)
{
}

ColumnClobStream* ColumnClobStream::clone() const
{
    return new ColumnClobStream(*this);
}

std::ptrdiff_t ColumnClobStream::read(void* buffer, std::size_t bufferSize)
{
    return readInternal(buffer, bufferSize);
}

bool ColumnClobStream::rewind()
{
    doRewind();
    return true;
}

}  // namespace siodb::iomgr::dbengine
