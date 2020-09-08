// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "NotEqualOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString NotEqualOperator::getExpressionText() const
{
    return "NOT EQUAL";
}

Variant NotEqualOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);
    return (leftValue.isNull() || rightValue.isNull()) ? false
                                                       : !leftValue.compatibleEqual(rightValue);
}

Expression* NotEqualOperator::clone() const
{
    return cloneImpl<NotEqualOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
