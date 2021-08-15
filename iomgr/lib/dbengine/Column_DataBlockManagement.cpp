// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Column.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDataBlock.h"
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/Format.h>

namespace siodb::iomgr::dbengine {

ColumnDataBlockPtr Column::selectAvailableBlock(std::size_t requiredLength)
{
    std::lock_guard lock(m_mutex);
    return selectAvailableBlockUnlocked(requiredLength);
}

ColumnDataBlockPtr Column::createBlock(std::uint64_t prevBlockId, ColumnDataBlockState state)
{
    std::lock_guard lock(m_mutex);
    auto block = std::make_shared<ColumnDataBlock>(*this, prevBlockId, state);
    m_blockCache.emplace(block->getId(), block);
    m_blockRegistry.recordBlockAndNextBlock(block->getId(), prevBlockId);
    return block;
}

std::uint64_t Column::findPrevBlockId(std::uint64_t blockId) const
{
    std::lock_guard lock(m_mutex);
    return m_blockRegistry.findPrevBlockId(blockId);
}

void Column::updateBlockState(std::uint64_t blockId, ColumnDataBlockState state) const
{
    std::lock_guard lock(m_mutex);
    m_blockRegistry.updateBlockState(blockId, state);
}

// --- internals ---

ColumnDataBlockPtr Column::loadBlock(std::uint64_t blockId)
{
    std::lock_guard lock(m_mutex);
    auto block = m_blockCache.get(blockId).value_or(nullptr);
    if (!block) {
        block = std::make_shared<ColumnDataBlock>(*this, blockId);
        m_blockCache.emplace(block->getId(), block);
    }
    return block;
}

ColumnDataBlockPtr Column::selectAvailableBlockUnlocked(std::size_t requiredLength)
{
    // If there are no available blocks, just create new one
    if (m_availableDataBlocks.empty()) {
        auto block = createBlock(0, ColumnDataBlockState::kCurrent);
        m_availableDataBlocks.emplace(block->getId(), block->getFreeDataSpace());
        return block;
    }

    // Try to find some block that has enough room
    std::pair<std::uint64_t, std::uint32_t> minFreeSpaceBlock = *m_availableDataBlocks.begin();
    for (const auto& block : m_availableDataBlocks) {
        if (block.second >= requiredLength) return loadBlock(block.first);
        if (minFreeSpaceBlock.second < block.second) minFreeSpaceBlock = block;
    }

    // Create new block chained to the block with minimum free space
    ColumnDataBlockPtr block = loadBlock(minFreeSpaceBlock.first);
    m_availableDataBlocks.erase(block->getId());
    block = createOrGetNextBlock(*block, requiredLength);
    return block;
}

void Column::updateAvailableBlock(const ColumnDataBlock& block)
{
    const auto freeSpace = block.getFreeDataSpace();
    const auto it = m_availableDataBlocks.find(block.getId());
    if (it == m_availableDataBlocks.end())
        m_availableDataBlocks.emplace(block.getId(), freeSpace);
    else
        it->second = freeSpace;
}

ColumnDataBlockPtr Column::createOrGetNextBlock(
        ColumnDataBlock& block, std::size_t requiredFreeSpace)
{
    // Validate requiredFreeSpace
    if (requiredFreeSpace == 0) throw std::invalid_argument("requiredFreeSpace is zero");
    if (requiredFreeSpace > m_dataBlockDataAreaSize)
        throw std::invalid_argument("requiredFreeSpace is too large");

    ColumnDataBlockPtr nextBlock;

    // Get existing next blocks
    const auto nextBlockIds = m_blockRegistry.findNextBlockIds(block.getId());
    if (!nextBlockIds.empty()) {
        // Iterare existing next blocks in the reverse order because there's higher probability
        // to get block with necessary free space this way.
        for (auto rit = nextBlockIds.rbegin(); rit != nextBlockIds.rend(); ++rit) {
            const auto nextBlockId = *rit;
            auto nextBlockCandidate = loadBlock(nextBlockId);
            if (!nextBlockCandidate) {
                throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist,
                        getDatabaseName(), m_table.getName(), m_name, nextBlockId,
                        getDatabaseUuid(), m_table.getId(), m_id);
            }
            const auto state = nextBlockCandidate->getState();
            if ((state == ColumnDataBlockState::kCurrent
                        || state == ColumnDataBlockState::kAvailable)
                    && nextBlockCandidate->getFreeDataSpace() >= requiredFreeSpace) {
                nextBlock = nextBlockCandidate;
                break;
            }
        }
    }

    if (!nextBlock) {
        // There are either no existing next blocks or no one of them has matched
        // So create new block
        nextBlock = createBlock(block.getId());
    }

    // Obtain previous block header
    ColumnDataBlockHeader::Digest prevBlockDigest;
    const auto prevBlockId = block.getPrevBlockId();
    if (prevBlockId == 0)
        prevBlockDigest = ColumnDataBlockHeader::kInitialPrevBlockDigest;
    else {
        auto prevBlock = m_blockCache.get(prevBlockId).value_or(nullptr);
        if (!prevBlock) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockNotAvailable,
                    getDatabaseName(), m_table.getName(), m_name, prevBlockId, getDatabaseUuid(),
                    m_table.getId(), m_id);
        }
        prevBlockDigest = prevBlock->getDigest();
    }

    block.finalize(prevBlockDigest);
    m_availableDataBlocks.erase(block.getId());
    updateAvailableBlock(*nextBlock);
    return nextBlock;
}

ColumnDataBlockPtr Column::findExistingBlock(std::uint64_t blockId)
{
    auto block = loadBlock(blockId);
    if (!block) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist, getDatabaseName(),
                m_table.getName(), m_name, blockId, getDatabaseUuid(), m_table.getId(), m_id);
    }
    return block;
}

std::uint64_t Column::findFirstBlock() const
{
    constexpr auto kColumnDataBlockFilePrefixLength = ct_strlen(ColumnDataBlock::kBlockFilePrefix);
    constexpr auto kColumnDataBlockFileExtensionLength = ct_strlen(kDataFileExtension);
    constexpr auto kColumnDataBlockFileStaticLength =
            kColumnDataBlockFilePrefixLength + kColumnDataBlockFileExtensionLength;

    // Find first available block
    auto firstBlockId = std::numeric_limits<std::uint64_t>::max();
    for (const auto& entry : fs::directory_iterator(m_dataDir)) {
        if (!fs::is_regular_file(entry)) continue;
        const auto fileName1 = entry.path().filename();
        const auto& fileName = fileName1.string();

        bool fileIgnored = true;
        if (fileName.length() > kColumnDataBlockFileStaticLength
                && std::strncmp(fileName.c_str(), ColumnDataBlock::kBlockFilePrefix,
                           kColumnDataBlockFilePrefixLength)
                           == 0
                && std::strcmp(fileName.c_str() + fileName.length()
                                       - kColumnDataBlockFileExtensionLength,
                           kDataFileExtension)
                           == 0) {
            const auto blockIdStr = fileName.substr(kColumnDataBlockFilePrefixLength,
                    fileName.length() - kColumnDataBlockFileStaticLength);
            std::uint64_t blockId;
            try {
                std::size_t idx = 0;
                blockId = std::stoull(blockIdStr, &idx, 10);
                if (blockIdStr.c_str()[idx] == '\0') {
                    fileIgnored = false;
                    firstBlockId = std::min(firstBlockId, blockId);
                }
            } catch (std::exception&) {
                // do nothing here
            }
        }
        if (fileIgnored && s_wellKnownIgnorableFiles.count(fileName) == 0) {
            LOG_WARNING << utils::format(
                    "Consistency check for column '%1%'.'%2%'.'%3%': file '%4%' ignored",
                    getDatabaseName(), m_table.getName(), m_name, fileName);
        }
    }

    if (firstBlockId == std::numeric_limits<std::uint64_t>::max()) firstBlockId = 0;
    return firstBlockId;
}

}  // namespace siodb::iomgr::dbengine
