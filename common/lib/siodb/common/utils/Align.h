// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <type_traits>

namespace siodb::utils {

/**
 * Returns nearest higher or equal to value multiple of an alignment factor.
 * IMPORTANT: Works only for alignments 2^N.
 * @tparam ValueType Value type.
 * @tparam AlignmentFactorType Alignement factor type.
 * @param value A value.
 * @param alignmentFactor Alignment factor.
 * @return Adjusted value.
 */
template<class ValueType, class AlignmentFactorType>
inline constexpr ValueType alignUp(ValueType value, AlignmentFactorType alignmentFactor) noexcept
{
    static_assert(std::is_integral_v<ValueType>, "ValueType must be integral");
    static_assert(std::is_integral_v<AlignmentFactorType>, "AlignmentFactorType must be integral");
    return (value + (alignmentFactor - 1)) & -alignmentFactor;
}

/**
 * Returns nearest higher or equal to value multiple of an alignment factor.
 * IMPORTANT: Works only for alignments 2^N.
 * @tparam ValueType Value type.
 * @tparam AlignmentFactorType Alignement factor type.
 * @param value A value.
 * @param alignmentFactor Alignment factor.
 * @return Adjusted value.
 */
template<class ValueType, class AlignmentFactorType>
inline constexpr ValueType alignDown(ValueType value, AlignmentFactorType alignmentFactor) noexcept
{
    static_assert(std::is_integral_v<ValueType>, "ValueType must be integral");
    static_assert(std::is_integral_v<AlignmentFactorType>, "AlignmentFactorType must be integral");
    return value & ~(alignmentFactor - 1);
}

}  // namespace siodb::utils
