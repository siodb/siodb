// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ClobWrapperBlobStream.h"

namespace siodb::iomgr::dbengine {

ClobWrapperBlobStream::ClobWrapperBlobStream(ClobStream* clobStream, bool owner)
    : LobStream(clobStream->getRemainingSize())
    , BlobStream(clobStream->getRemainingSize())
    , m_clobStreamHolder(owner ? clobStream : nullptr)
    , m_clobStream(clobStream)
    , m_initialPos(m_clobStream->getPos())
{
}

ClobWrapperBlobStream* ClobWrapperBlobStream::clone() const
{
    return nullptr;
}

std::ptrdiff_t ClobWrapperBlobStream::read(void* buffer, std::size_t bufferSize)
{
    auto res = m_clobStream->read(buffer, bufferSize);
    m_pos = m_clobStream->getPos() - m_initialPos;
    return res;
}

bool ClobWrapperBlobStream::rewind()
{
    return m_clobStream->rewind();
}

}  // namespace siodb::iomgr::dbengine
