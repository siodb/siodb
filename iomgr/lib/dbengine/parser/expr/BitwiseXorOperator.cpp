// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BitwiseXorOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString BitwiseXorOperator::getExpressionText() const
{
    return "Bitwise XOR";
}

Variant BitwiseXorOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    if (leftValue.isNull() || rightValue.isNull()) return nullptr;

    return leftValue ^ rightValue;
}

Expression* BitwiseXorOperator::clone() const
{
    return cloneImpl<BitwiseXorOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
