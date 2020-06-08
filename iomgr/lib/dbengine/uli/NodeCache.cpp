// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "NodeCache.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "FileData.h"
#include "Node.h"
#include "UniqueLinearIndex.h"
#include "../Debug.h"
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/log/Log.h>

namespace siodb::iomgr::dbengine::uli {

NodeCache::~NodeCache()
{
    try {
        flush();
    } catch (...) {
        // Ignore errors
    }
}

void NodeCache::flush()
{
    bool hadErrors = false;
    for (const auto& e : map_internal()) {
        try {
            try {
                if (e.second.first->m_modified) {
                    saveNode(e);
                }
            } catch (DatabaseError& ex) {
                hadErrors = true;
                LOG_ERROR << m_owner.m_index.makeDisplayName() << ": Failed to save ULI node #"
                          << e.second.first->m_nodeId << ": " << ex.getErrorCode() << ' '
                          << ex.what();
            } catch (std::exception& ex) {
                hadErrors = true;
                LOG_ERROR << m_owner.m_index.makeDisplayName() << ": Failed to save ULI node #"
                          << e.second.first->m_nodeId << ": " << ex.what();
            } catch (...) {
                hadErrors = true;
                LOG_ERROR << m_owner.m_index.makeDisplayName() << ": Failed to save ULI node #"
                          << e.second.first->m_nodeId << ": unknown error";
            }
        } catch (...) {
            hadErrors = true;
            // ignore all exceptions reached this point
        }
    }
    if (hadErrors) {
        throw std::runtime_error("Error flushing ULI node cache");
    }
}

bool NodeCache::can_evict([[maybe_unused]] const key_type& key, const mapped_type& value) const
        noexcept
{
    return !value->m_modified;
}

void NodeCache::on_evict([[maybe_unused]] const key_type& key, mapped_type& value,
        [[maybe_unused]] bool clearingCache) const
{
    ULI_DBG_LOG_DEBUG("NodeCache " << m_owner.m_index.makeDisplayName() << ": Evicting node #"
                                   << key << " tag " << value->m_tag);
    if (value->m_modified) {
        throw std::runtime_error(
                "UniqueLinearIndex: attempt to evict modified node from the cache");
    }
}

bool NodeCache::on_last_chance_cleanup()
{
    ULI_DBG_LOG_DEBUG("NodeCache " << m_owner.m_index.makeDisplayName() << ": Last chance cleanup");
    std::size_t savedCount = 0;
    for (const auto& e : map_internal()) {
        if (!e.second.first->m_modified) continue;
        saveNode(e);
        ++savedCount;
    }
    return savedCount > 0;
}

void NodeCache::saveNode(const map_type::value_type& mapElement) const
{
    auto& node = *mapElement.second.first;
    // Log this always
    LOG_DEBUG << "NodeCache " << m_owner.m_index.makeDisplayName() << ": Saving node #"
              << node.m_nodeId << " tag " << node.m_tag << ": node "
              << (node.m_modified ? " modified" : " not modified");
    if (!node.m_modified) return;
    const auto nodeOffset = m_owner.getNodeOffset(node.m_nodeId);
    if (m_owner.m_file->write(node.m_data, Node::kSize, nodeOffset) != Node::kSize) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile,
                m_owner.m_index.makeIndexFilePath(m_indexFileId), m_owner.m_index.getDatabaseName(),
                m_owner.m_index.getTableName(), m_owner.m_index.getName(),
                m_owner.m_index.getDatabaseUuid(), m_owner.m_index.getTableId(),
                m_owner.m_index.getId(), nodeOffset, Node::kSize, m_owner.m_file->getLastError(),
                std::strerror(m_owner.m_file->getLastError()));
    }
    node.m_modified = false;
}

}  // namespace siodb::iomgr::dbengine::uli
