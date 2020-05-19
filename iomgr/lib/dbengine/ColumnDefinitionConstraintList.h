// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDefinitionConstraint.h"
#include "ColumnDefinitionConstraintListPtr.h"

namespace siodb::iomgr::dbengine {

/** In-memory column definition constraint list with multiple indexes */
class ColumnDefinitionConstraintList {
public:
    using value_type = ColumnDefinitionConstraintPtr;

private:
    /** Index tag */
    struct ByIdTag {
    };

    /** Index tag */
    struct ByConstraintIdTag {
    };

    /** Index tag */
    struct ByConstraintNameTag {
    };

    /** Index tag */
    struct ByConstraintTypeTag {
    };

    /** Key extractor for the column definition constaint ID */
    struct ExtractId {
        typedef std::uint64_t result_type;
        result_type operator()(const value_type& r) const noexcept
        {
            return r->getId();
        }
    };

    /** Key extractor for the constaint ID */
    struct ExtractConstraintId {
        typedef std::uint64_t result_type;
        result_type operator()(const value_type& r) const noexcept
        {
            return r->getConstraint().getId();
        }
    };

    /** Key extractor for the constaint name */
    struct ExtractConstraintName {
        typedef std::string result_type;
        const result_type& operator()(const value_type& r) const noexcept
        {
            return r->getConstraint().getName();
        }
    };

    /** Key extractor for the constaint type */
    struct ExtractConstraintType {
        typedef ConstraintType result_type;
        result_type operator()(const value_type& r) const noexcept
        {
            return r->getConstraint().getType();
        }
    };

    /** Data container type */
    typedef boost::multi_index::multi_index_container<value_type,
            boost::multi_index::indexed_by<
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByIdTag>, ExtractId>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByConstraintIdTag>,
                            ExtractConstraintId>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByConstraintNameTag>,
                            ExtractConstraintName>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByConstraintTypeTag>,
                            ExtractConstraintType>>>
            Container;

public:
    /**
     * Returns read-only index by column definition constraint ID.
     * @return Index object.
     */
    const auto& byId() const noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns read-only index by constraint ID.
     * @return Index object.
     */
    const auto& byConstraintId() const noexcept
    {
        return m_container.get<ByConstraintIdTag>();
    }

    /**
     * Returns read-only index by constraint type.
     * @return Index object.
     */
    const auto& byConstraintType() const noexcept
    {
        return m_container.get<ByConstraintTypeTag>();
    }

    /**
     * Returns read-only index by constraint name.
     * @return Index object.
     */
    const auto& byConstraintName() const noexcept
    {
        return m_container.get<ByConstraintNameTag>();
    }

    /**
     * Returns indication that constainer is empty.
     * @return true if constainer is empty, false otherwise.
     */
    bool empty() const noexcept
    {
        return m_container.empty();
    }

    /**
     * Returns number of records in the container.
     * @return Number of records in the container.
     */
    std::size_t size() const noexcept
    {
        return m_container.size();
    }

    /**
     * Inserts new object into the constainer.
     * @param args Column definition constraint record construction arguments.
     */
    template<typename... Args>
    void emplace(Args&&... args)
    {
        m_container.insert(
                std::make_shared<ColumnDefinitionConstraint>(std::forward<Args>(args)...));
    }

    /**
     * Inserts a record into the constainer.
     * @param columnDefinitionConstraint Column definition constraint.
     */
    void insert(const value_type& columnDefinitionConstraint)
    {
        m_container.insert(columnDefinitionConstraint);
    }

    /** Clears constainer */
    void clear() noexcept
    {
        m_container.clear();
    }

    /**
     * Swaps content with another container.
     * @param other Other container.
     */
    void swap(ColumnDefinitionConstraintList& other) noexcept
    {
        if (&other != this) m_container.swap(other.m_container);
    }

private:
    /** Data container */
    Container m_container;
};

}  // namespace siodb::iomgr::dbengine
