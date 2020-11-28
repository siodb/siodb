// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine::requests {

/** Expression type */
enum class ExpressionType {
    // Primitive expressions
    kConstant,
    kSingleColumnReference,
    kAllColumnsReference,

    // Complex expressions
    kList,
    kSubquery,  // NOT SUPPORTED YET

    // Logical operators
    kLogicalNotOperator,
    kLogicalAndOperator,
    kLogicalOrOperator,

    // Unrary arithmetic operators
    kUnaryMinusOperator,
    kUnaryPlusOperator,

    // Binary artithemtic operators
    kAddOperator,
    kSubtractOperator,
    kMultiplyOperator,
    kDivideOperator,
    kModuloOperator,

    // Concatenation
    kConcatenateOperator,

    // Bitwise operators
    kBitwiseOrOperator,
    kBitwiseAndOperator,
    kBitwiseXorOperator,
    kBitwiseComplementOperator,
    kRightShiftOperator,
    kLeftShiftOperator,

    // Type case
    kCastOperator,  // NOT COMPLETELY SUPPORTED YET

    // Multiple choice
    kCaseOperator,  // NOT SUPPORTED YET

    // Comparisons
    kEqualPredicate,
    kNotEqualPredicate,
    kLessPredicate,
    kLessOrEqualPredicate,
    kGreaterOrEqualPredicate,
    kGreaterPredicate,
    kDistinctFromPredicate,  // NOT SUPPORTED YET

    // Quantified comparisons
    kAllPredicate,  // NOT SUPPORTED YET
    kAnyPredicate,  // NOT SUPPORTED YET
    kSomePredicate,  // NOT SUPPORTED YET

    // Predicates
    kInPredicate,
    kIsPredicate,
    kBetweenPredicate,
    kLikePredicate,
    kMatchPredicate,  // NOT SUPPORTED YET
    kExistsPredicate,  // NOT SUPPORTED YET
    kUniquePredicate,  // NOT SUPPORTED YET
    kOverlapsPredicate,  // NOT SUPPORTED YET
    kSimilarToPredicate,  // NOT SUPPORTED YET
    kIsOfTypePredicate,  // NOT SUPPORTED YET

    // Quantified predicates
    kForAllPredicate,  // NOT SUPPORTED YET
    kForAnyPredicate,  // NOT SUPPORTED YET
    kForSomePredicate,  // NOT SUPPORTED YET

    // Aggreation functions
    kMaxFunction,  // NOT SUPPORTED YET
    kMinFunction,  // NOT SUPPORTED YET
    kSumFunction,  // NOT SUPPORTED YET
    kAvgFunction,  // NOT SUPPORTED YET
    kCountFunction,  // NOT SUPPORTED YET
    kDistinctFunction,  // NOT SUPPORTED YET

    // Text functions
    kSubstringFunction,  // NOT SUPPORTED YET
    kRegexpSubstring,  // NOT SUPPORTED YET
    kOverlayFunction,  // NOT SUPPORTED YET
    kUpperFunction,  // NOT SUPPORTED YET
    kLowerFunction,  // NOT SUPPORTED YET
    kLeftTrimFunction,  // NOT SUPPORTED YET
    kRightTrimFunction,  // NOT SUPPORTED YET
    kTrimFunction,  // NOT SUPPORTED YET
    kCharPositionFunction,  // NOT SUPPORTED YET
    kBitLengthFunction,  // NOT SUPPORTED YET
    kCharLengthFunction,  // NOT SUPPORTED YET
    kOctetLengthFunction,  // NOT SUPPORTED YET

    // Other built-in functions
    kNullIf,  // NOT SUPPORTED YET
    kCoalesce,  // NOT SUPPORTED YET

    // IMPORTANT: WHEN STABLE PUBLIC RELEASE ACHIEVED, ADD NEW EXPRESSION TYPES HERE
    // TO AVOID CONSTANT SHIFTS.

    kMax  // Number of expression types
};

/**
 * Returns serialized size for the given expression type constant.
 * @param expressionType Expression type constant.
 * @return Number of bytes to serialize this value.
 */
unsigned getExpressionTypeSerializedSize(ExpressionType expressionType) noexcept;

/**
 * Serializes expression type, doesn't check buffer size.
 * @param expressionType Expression type to be serialized.
 * @return Address after a last written byte.
 */
std::uint8_t* serializeExpressionTypeUnchecked(
        ExpressionType expressionType, std::uint8_t* buffer) noexcept;

}  // namespace siodb::iomgr::dbengine::requests
