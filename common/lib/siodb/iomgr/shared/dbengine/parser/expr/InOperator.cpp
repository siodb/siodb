// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "InOperator.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

// STL headers
#include <algorithm>
#include <sstream>

namespace siodb::iomgr::dbengine::requests {

InOperator::InOperator(
        ExpressionPtr&& value, std::vector<ExpressionPtr>&& variants, bool notIn) noexcept
    : Expression(ExpressionType::kInPredicate)
    , m_value(std::move(value))
    , m_variants(std::move(variants))
    , m_notIn(notIn)
{
}

VariantType InOperator::getResultValueType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return VariantType::kBool;
}

ColumnDataType InOperator::getColumnDataType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return COLUMN_DATA_TYPE_BOOL;
}

MutableOrConstantString InOperator::getExpressionText() const
{
    return m_notIn ? "NOT IN" : "IN";
}

void InOperator::validate(const ExpressionEvaluationContext& context) const
{
    m_value->validate(context);
    const auto resultType = m_value->getResultValueType(context);

    VariantType expectedType = VariantType::kNull;
    if (isBoolType(resultType))
        expectedType = VariantType::kBool;
    else if (isNumericType(resultType))
        // Int32 for any numeric type
        expectedType = VariantType::kInt32;
    else if (isDateTimeType(resultType) || m_value->canCastAsDateTime(context))
        expectedType = VariantType::kDateTime;
    else if (isStringType(resultType))
        expectedType = VariantType::kString;
    else if (isBinaryType(resultType))
        expectedType = VariantType::kBinary;

    size_t index = 0;
    for (const auto& variant : m_variants) {
        ++index;

        variant->validate(context);
        const auto resultType = variant->getResultValueType(context);
        if (isNullType(resultType)) continue;

        switch (expectedType) {
            case VariantType::kNull: break;
            case VariantType::kBool: {
                if (isBoolType(resultType)) break;
                std::ostringstream err;
                err << "IN operator: Variant #" << index << " type is not boolean";
                throw std::runtime_error(err.str());
            }
            case VariantType::kInt32: {
                if (isNumericType(resultType)) break;
                std::ostringstream err;
                err << "IN operator: Variant #" << index << " type is not numeric";
                throw std::runtime_error(err.str());
            }
            case VariantType::kDateTime: {
                if (isDateTimeType(resultType) || variant->canCastAsDateTime(context)) break;
                std::ostringstream err;
                err << "IN operator: Variant #" << index
                    << " type is not a timestamp or has invalid format";
                throw std::runtime_error(err.str());
            }
            case VariantType::kString: {
                if (isStringType(resultType)) break;
                std::ostringstream err;
                err << "IN operator: Variant #" << index << " type is not string";
                throw std::runtime_error(err.str());
            }
            case VariantType::kBinary: {
                if (isBinaryType(resultType)) break;
                std::ostringstream err;
                err << "IN operator: Variant #" << index << " type is not binary";
                throw std::runtime_error(err.str());
            }
            default: throw std::runtime_error("IN operator: Unexpected error happened");
        }
    }
}

Variant InOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto value = m_value->evaluate(context);
    if (value.isNull()) {
        // TODO: SIODB-172
        return false;
    }

    const auto variantIter =
            std::find_if(m_variants.begin(), m_variants.end(), [&](const auto& variantExpr) {
                const auto variantValue = variantExpr->evaluate(context);
                if (variantValue.isNull()) {
                    // TODO: SIODB-172
                    return false;
                }
                return variantValue.compatibleEqual(value);
            });
    return m_notIn != (variantIter != m_variants.end());
}

std::size_t InOperator::getSerializedSize() const noexcept
{
    std::size_t n = getExpressionTypeSerializedSize(m_type) + m_value->getSerializedSize()
                    + ::getVarIntSize(m_variants.size()) + 1;
    for (const auto& v : m_variants)
        n += v->getSerializedSize();
    return n;
}

std::uint8_t* InOperator::serializeUnchecked(std::uint8_t* buffer) const
{
    buffer = serializeExpressionTypeUnchecked(m_type, buffer);
    buffer = m_value->serializeUnchecked(buffer);
    buffer = ::encodeVarInt(m_variants.size(), buffer);
    for (const auto& v : m_variants)
        buffer = v->serializeUnchecked(buffer);
    *buffer = m_notIn ? 1 : 0;
    return buffer + 1;
}

Expression* InOperator::clone() const
{
    ExpressionPtr value(m_value->clone());
    std::vector<ExpressionPtr> variants;
    if (!m_variants.empty()) {
        variants.reserve(m_variants.size());
        for (const auto& variant : m_variants) {
            // NOTE: works correctly w/o intermediate unqiue_ptr
            // because we have reserved space
            variants.emplace_back(variant->clone());
        }
    }
    return new InOperator(std::move(value), std::move(variants), m_notIn);
}

// --- internals ---

bool InOperator::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherInOperator = static_cast<const InOperator&>(other);

    if (m_notIn != otherInOperator.m_notIn || *m_value != *otherInOperator.m_value
            || m_variants.size() != otherInOperator.m_variants.size())
        return false;

    const auto n = m_variants.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (*m_variants[i] == *otherInOperator.m_variants[i]) continue;
        return false;
    }

    return true;
}

void InOperator::dumpImpl(std::ostream& os) const
{
    os << " value:" << *m_value << " variants:[";
    const auto n = m_variants.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (i > 0) os << ' ';
        os << *m_variants[i];
    }
    os << ']';
}

}  // namespace siodb::iomgr::dbengine::requests
