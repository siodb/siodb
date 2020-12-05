// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ComparisonBinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType ComparisonBinaryOperator::getResultValueType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return VariantType::kBool;
}

ColumnDataType ComparisonBinaryOperator::getColumnDataType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return COLUMN_DATA_TYPE_BOOL;
}

}  // namespace siodb::iomgr::dbengine::requests