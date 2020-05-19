// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "LogicalAndOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString LogicalAndOperator::getExpressionText() const
{
    return "Logical AND";
}

Variant LogicalAndOperator::evaluate(Context& context) const
{
    const auto leftVal = m_left->evaluate(context);
    if (leftVal.isNull()) return nullptr;

    if (!leftVal.isBool()) throw std::runtime_error("Left value isn't bool");
    if (!leftVal.getBool()) return false;

    const auto rightVal = m_right->evaluate(context);
    if (rightVal.isNull()) return nullptr;

    if (!rightVal.isBool()) throw std::runtime_error("Right value isn't bool");
    return rightVal.getBool();
}

Expression* LogicalAndOperator::clone() const
{
    return cloneImpl<LogicalAndOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
