// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BitwiseOrOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString BitwiseOrOperator::getExpressionText() const
{
    return "Bitwise OR";
}

Variant BitwiseOrOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    if (leftValue.isNull() || rightValue.isNull()) return nullptr;

    return leftValue | rightValue;
}

Expression* BitwiseOrOperator::clone() const
{
    return cloneImpl<BitwiseOrOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
