// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDataBlockHeader.h"

// Common project headers
#include <siodb/common/stl_ext/utility_ext.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

std::uint8_t* ColumnDataBlockHeader::serialize(std::uint8_t* buffer) const noexcept
{
    buffer = ::pbeEncodeUInt32(m_version, buffer);
    buffer = ::pbeEncodeBinary(m_fullColumnDataBlockId.m_databaseUuid.data,
            m_fullColumnDataBlockId.m_databaseUuid.size(), buffer);
    buffer = ::pbeEncodeUInt32(m_fullColumnDataBlockId.m_tableId, buffer);
    buffer = ::pbeEncodeUInt64(m_fullColumnDataBlockId.m_columnId, buffer);
    buffer = ::pbeEncodeUInt64(m_fullColumnDataBlockId.m_blockId, buffer);
    buffer = ::pbeEncodeUInt64(m_prevBlockId, buffer);
    buffer = ::pbeEncodeUInt32(m_dataAreaOffset, buffer);
    buffer = ::pbeEncodeUInt32(m_dataAreaSize, buffer);
    buffer = ::pbeEncodeUInt32(m_nextDataOffset, buffer);
    buffer = ::pbeEncodeUInt32(m_commitedDataOffset, buffer);
    buffer = ::pbeEncodeUInt64(m_fillTimestamp, buffer);
    buffer = ::pbeEncodeBinary(m_digest.data(), m_digest.size(), buffer);
    return buffer;
}

const std::uint8_t* ColumnDataBlockHeader::deserialize(const std::uint8_t* buffer) noexcept
{
    buffer = ::pbeDecodeUInt32(buffer, &m_version);
    if (m_version > kCurrentVersion) return nullptr;
    auto fullColumnDataBlockId = &m_fullColumnDataBlockId;
    buffer = ::pbeDecodeBinary(buffer,
            stdext::as_mutable_ptr(reinterpret_cast<const std::uint8_t*>(
                    &m_fullColumnDataBlockId.m_databaseUuid.data)),
            m_fullColumnDataBlockId.m_databaseUuid.size());
    buffer = ::pbeDecodeUInt32(buffer, &fullColumnDataBlockId->m_tableId);
    buffer = ::pbeDecodeUInt64(buffer, &fullColumnDataBlockId->m_columnId);
    buffer = ::pbeDecodeUInt64(buffer, &fullColumnDataBlockId->m_blockId);
    buffer = ::pbeDecodeUInt64(buffer, &m_prevBlockId);
    buffer = ::pbeDecodeUInt32(buffer, &m_dataAreaOffset);
    buffer = ::pbeDecodeUInt32(buffer, &m_dataAreaSize);
    buffer = ::pbeDecodeUInt32(buffer, &m_nextDataOffset);
    buffer = ::pbeDecodeUInt32(buffer, &m_commitedDataOffset);
    buffer = ::pbeDecodeUInt64(buffer, &m_fillTimestamp);
    buffer = ::pbeDecodeBinary(buffer, m_digest.data(), m_digest.size());
    return buffer;
}

}  // namespace siodb::iomgr::dbengine
