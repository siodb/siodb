// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BinaryValueBlobStream.h"

// CRT headers
#include <cstring>

namespace siodb::iomgr::dbengine {

BinaryValueBlobStream::BinaryValueBlobStream(const BinaryValueBlobStream& src)
    : LobStream(src.m_content->size())
    , BlobStream(src.m_content->size())
    , m_content(src.m_content)
{
}

BinaryValueBlobStream::BinaryValueBlobStream(const BinaryValue& v)
    : LobStream(v.size())
    , BlobStream(v.size())
    , m_content(std::make_shared<BinaryValue>(v))
{
}

BinaryValueBlobStream::BinaryValueBlobStream(BinaryValue&& v) noexcept
    : LobStream(v.size())
    , BlobStream(v.size())
    , m_content(std::make_shared<BinaryValue>(std::move(v)))
{
}

BinaryValueBlobStream* BinaryValueBlobStream::clone() const
{
    return new BinaryValueBlobStream(*this);
}

std::ptrdiff_t BinaryValueBlobStream::read(void* buffer, std::size_t bufferSize)
{
    const auto remaining = m_content->size() - m_pos;
    const auto outputSize = std::min(remaining, bufferSize);
    if (outputSize > 0) {
        std::memcpy(buffer, m_content->data() + m_pos, outputSize);
        m_pos += outputSize;
    }
    return outputSize;
}

bool BinaryValueBlobStream::rewind()
{
    trivialRewind();
    return true;
}

}  // namespace siodb::iomgr::dbengine
