// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "EqualOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString EqualOperator::getExpressionText() const
{
    return "EQUAL";
}

Variant EqualOperator::evaluate(Context& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);
    // Note: Variant::compatibleEqual() handles SQL NULLs correctly.
    return leftValue.compatibleEqual(rightValue);
}

Expression* EqualOperator::clone() const
{
    return cloneImpl<EqualOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
