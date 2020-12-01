// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "LikeOperator.h"
//#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
//#include "../../ThrowDatabaseError.h"

#include <utf8cpp/utf8.h>

namespace siodb::iomgr::dbengine::requests {

VariantType LikeOperator::getResultValueType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return VariantType::kBool;
}

ColumnDataType LikeOperator::getColumnDataType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return COLUMN_DATA_TYPE_BOOL;
}

MutableOrConstantString LikeOperator::getExpressionText() const
{
    return m_notLike ? "NOT LIKE" : "LIKE";
}

std::size_t LikeOperator::getSerializedSize() const noexcept
{
    return BinaryOperator::getSerializedSize() + 1;
}

void LikeOperator::validate(const ExpressionEvaluationContext& context) const
{
    m_left->validate(context);
    m_right->validate(context);

    const auto leftResultType = m_left->getResultValueType(context);
    if (!isStringType(leftResultType) && !isNullType(leftResultType))
        throw std::runtime_error("LIKE operator: left operand type isn't string");

    const auto rightResultType = m_right->getResultValueType(context);
    if (!isStringType(rightResultType) && !isNullType(rightResultType))
        throw std::runtime_error("LIKE operator: right operand type isn't string");
}

Variant LikeOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto value = m_left->evaluate(context);
    const auto pattern = m_right->evaluate(context);

    if (value.isNull() || pattern.isNull()) {
        // TODO: SIODB-172
        return false;
    }

    if (!value.isString()) {
        throw std::runtime_error("LIKE operator: left operand isn't string");
        //throwDatabaseError(IOManagerMessageId::kErrorLikeValueTypeIsWrong,
        //        getColumnDataTypeName(convertVariantTypeToColumnDataType(value.getValueType())));
    }

    if (!pattern.isString()) {
        throw std::runtime_error("LIKE operator: right operand isn't string");
        //throwDatabaseError(IOManagerMessageId::kErrorLikePatternTypeIsWrong,
        //        getColumnDataTypeName(convertVariantTypeToColumnDataType(pattern.getValueType())));
    }

    const auto& valueStr = value.getString();
    const auto& patternStr = pattern.getString();
    return matchPattern(valueStr.c_str(), valueStr.c_str() + valueStr.length(), patternStr.c_str(),
                   patternStr.c_str() + patternStr.length())
           != m_notLike;
}

std::uint8_t* LikeOperator::serializeUnchecked(std::uint8_t* buffer) const
{
    buffer = BinaryOperator::serializeUnchecked(buffer);
    *buffer = m_notLike ? 1 : 0;
    return buffer + 1;
}

Expression* LikeOperator::clone() const
{
    ExpressionPtr left(m_left->clone()), right(m_right->clone());
    return new LikeOperator(std::move(left), std::move(right), m_notLike);
}

// ----- internals -----

bool LikeOperator::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherLikeOperator = static_cast<const LikeOperator&>(other);
    return m_notLike == otherLikeOperator.m_notLike && *m_left == *otherLikeOperator.m_left
           && *m_right == *otherLikeOperator.m_right;
}

bool LikeOperator::matchPattern(
        const char* str, const char* strEnd, const char* pattern, const char* patternEnd)
{
    if (str == strEnd) return pattern == patternEnd;

    constexpr char kAnyChar = '_';
    constexpr char kAnyCharSeq = '%';

    // Last occurrences of the '%'
    const char* strLastAnyCharSeq = nullptr;
    const char* patternLastAnyCharSeq = nullptr;

    while (str < strEnd) {
        const auto patternStart = pattern;
        const auto patternChar = (pattern != patternEnd) ? utf8::next(pattern, patternEnd) : 0;
        if (patternChar == kAnyChar)
            utf8::next(str, strEnd);
        else if (patternChar == kAnyCharSeq) {
            // Remember last point where '%' has occurred
            strLastAnyCharSeq = str;
            patternLastAnyCharSeq = patternStart;
        } else if (utf8::next(str, strEnd) == patternChar)
            continue;
        else if (patternLastAnyCharSeq != nullptr) {
            // If pattern character is not equal to the str character,
            // go back to the last '%' point and advance 'str' to a next character
            str = strLastAnyCharSeq;
            pattern = patternLastAnyCharSeq;
            utf8::next(str, strEnd);
            strLastAnyCharSeq = str;
        } else
            return false;
    }

    // Skip all '%'
    while (pattern < patternEnd) {
        const auto patternChar = utf8::next(pattern, patternEnd);
        if (patternChar != kAnyCharSeq) return false;
    }

    // Pattern is empty, string is empty, so string is matched
    return true;
}

}  // namespace siodb::iomgr::dbengine::requests
