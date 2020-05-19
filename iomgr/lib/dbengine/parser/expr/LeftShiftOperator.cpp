// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "LeftShiftOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString LeftShiftOperator::getExpressionText() const
{
    return "LEFT SHIFT";
}

Variant LeftShiftOperator::evaluate(Context& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    if (leftValue.isNull() || rightValue.isNull()) return nullptr;

    return leftValue << rightValue;
}

Expression* LeftShiftOperator::clone() const
{
    return cloneImpl<LeftShiftOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
