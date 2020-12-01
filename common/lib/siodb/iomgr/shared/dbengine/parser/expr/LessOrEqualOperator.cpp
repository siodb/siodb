// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "LessOrEqualOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString LessOrEqualOperator::getExpressionText() const
{
    return "LESS OR EQUAL";
}

Variant LessOrEqualOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    // TODO: SIODB-172
    if (leftValue.isNull() || rightValue.isNull()) return false;

    return leftValue.compatibleLessOrEqual(rightValue);
}

Expression* LessOrEqualOperator::clone() const
{
    return cloneImpl<LessOrEqualOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
