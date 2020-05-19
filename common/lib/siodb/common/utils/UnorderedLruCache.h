// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "LruCache.h"

// STL headers
#include <unordered_map>

namespace siodb::utils {

/** LRU cache specialization with unordered map. */
template<class Key, class Value>
using unordered_lru_cache = basic_lru_cache<Key, Value,
        std::unordered_map<Key, std::pair<Value, typename std::list<Key>::iterator>>,
        std::list<Key>>;

}  // namespace siodb::utils
