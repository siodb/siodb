// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IndexKeyTraits.h"

namespace siodb::iomgr::dbengine {

/** Index key traits information provider for the int16_t based index. */
class UInt16IndexKeyTraits final : public IndexKeyTraits {
public:
    /**
     * Returns key size.
     * @return Key size in bytes.
     */
    std::size_t getKeySize() const noexcept override;

    /**
     * Writes minimum key value to the given buffer.
     * @param key Key buffer.
     * @return Key buffer.
     */
    void* getMinKey(void* key) const noexcept override;

    /**
     * Writes maximum key value to the given buffer.
     * @param key Key buffer.
     * @return Key buffer.
     */
    void* getMaxKey(void* key) const noexcept override;

    /**
     * Returns numeric key type.
     * @return Numeric key type.
     */
    NumericKeyType getNumericKeyType() const noexcept override;

    /**
     * 3-way key compare function.
     * @param left Left operand.
     * @param right Right operand.
     * @return 0, if operands are equal, negative value if left < right,
     *         positive value if left > right.
     */
    static int compareKeys(const void* left, const void* right) noexcept;
};

}  // namespace siodb::iomgr::dbengine
