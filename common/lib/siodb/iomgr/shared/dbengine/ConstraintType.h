// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** Constraint types */
enum class ConstraintType {
    /** NULL or NOT NULL column level constraint */
    kNotNull,

    /** DEFAULT with constant value column level constraint */
    kDefaultValue,

    /** UNIQUE for single column column level constraint */
    kSingleColumnUnique,

    /** UNIQUE for multiple columns table level constraint */
    kMultiColumnUnique,

    /** REFERENCES column level constraint */
    kReferences,

    /** COLLATE column level constraint */
    kCollate,

    /** CHECK table level constaint */
    kCheck,

    /** FOREIGN KEY table level constraint */
    kForeignKey,

    /** Supplementary value, that defines max constraint type ID. */
    kMax,
};

/**
 * Returns constraint type name.
 * @param type Constraint type.
 * @return Constraint type name.
 * @throw std::out_of_range if type is invalid or kMax.
 */
const char* getConstraintTypeName(ConstraintType type);

/**
 * Returns aut-generated constraint name prefix.
 * @param type Constraint type.
 * @return Constraint name prefix.
 * @throw std::out_of_range if type is invalid or kMax.
 */
const char* getConstaintNamePrefix(ConstraintType type);

}  // namespace siodb::iomgr::dbengine
