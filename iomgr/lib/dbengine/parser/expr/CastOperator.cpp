// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "CastOperator.h"

// Project headers
#include "ConstantExpression.h"
#include "../../ColumnDataType.h"

namespace siodb::iomgr::dbengine::requests {

VariantType CastOperator::getResultValueType(const Context& context) const
{
    return convertColumnDataTypeToVariantType(getColumnDataType(context));
}

ColumnDataType CastOperator::getColumnDataType([[maybe_unused]] const Context& context) const
{
    return getColumnDataTypeByName(
            static_cast<const ConstantExpression&>(*m_right).getValue().getString());
}

MutableOrConstantString CastOperator::getExpressionText() const
{
    return "CAST";
}

void CastOperator::validate(const Context& context) const
{
    m_left->validate(context);
}

Variant CastOperator::evaluate(Context& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto resultType = getResultValueType(context);
    switch (resultType) {
        case VariantType::kBool: return leftValue.asBool();
        case VariantType::kInt8: return leftValue.asInt8();
        case VariantType::kUInt8: return leftValue.asUInt8();
        case VariantType::kInt16: return leftValue.asInt16();
        case VariantType::kUInt16: return leftValue.asUInt16();
        case VariantType::kInt32: return leftValue.asInt32();
        case VariantType::kUInt32: return leftValue.asInt32();
        case VariantType::kInt64: return leftValue.asInt64();
        case VariantType::kUInt64: return leftValue.asUInt64();
        case VariantType::kFloat: return leftValue.asFloat();
        case VariantType::kDouble: return leftValue.asDouble();
        case VariantType::kDateTime: return leftValue.asDateTime();
        case VariantType::kString: return leftValue.asString().release();
        case VariantType::kBinary: return leftValue.asBinary().release();
        case VariantType::kClob: return leftValue.asClob().release();
        case VariantType::kBlob: return leftValue.asBlob().release();
        default: {
            const auto s = *m_right->evaluate(context).asString();
            throw std::runtime_error("Unsupported type cast to " + s);
        }
    };
}

Expression* CastOperator::clone() const
{
    return cloneImpl<CastOperator>();
}

// ----- internals -----

void CastOperator::checkRightIsStringConstant() const
{
    if (!m_right->isConstant()
            || !isStringType(
                    static_cast<const ConstantExpression&>(*m_right).getValue().getValueType()))
        throw std::runtime_error("Cast operator: destination type name is not a constant string");
}

}  // namespace siodb::iomgr::dbengine::requests
