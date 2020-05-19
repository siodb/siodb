// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UnaryPlusOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString UnaryPlusOperator::getExpressionText() const
{
    return "UNARY PLUS";
}

Variant UnaryPlusOperator::evaluate(Context& context) const
{
    const auto value = m_operand->evaluate(context);
    if (value.isNull()) return nullptr;

    return +value;
}

Expression* UnaryPlusOperator::clone() const
{
    return cloneImpl<UnaryPlusOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
