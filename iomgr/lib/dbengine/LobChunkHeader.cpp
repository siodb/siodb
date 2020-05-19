// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "LobChunkHeader.h"

// Common project headers
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

std::uint8_t* LobChunkHeader::serialize(std::uint8_t* buffer) const noexcept
{
    buffer = ::pbeEncodeUInt32(m_remainingLobLength, buffer);
    buffer = ::pbeEncodeUInt32(m_chunkLength, buffer);
    buffer = ::pbeEncodeUInt64(m_nextChunkBlockId, buffer);
    buffer = ::pbeEncodeUInt32(m_nextChunkOffset, buffer);
    return buffer;
}

const std::uint8_t* LobChunkHeader::deserialize(const std::uint8_t* buffer) noexcept
{
    buffer = ::pbeDecodeUInt32(buffer, &m_remainingLobLength);
    buffer = ::pbeDecodeUInt32(buffer, &m_chunkLength);
    buffer = ::pbeDecodeUInt64(buffer, &m_nextChunkBlockId);
    buffer = ::pbeDecodeUInt32(buffer, &m_nextChunkOffset);
    return buffer;
}

}  // namespace siodb::iomgr::dbengine
