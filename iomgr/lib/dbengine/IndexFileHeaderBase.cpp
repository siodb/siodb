// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IndexFileHeaderBase.h"

// Common project headers
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

std::uint8_t* IndexFileHeaderBase::serialize(std::uint8_t* buffer) const noexcept
{
    buffer = ::pbeEncodeUInt32(m_version, buffer);
    *buffer++ = static_cast<uint8_t>(m_indexType);
    buffer = ::pbeEncodeBinary(
            m_fullIndexId.m_databaseUuid.data, m_fullIndexId.m_databaseUuid.size(), buffer);
    buffer = ::pbeEncodeUInt32(m_fullIndexId.m_tableId, buffer);
    buffer = ::pbeEncodeUInt64(m_fullIndexId.m_indexId, buffer);
    return buffer;
}

const std::uint8_t* IndexFileHeaderBase::deserialize(const std::uint8_t* buffer) noexcept
{
    buffer = ::pbeDecodeUInt32(buffer, &m_version);
    if (m_version > kCurrentVersion) return nullptr;
    const auto indexType = static_cast<IndexType>(*buffer++);
    if (indexType != m_indexType) return nullptr;
    buffer = ::pbeDecodeBinary(
            buffer, &m_fullIndexId.m_databaseUuid.data, m_fullIndexId.m_databaseUuid.size());
    buffer = ::pbeDecodeUInt32(buffer, &m_fullIndexId.m_tableId);
    buffer = ::pbeDecodeUInt64(buffer, &m_fullIndexId.m_indexId);
    return buffer;
}

}  // namespace siodb::iomgr::dbengine
