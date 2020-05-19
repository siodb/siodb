// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "NodePtr.h"

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine::uli {

/** Linear index node */
struct Node {
    /** Node size */
    static constexpr std::size_t kSize = 8192;

    /**
     * Initializes object of class Node.
     * @param nodeId Node ID.
     */
    Node(std::uint64_t nodeId, std::uint64_t tag) noexcept
        : m_nodeId(nodeId)
        , m_tag(tag)
        , m_modified(false)
    {
    }

    /** Node ID */
    std::uint64_t m_nodeId;

    /** Node tag */
    const std::uint64_t m_tag;

    /** Node data */
    std::uint8_t m_data[kSize];

    /** Modification flag */
    bool m_modified;
};

}  // namespace siodb::iomgr::dbengine::uli
