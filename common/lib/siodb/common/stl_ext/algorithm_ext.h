// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace stdext {

/**
 * Converts range to map, using sequential numbers starting from @ref key as keys.
 * @tparam Map Resulting map type.
 * @tparam ForwardIterator Range iterator type.
 * @param first Range start.
 * @param last Range end.
 * @param key First value of the key.
 * @return Map object.
 */
template<class Map, class ForwardIterator>
Map range_to_map(ForwardIterator first, ForwardIterator last, typename Map::key_type key)
{
    Map result;
    while (first != last)
        result.emplace(key++, *first++);
    return result;
}

/**
 * Converts range to map, using values as keys and sequential numbers starting
 * from value as values.
 * @tparam Map Resulting map type.
 * @tparam ForwardIterator Range iterator type.
 * @param first Range start.
 * @param last Range end.
 * @param value First value to use.
 * @return Map object.
 */
template<class Map, class ForwardIterator>
Map range_to_value_map(ForwardIterator first, ForwardIterator last, typename Map::mapped_type value)
{
    Map result;
    while (first != last)
        result.emplace(*first++, value++);
    return result;
}

}  // namespace stdext
