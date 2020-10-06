// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "FileDataPtr.h"
#include "UniqueLinearIndex.h"

// Common project headers
#include <siodb/common/stl_ext/buffer.h>
#include <siodb/iomgr/shared/dbengine/io/File.h>

namespace siodb::iomgr::dbengine {

class UniqueLinearIndex;

}  // namespace siodb::iomgr::dbengine

namespace siodb::iomgr::dbengine::uli {

/** Linear index file related data */
class FileData {
public:
    /**
     * Initializes object of class FileData.
     * @param index Owner index object.
     * @param fileId File ID.
     * @param file File object.
     */
    FileData(UniqueLinearIndex& index, std::uint64_t fileId, io::FilePtr&& file);

    /**
     * Returns mutable buffer address.
     * @return Buffer address.
     */
    std::uint8_t* getBuffer() noexcept
    {
        return m_data.data();
    }

    /**
     * Returns mutable buffer address.
     * @return Buffer address.
     */
    const std::uint8_t* getBuffer() const noexcept
    {
        return m_data.data();
    }

    /**
     * Calculates record offset in the memory.
     * Assumes record belongs to this file.
     * @param recordId Record identifier.
     * @return Record offset in the file.
     */
    std::size_t getRecordOffsetInMemory(std::uint64_t recordId) const noexcept
    {
        return (recordId % m_index.getNumberOfRecordsPerFile()) * m_index.getRecordSize();
    }

    /**
     * Calculates record offset in the data file.
     * Assumes record belongs to this file.
     * @param recordId Record ID.
     * @return Record offset in the file.
     */
    off_t getRecordOffsetInFile(std::uint64_t recordId) const noexcept
    {
        return getRecordOffsetInMemory(recordId) + UniqueLinearIndex::kIndexFileHeaderSize;
    }

    /**
     * Updates data in the memory and file.
     * @param pos Position in the memory buffer.
     * @param src Source buffer address.
     * @param size Data size.
     * @throw std::out_of_range if combination of the destOffset and size is invalid.
     * @throw DatabaseError if write to file fails.
     */
    void update(std::size_t pos, const void* src, std::size_t size);

private:
    /** Owner index object */
    UniqueLinearIndex& m_index;

    /** File identifier */
    const std::uint64_t m_fileId;

    /** Index file. */
    io::FilePtr m_file;

    /** Index file data buffer */
    stdext::buffer<std::uint8_t> m_data;
};

}  // namespace siodb::iomgr::dbengine::uli
