// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "LogicalOrOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString LogicalOrOperator::getExpressionText() const
{
    return "Logical OR";
}

Variant LogicalOrOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto leftVal = m_left->evaluate(context);
    if (leftVal.isNull()) return nullptr;
    if (!leftVal.isBool()) throw std::runtime_error("Left value isn't bool");
    if (leftVal.getBool()) return true;

    const auto rightVal = m_right->evaluate(context);
    if (rightVal.isNull()) return nullptr;
    if (!rightVal.isBool()) throw std::runtime_error("Right value isn't bool");
    return rightVal.getBool();
}

Expression* LogicalOrOperator::clone() const
{
    return cloneImpl<LogicalOrOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
