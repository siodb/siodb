// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConstraintType.h"

// STL headers
#include <array>

namespace siodb::iomgr::dbengine {

namespace {

/** Constraint type pproperties */
struct ConstraintTypeTraits {
    /** Type name */
    const char* m_typeName;

    /** Auto-generated constraint name prefix */
    const char* m_constraintNamePrefix;
};

const std::array<ConstraintTypeTraits, static_cast<std::size_t>(ConstraintType::kMax)>
        g_ConstraintTypeTraits {
                ConstraintTypeTraits {"NOT NULL", "NN"},
                ConstraintTypeTraits {"DEFAULT", "DEF"},
                ConstraintTypeTraits {"UNIQUE", "UQ"},
                ConstraintTypeTraits {"UNIQUE", "MUQ"},
                ConstraintTypeTraits {"REFERENCES", "REF"},
                ConstraintTypeTraits {"COLLATE", "COLL"},
                ConstraintTypeTraits {"CHECK", "CK"},
                ConstraintTypeTraits {"FOREIGN KEY", "FK"},
        };

}  // namespace

const char* getConstraintTypeName(ConstraintType type)
{
    return g_ConstraintTypeTraits.at(static_cast<std::size_t>(type)).m_typeName;
}

const char* getConstaintNamePrefix(ConstraintType type)
{
    return g_ConstraintTypeTraits.at(static_cast<std::size_t>(type)).m_constraintNamePrefix;
}

}  // namespace siodb::iomgr::dbengine
