// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SubtractOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString SubtractOperator::getExpressionText() const
{
    return "SUBTRACT";
}

Variant SubtractOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    if (leftValue.isNull() || rightValue.isNull()) return nullptr;

    return leftValue - rightValue;
}

Expression* SubtractOperator::clone() const
{
    return cloneImpl<SubtractOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
