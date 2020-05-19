// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "dbengine/parser/expr/AllExpressions.h"

// STL headers
#include <initializer_list>

namespace dbengine = siodb::iomgr::dbengine;
namespace requests = dbengine::requests;

// ------------------------------- EXPRESSION FACTORIES ----------------------------------------

template<class T>
inline requests::ExpressionPtr makeConstant(const T& value)
{
    return std::make_unique<requests::ConstantExpression>(dbengine::Variant(value));
}

template<typename ExprType, typename T1>
requests::ExpressionPtr makeUnaryOperator(const T1& v1)
{
    return std::make_unique<ExprType>(makeConstant(v1));
}

template<typename ExprType, typename T1, typename T2>
requests::ExpressionPtr makeBinaryOperator(const T1& v1, const T2& v2)
{
    return std::make_unique<ExprType>(makeConstant(v1), makeConstant(v2));
}

template<typename ExprType, typename T1, typename T2, typename T3>
requests::ExpressionPtr makeTernaryOperator(const T1& v1, const T2& v2, const T3& v3)
{
    return std::make_unique<ExprType>(makeConstant(v1), makeConstant(v2), makeConstant(v3));
}

template<typename T1>
inline requests::ExpressionPtr makeUnaryPlus(const T1& v1)
{
    return makeUnaryOperator<requests::UnaryPlusOperator, T1>(v1);
}

template<typename T1>
inline requests::ExpressionPtr makeUnaryMinus(const T1& v1)
{
    return makeUnaryOperator<requests::UnaryMinusOperator, T1>(v1);
}

template<typename T1>
inline requests::ExpressionPtr makeComplement(const T1& v1)
{
    return makeUnaryOperator<requests::ComplementOperator, T1>(v1);
}

template<typename T1>
inline requests::ExpressionPtr makeNot(const T1& v1)
{
    return makeUnaryOperator<requests::LogicalNotOperator, T1>(v1);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeAddition(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::AddOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeSubstraction(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::SubtractOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeMultiplication(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::MultiplyOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeDivision(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::DivideOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeModulo(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::ModuloOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeEqual(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::EqualOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeGreater(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::GreaterOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeGreaterOrEqual(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::GreaterOrEqualOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeLess(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::LessOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeLessOrEqual(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::LessOrEqualOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeLeftShift(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::LeftShiftOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeRightShift(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::RightShiftOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeBitwiseOr(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::BitwiseOrOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeBitwiseAnd(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::BitwiseAndOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeAnd(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::LogicalAndOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeOr(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::LogicalOrOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2>
inline requests::ExpressionPtr makeConcatenation(const T1& v1, const T2& v2)
{
    return makeBinaryOperator<requests::ConcatenationOperator, T1, T2>(v1, v2);
}

template<typename T1, typename T2, typename T3>
requests::ExpressionPtr makeBetween(const T1& v1, const T2& v2, const T3& v3, bool notBetween)
{
    return std::make_unique<requests::BetweenOperator>(
            makeConstant(v1), makeConstant(v2), makeConstant(v3), notBetween);
}

template<typename T1, typename T2>
requests::ExpressionPtr makeLike(const T1& v1, const T2& v2, bool notLike)
{
    return std::make_unique<requests::LikeOperator>(makeConstant(v1), makeConstant(v2), notLike);
}

template<typename T1, typename T2>
requests::ExpressionPtr makeIs(const T1& v1, const T2& v2, bool isNot)
{
    return std::make_unique<requests::IsOperator>(makeConstant(v1), makeConstant(v2), isNot);
}

template<typename ValueType, typename VariantType>
requests::ExpressionPtr makeIn(
        const ValueType& value, const std::initializer_list<VariantType>& variants, bool notIn)
{
    auto valueExpr = makeConstant(value);

    std::vector<requests::ExpressionPtr> variantsExpr;
    variantsExpr.reserve(variants.size());
    for (const auto& variant : variants)
        variantsExpr.push_back(makeConstant(variant));

    return std::make_unique<requests::InOperator>(
            std::move(valueExpr), std::move(variantsExpr), notIn);
}

template<typename VariantType>
requests::ExpressionPtr makeInWithColumn(requests::ExpressionPtr&& valueExpr,
        const std::initializer_list<VariantType>& variants, bool notIn)
{
    std::vector<requests::ExpressionPtr> variantsExpr;
    variantsExpr.reserve(variants.size());
    for (const auto& variant : variants)
        variantsExpr.push_back(makeConstant(variant));

    return std::make_unique<requests::InOperator>(
            std::move(valueExpr), std::move(variantsExpr), notIn);
}
