// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UnaryMinusOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString UnaryMinusOperator::getExpressionText() const
{
    return "UNARY MINUS";
}

Variant UnaryMinusOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto value = m_operand->evaluate(context);
    if (value.isNull()) return nullptr;

    return -value;
}

Expression* UnaryMinusOperator::clone() const
{
    return cloneImpl<UnaryMinusOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
