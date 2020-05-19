// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IndexKeyTraits.h"

// Project headers

namespace siodb::iomgr::dbengine {

BinaryValue IndexKeyTraits::getMinKey() const
{
    BinaryValue minKey(getKeySize());
    getMinKey(minKey.data());
    return minKey;
}

BinaryValue IndexKeyTraits::getMaxKey() const
{
    BinaryValue maxKey(getKeySize());
    getMaxKey(maxKey.data());
    return maxKey;
}

}  // namespace siodb::iomgr::dbengine
