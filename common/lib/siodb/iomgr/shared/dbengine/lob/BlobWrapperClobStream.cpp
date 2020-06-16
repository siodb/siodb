// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BlobWrapperClobStream.h"

// CRT headers
#include <cstring>

namespace siodb::iomgr::dbengine {

const char BlobWrapperClobStream::s_hexTable[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

BlobWrapperClobStream::BlobWrapperClobStream(BlobStream* blobStream, bool owner) noexcept
    : LobStream(blobStream->getRemainingSize() * 2)
    , ClobStream(blobStream->getRemainingSize() * 2)
    , m_blobStreamHolder(owner ? blobStream : nullptr)
    , m_blobStream(blobStream)
    , m_initialPos(m_blobStream->getPos())
    , m_pendingChar(0)
{
}

BlobWrapperClobStream* BlobWrapperClobStream::clone() const
{
    return nullptr;
}

std::ptrdiff_t BlobWrapperClobStream::read(void* buffer, std::size_t bufferSize)
{
    if (bufferSize == 0) return 0;

    char* dest = static_cast<char*>(buffer);
    char* const end = dest + bufferSize;

    // Handle pending character if any
    if (m_pendingChar) {
        *dest = m_pendingChar;
        m_pendingChar = '\0';
        if (bufferSize == 1) {
            updatePos();
            return 1;
        }
        ++dest;
    }

    // Read data into larger buffer
    const auto requiredInputSize = (bufferSize / 2) + 1;
    auto src = static_cast<char*>(buffer) + (bufferSize - requiredInputSize);
    const auto actualInputSize = m_blobStream->read(src, requiredInputSize);
    if (actualInputSize < 1) {
        const auto outputSize = end - dest;
        updatePos();
        return outputSize > 0 ? outputSize : actualInputSize;
    }

    // Adjust data position if less bytes were read
    if (static_cast<size_t>(actualInputSize) < requiredInputSize) {
        const auto newSrc = src + (requiredInputSize - actualInputSize);
        memmove(newSrc, src, actualInputSize);
        src = newSrc;
    }

    // Convert to text as hex
    while (src < end) {
        const char srcByte = *src++;
        const char hiChar = s_hexTable[(srcByte >> 4) & 0xF];
        const char loChar = s_hexTable[srcByte & 0xF];
        *dest++ = hiChar;
        if (dest == end) {
            m_pendingChar = loChar;
            break;
        }
        *dest++ = loChar;
    }

    updatePos();
    return dest - static_cast<char*>(buffer);
}

bool BlobWrapperClobStream::rewind()
{
    return m_blobStream->rewind();
}

}  // namespace siodb::iomgr::dbengine
