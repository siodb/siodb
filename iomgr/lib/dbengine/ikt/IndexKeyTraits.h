// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/BinaryValue.h>

namespace siodb::iomgr::dbengine {

/** Type of numeric key */
enum class NumericKeyType {
    kNonNumeric,
    kSignedInt,
    kUnsignedInt,
    kFloatingPoint,
    kOther,
};

/** Index key traits information provider. */
class IndexKeyTraits {
public:
    /**
     * Returns key size.
     * @return Key size in bytes.
     */
    virtual std::size_t getKeySize() const noexcept = 0;

    /**
     * Writes minimum key value to the given buffer.
     * @param key Key buffer.
     * @return Key buffer.
     */
    virtual void* getMinKey(void* key) const noexcept = 0;

    /**
     * Writes maximum key value to the given buffer.
     * @param key Key buffer.
     * @return Key buffer.
     */
    virtual void* getMaxKey(void* key) const noexcept = 0;

    /**
     * Returns numeric key type.
     * @return Numeric key type.
     */
    virtual NumericKeyType getNumericKeyType() const noexcept = 0;

    /**
     * Retuns buffer filled with minimum key.
     * @return Key buffer.
     */
    BinaryValue getMinKey() const;

    /**
     * Retuns buffer filled with minimum key.
     * @return Key buffer.
     */
    BinaryValue getMaxKey() const;
};

}  // namespace siodb::iomgr::dbengine
