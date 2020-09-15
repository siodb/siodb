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

/**
 * Applies the given function to a range and stores the result in another range.
 * @tparam InputIt Input range iterator type.
 * @tparam OutputIt Output range iterator type.
 * @tparam UnaryOperation Unary operation type.
 * @tparam UnaryPredicate Unary predicate type.
 * @param first Beginning of the range of elements to transform.
 * @param last End of the range of elements to transform.
 * @param d_first Beginning of the destination range, may be equal to first.
 * @param unary_op Unary operation function object that will be applied.
 * @param pred Unary predicate which returns ​true for the required elements.
 * @return 
 */
template<class InputIt, class OutputIt, class UnaryOperation, class UnaryPredicate>
OutputIt transform_if(
        InputIt first, InputIt last, OutputIt d_first, UnaryOperation unary_op, UnaryPredicate pred)
{
    while (first != last) {
        if (pred(*first)) {
            *d_first = unary_op(*first);
            ++d_first;
        }
        ++first;
    }
    return d_first;
}

/**
 * Applies the given function to a pair of elements from two ranges
 * and stores the result in another range.
 * @tparam InputIt1 First input range iterator type.
 * @tparam InputIt2 Second input range iterator type.
 * @tparam OutputIt Output range iterator type.
 * @tparam UnaryOperation Unary operation type.
 * @tparam UnaryPredicate Unary predicate type.
 * @param first1 Beginning of the first range of elements to transform.
 * @param last1 End of the first range of elements to transform.
 * @param first2 Beginning of the second range of elements to transform.
 * @param last2 End of the second range of elements to transform.
 * @param d_first Beginning of the destination range, may be equal to first1 or first2.
 * @param binary_op Binary operation function object that will be applied.
 * @param pred Binary predicate which returns ​true for the required elements.
 * @return 
 */
template<class InputIt1, class InputIt2, class OutputIt, class BinaryOperation,
        class BinaryPredicate>
OutputIt transform_if(InputIt1 first1, InputIt1 last1, InputIt2 first2, OutputIt d_first,
        BinaryOperation binary_op, BinaryPredicate pred)
{
    while (first1 != last1) {
        if (pred(*first1, *first2)) {
            *d_first = binary_op(*first1, *first2);
            ++d_first;
        }
        ++first1;
        ++first2;
    }
    return d_first;
}

}  // namespace stdext
