// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "MultiplyOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString MultiplyOperator::getExpressionText() const
{
    return "MULTIPLY";
}

Variant MultiplyOperator::evaluate(Context& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    if (leftValue.isNull() || rightValue.isNull()) return nullptr;

    return leftValue * rightValue;
}

Expression* MultiplyOperator::clone() const
{
    return cloneImpl<MultiplyOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
