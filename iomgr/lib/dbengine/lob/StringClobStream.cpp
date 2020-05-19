// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "StringClobStream.h"

// CRT headers
#include <cstring>

namespace siodb::iomgr::dbengine {

StringClobStream::StringClobStream(const StringClobStream& src)
    : LobStream(src.m_content->size())
    , ClobStream(src.m_content->size())
    , m_content(src.m_content)
{
}

StringClobStream::StringClobStream(const std::string& s)
    : LobStream(s.length())
    , ClobStream(s.length())
    , m_content(std::make_shared<std::string>(s))
{
}

StringClobStream::StringClobStream(std::string&& s) noexcept
    : LobStream(s.length())
    , ClobStream(s.length())
    , m_content(std::make_shared<std::string>(std::move(s)))
{
}

StringClobStream* StringClobStream::clone() const
{
    return new StringClobStream(*this);
}

std::ptrdiff_t StringClobStream::read(void* buffer, std::size_t bufferSize)
{
    const auto remaining = m_content->length() - m_pos;
    const auto outputSize = std::min(remaining, bufferSize);
    if (outputSize > 0) {
        std::memcpy(buffer, m_content->c_str() + m_pos, outputSize);
        m_pos += outputSize;
    }
    return outputSize;
}

bool StringClobStream::rewind()
{
    trivialRewind();
    return true;
}

}  // namespace siodb::iomgr::dbengine
