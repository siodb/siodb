// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ComplementOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString ComplementOperator::getExpressionText() const
{
    return "Complement";
}

Variant ComplementOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto value = m_operand->evaluate(context);
    if (value.isNull()) return nullptr;
    return ~value;
}

Expression* ComplementOperator::clone() const
{
    return cloneImpl<ComplementOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
