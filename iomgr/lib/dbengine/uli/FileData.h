// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "FileDataPtr.h"
#include "NodeCache.h"

// Common project headers
#include <siodb/iomgr/shared/dbengine/io/File.h>

// STL headers
#include <atomic>
#include <unordered_set>

namespace siodb::iomgr::dbengine {

class UniqueLinearIndex;

}  // namespace siodb::iomgr::dbengine

namespace siodb::iomgr::dbengine::uli {

/** Linear index file related data */
struct FileData {
    /** Node cache capacity */
    static constexpr std::size_t kNodeCacheCapacity = 16;

    /**
     * Initilizes object of class FileData.
     * @param index Owner index object.
     * @param file File object.
     * @param fileId File ID.
     */
    FileData(UniqueLinearIndex& index, io::FilePtr&& file, std::uint64_t fileId);

    /**
     * Counts actual number of nodes in the index file.
     * @return Actual number of nodes in the index file.
     */
    std::size_t countNodesInFile() const;

    /**
     * Returns existing physical node.
     * @param nodeId Node ID.
     * @return Node object.
     */
    NodePtr findNode(std::uint64_t nodeId);

    /**
     * Reads in existing physical node.
     * @param nodeId Node ID.
     * @return Node object.
     */
    NodePtr readNode(std::uint64_t nodeId);

    /**
     * Calculates node offset.
     * @param nodeId Node ID.
     * @return Node offset in the file.
     */
    off_t getNodeOffset(std::uint64_t nodeId) const noexcept;

    /**
     * Returns file ID.
     * @return File ID.
     */
    auto getFileId() const noexcept
    {
        return m_nodeCache.getIndexFileId();
    }

    /** Owner index object */
    UniqueLinearIndex& m_index;

    /** Node tag counter */
    std::atomic<std::uint64_t> m_lastNodeTag;

    /** Index file descriptor. */
    io::FilePtr m_file;

    /** Number of nodes in the file */
    const std::uint64_t m_nodeCount;

    /** Node cache */
    NodeCache m_nodeCache;

    /** Maximum available node ID */
    std::uint64_t m_maxNodeId;

    /** Modified nodes (used during modification operations) */
    std::unordered_set<Node*> m_modifiedNodes;
};

}  // namespace siodb::iomgr::dbengine::uli
