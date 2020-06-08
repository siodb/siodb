// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "FileData.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Node.h"
#include "UniqueLinearIndex.h"
#include "../Debug.h"
#include "../ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine::uli {

FileData::FileData(UniqueLinearIndex& index, io::FilePtr&& file, std::uint64_t fileId)
    : m_index(index)
    , m_lastNodeTag(0)
    , m_file(std::move(file))
    , m_nodeCount(countNodesInFile())
    , m_nodeCache(*this, fileId, kNodeCacheCapacity)
{
}

std::size_t FileData::countNodesInFile() const
{
    // Validate index file size
    struct stat st;
    if (!m_file->stat(st)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotStatIndexFile, m_index.getDatabaseName(),
                m_index.getTableName(), m_index.getName(), m_file->getLastError(),
                std::strerror(m_file->getLastError()));
    }

    const auto nodeCount = st.st_size / Node::kSize;
    if (st.st_size % Node::kSize > 0 || nodeCount < 2) {
        std::ostringstream str;
        str << "invalid file size " << st.st_size;
        throwDatabaseError(IOManagerMessageId::kErrorIndexFileCorrupted, m_index.getDatabaseName(),
                m_index.getTableName(), m_index.getName(), m_index.getDatabaseUuid(),
                m_index.getTableId(), m_index.getId(), str.str());
    }

    return nodeCount;
}

NodePtr FileData::findNode(std::uint64_t nodeId)
{
    auto cachedNode = m_nodeCache.get(nodeId);
    return cachedNode ? *cachedNode : readNode(nodeId);
}

NodePtr FileData::readNode(std::uint64_t nodeId)
{
    auto node = std::make_shared<Node>(nodeId, ++m_lastNodeTag);
    const auto nodeOffset = getNodeOffset(nodeId);
    if (m_file->read(node->m_data, Node::kSize, nodeOffset) != Node::kSize) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadIndexFile,
                m_index.makeIndexFilePath(getFileId()), m_index.getDatabaseName(),
                m_index.getTableName(), m_index.getName(), m_index.getDatabaseUuid(),
                m_index.getTableId(), m_index.getId(), nodeOffset, Node::kSize,
                m_file->getLastError(), std::strerror(m_file->getLastError()));
    }
    m_nodeCache.emplace(node->m_nodeId, node);
    return node;
}

off_t FileData::getNodeOffset(std::uint64_t nodeId) const noexcept
{
    return (((nodeId - 1) % m_index.getNumberOfNodesPerFile()) + 1) * Node::kSize;
}

}  // namespace siodb::iomgr::dbengine::uli
