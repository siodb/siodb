// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BitwiseAndOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString BitwiseAndOperator::getExpressionText() const
{
    return "Bitwise AND";
}

Variant BitwiseAndOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    if (leftValue.isNull() || rightValue.isNull()) return nullptr;

    return leftValue & rightValue;
}

Expression* BitwiseAndOperator::clone() const
{
    return cloneImpl<BitwiseAndOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
