// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "GreaterOrEqualOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString GreaterOrEqualOperator::getExpressionText() const
{
    return "GREATER OR EQUAL";
}

Variant GreaterOrEqualOperator::evaluate(Context& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    // TODO: SIODB-172
    if (leftValue.isNull() || rightValue.isNull()) return false;

    return leftValue.compatibleGreaterOrEqual(rightValue);
}

Expression* GreaterOrEqualOperator::clone() const
{
    return cloneImpl<GreaterOrEqualOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
