// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "FileData.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/crt_ext/compiler_defs.h>

namespace siodb::iomgr::dbengine::uli {

FileData::FileData(UniqueLinearIndex& index, std::uint64_t fileId, io::FilePtr&& file)
    : m_index(index)
    , m_fileId(fileId)
    , m_file(std::move(file))
    , m_data(m_index.getDataFileSize() - UniqueLinearIndex::kDataFileHeaderSize)
{
    if (m_file->read(m_data.data(), m_data.size(), UniqueLinearIndex::kDataFileHeaderSize)
            != m_data.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadIndexFile,
                m_index.makeIndexFilePath(m_fileId), m_index.getDatabaseName(),
                m_index.getTableName(), m_index.getName(), m_index.getDatabaseUuid(),
                m_index.getTableId(), m_index.getId(), UniqueLinearIndex::kDataFileHeaderSize,
                m_data.size(), m_file->getLastError(), std::strerror(m_file->getLastError()));
    }
}

void FileData::update(std::size_t pos, const void* src, std::size_t size)
{
    const auto addr = &m_data.at(pos);
    if (SIODB_LIKELY(size > 0)) {
        const auto dataSize = m_data.size();
        if (size > dataSize || dataSize - size < pos) {
            std::ostringstream err;
            err << "ULI: File #" << m_fileId << ": Invalid update: dataSize=" << dataSize
                << ", position=" << pos << ", size=" << size;
            throw std::out_of_range(err.str());
        }
        std::memcpy(addr, src, size);
        const auto offsetInFile = pos + UniqueLinearIndex::kDataFileHeaderSize;
        if (m_file->write(addr, size, offsetInFile) != size) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile,
                    m_index.makeIndexFilePath(m_fileId), m_index.getDatabaseName(),
                    m_index.getTableName(), m_index.getName(), m_index.getDatabaseUuid(),
                    m_index.getTableId(), m_index.getId(), offsetInFile, size,
                    m_file->getLastError(), std::strerror(m_file->getLastError()));
        }
    }
}

}  // namespace siodb::iomgr::dbengine::uli
