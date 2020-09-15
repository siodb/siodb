// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ListExpression.h"

namespace siodb::iomgr::dbengine::requests {

VariantType ListExpression::getResultValueType(const ExpressionEvaluationContext& context) const
{
    return m_items.empty() ? VariantType::kNull : m_items.back()->getResultValueType(context);
}

ColumnDataType ListExpression::getColumnDataType(const ExpressionEvaluationContext& context) const
{
    return m_items.empty() ? COLUMN_DATA_TYPE_UNKNOWN : m_items.back()->getColumnDataType(context);
}

MutableOrConstantString ListExpression::getExpressionText() const
{
    return "LIST";
}

std::size_t ListExpression::getSerializedSize() const noexcept
{
    auto result = getExpressionTypeSerializedSize(m_type) + ::getVarIntSize(m_items.size());
    for (const auto& item : m_items)
        result += item->getSerializedSize();
    return result;
}

void ListExpression::validate(const ExpressionEvaluationContext& context) const
{
    for (const auto& item : m_items)
        item->validate(context);
}

Variant ListExpression::evaluate(ExpressionEvaluationContext& context) const
{
    Variant v;
    if (!m_items.empty()) {
        for (const auto& item : m_items)
            v = item->evaluate(context);
    }
    return v;
}

std::uint8_t* ListExpression::serializeUnchecked(std::uint8_t* buffer) const
{
    buffer = serializeExpressionTypeUnchecked(m_type, buffer);
    buffer = ::encodeVarInt(m_items.size(), buffer);
    for (const auto& item : m_items)
        buffer = item->serializeUnchecked(buffer);
    return buffer;
}

Expression* ListExpression::clone() const
{
    std::vector<ExpressionPtr> items;
    if (!m_items.empty()) {
        items.reserve(m_items.size());
        for (const auto& item : m_items) {
            // NOTE: works correctly w/o intermediate unqiue_ptr,
            // because we have reserved space
            items.emplace_back(item->clone());
        }
    }
    return new ListExpression(std::move(items));
}

// ----- internals -----

bool ListExpression::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherListExpr = static_cast<const ListExpression&>(other);
    const auto n = m_items.size();
    if (n != otherListExpr.m_items.size()) return false;
    for (std::size_t i = 0; i < n; ++i) {
        if (*m_items[i] != *otherListExpr.m_items[i]) return false;
    }
    return true;
}

void ListExpression::dumpImpl(std::ostream& os) const
{
    os << " L=" << m_items.size();
    if (!m_items.empty()) {
        const auto n = m_items.size();
        for (std::size_t i = 0; i < n; ++i) {
            if (n > 0)
                os << ", ";
            else
                os << ' ';
            os << '[' << i << "]:";
            m_items[i]->dump(os);
        }
    }
}

}  // namespace siodb::iomgr::dbengine::requests
