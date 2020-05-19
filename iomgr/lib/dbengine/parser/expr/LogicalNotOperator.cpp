// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "LogicalNotOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString LogicalNotOperator::getExpressionText() const
{
    return "Logical NOT";
}

Variant LogicalNotOperator::evaluate(Context& context) const
{
    const auto value = m_operand->evaluate(context);

    if (value.isNull()) return nullptr;

    if (!value.isBool()) throw std::runtime_error("NOT operator: Value isn't bool");

    return !value.getBool();
}

Expression* LogicalNotOperator::clone() const
{
    return cloneImpl<LogicalNotOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
