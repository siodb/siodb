// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "AllExpressions.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

namespace siodb::iomgr::dbengine::requests {

bool Expression::isConstant() const noexcept
{
    return false;
}

bool Expression::isUnaryOperator() const noexcept
{
    return false;
}

bool Expression::isBinaryOperator() const noexcept
{
    return false;
}

bool Expression::isTernaryOperator() const noexcept
{
    return false;
}

bool Expression::canCastAsDateTime(const ExpressionEvaluationContext& context) const noexcept
{
    return isDateTimeType(getResultValueType(context));
}

namespace {

template<class ExprT>
std::size_t deserializeUnaryExpression(
        const std::uint8_t* buffer, std::size_t length, ExpressionPtr& result)
{
    ExpressionPtr operand;
    const auto consumed = Expression::deserialize(buffer, length, operand);
    result = std::make_unique<ExprT>(std::move(operand));
    return consumed;
}

template<class ExprT>
std::size_t deserializeBinaryExpression(
        const std::uint8_t* buffer, std::size_t length, ExpressionPtr& result)
{
    ExpressionPtr left;
    auto consumed = Expression::deserialize(buffer, length, left);

    ExpressionPtr right;
    consumed += Expression::deserialize(buffer + consumed, length - consumed, right);

    result = std::make_unique<ExprT>(std::move(left), std::move(right));
    return consumed;
}

template<class ExprT>
std::size_t deserializeTernaryExpression(
        const std::uint8_t* buffer, std::size_t length, ExpressionPtr& result)
{
    ExpressionPtr left;
    auto consumed = Expression::deserialize(buffer, length, left);

    ExpressionPtr middle;
    consumed += Expression::deserialize(buffer + consumed, length - consumed, middle);

    ExpressionPtr right;
    consumed += Expression::deserialize(buffer + consumed, length - consumed, right);

    result = std::make_unique<ExprT>(std::move(left), std::move(middle), std::move(right));
    return consumed;
}

}  // anonymous namespace

std::size_t Expression::deserialize(
        const std::uint8_t* buffer, std::size_t length, ExpressionPtr& result)
{
    std::uint32_t expressionType = 0;
    const int consumedForType = ::decodeVarInt(buffer, length, expressionType);

    if (SIODB_UNLIKELY(consumedForType < 0)) throw std::runtime_error("Corrupt expression type");

    if (SIODB_UNLIKELY(consumedForType == 0))
        throw std::runtime_error("Not enough data for the expression type");

    if (SIODB_UNLIKELY(expressionType >= static_cast<std::uint32_t>(ExpressionType::kMax)))
        throw std::runtime_error("Invalid expression type: " + std::to_string(expressionType));

    std::size_t consumed = consumedForType;

    switch (static_cast<ExpressionType>(expressionType)) {
        case ExpressionType::kConstant: {
            Variant v;
            consumed += v.deserialize(buffer + consumed, length - consumed);
            result = std::make_unique<ConstantExpression>(std::move(v));
            return consumed;
        }
        case ExpressionType::kSingleColumnReference: {
            std::string tableName;
            consumed += ::deserializeObject(buffer + consumed, length - consumed, tableName);
            std::string columnName;
            consumed += ::deserializeObject(buffer + consumed, length - consumed, columnName);
            result = std::make_unique<SingleColumnExpression>(
                    std::move(tableName), std::move(columnName));
            return consumed;
        }
        case ExpressionType::kAllColumnsReference: {
            std::string tableName;
            consumed += ::deserializeObject(buffer + consumed, length - consumed, tableName);
            result = std::make_unique<AllColumnsExpression>(std::move(tableName));
            return consumed;
        }
        case ExpressionType::kList: {
            std::uint64_t itemCount = 0;
            const int consumed1 = ::decodeVarInt(buffer + consumed, length - consumed, itemCount);

            if (SIODB_UNLIKELY(consumed1 < 0))
                throw VariantDeserializationError("Corrupt item count");

            if (SIODB_UNLIKELY(consumed1 == 0)) {
                throw VariantDeserializationError(
                        "Not enough data for the item count: " + std::to_string(length - consumed));
            }

            consumed += consumed1;

            std::vector<ExpressionPtr> items;
            if (itemCount > 0) {
                items.reserve(itemCount);
                for (std::size_t i = 0; i < itemCount; ++i) {
                    ExpressionPtr item;
                    consumed += Expression::deserialize(buffer + consumed, length - consumed, item);
                    items.push_back(std::move(item));
                }
            }

            result = std::make_unique<ListExpression>(std::move(items));
            return consumed;
        }
        case ExpressionType::kLogicalNotOperator: {
            return consumed
                   + deserializeUnaryExpression<LogicalNotOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kLogicalAndOperator: {
            return consumed
                   + deserializeBinaryExpression<LogicalAndOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kLogicalOrOperator: {
            return consumed
                   + deserializeBinaryExpression<LogicalOrOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kLessPredicate: {
            return consumed
                   + deserializeBinaryExpression<LessOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kLessOrEqualPredicate: {
            return consumed
                   + deserializeBinaryExpression<LessOrEqualOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kEqualPredicate: {
            return consumed
                   + deserializeBinaryExpression<EqualOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kNotEqualPredicate: {
            return consumed
                   + deserializeBinaryExpression<NotEqualOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kGreaterOrEqualPredicate: {
            return consumed
                   + deserializeBinaryExpression<GreaterOrEqualOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kGreaterPredicate: {
            return consumed
                   + deserializeBinaryExpression<GreaterOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kUnaryMinusOperator: {
            return consumed
                   + deserializeUnaryExpression<UnaryMinusOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kUnaryPlusOperator: {
            return consumed
                   + deserializeUnaryExpression<UnaryPlusOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kAddOperator: {
            return consumed
                   + deserializeBinaryExpression<AddOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kSubtractOperator: {
            return consumed
                   + deserializeBinaryExpression<SubtractOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kDivideOperator: {
            return consumed
                   + deserializeBinaryExpression<DivideOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kMultiplyOperator: {
            return consumed
                   + deserializeBinaryExpression<MultiplyOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kModuloOperator: {
            return consumed
                   + deserializeBinaryExpression<ModuloOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kLikePredicate: {
            ExpressionPtr left;
            consumed += Expression::deserialize(buffer + consumed, length - consumed, left);

            ExpressionPtr right;
            consumed += Expression::deserialize(buffer + consumed, length - consumed, right);

            if (SIODB_UNLIKELY(consumed == length))
                throw VariantDeserializationError("Not enough data for the notLike attribute");
            if (SIODB_UNLIKELY(buffer[consumed] > 1))
                throw VariantDeserializationError("Invalid notLike attribute");
            const bool notLike = buffer[consumed++] == 1;

            result = std::make_unique<LikeOperator>(std::move(left), std::move(right), notLike);
            return consumed;
        }
        case ExpressionType::kBetweenPredicate: {
            ExpressionPtr left;
            consumed += Expression::deserialize(buffer + consumed, length - consumed, left);

            ExpressionPtr middle;
            consumed += Expression::deserialize(buffer + consumed, length - consumed, middle);

            ExpressionPtr right;
            consumed += Expression::deserialize(buffer + consumed, length - consumed, right);

            if (SIODB_UNLIKELY(consumed == length)) {
                throw VariantDeserializationError("Not enough data for the notBetween attribute");
            }
            if (SIODB_UNLIKELY(buffer[consumed] > 1))
                throw VariantDeserializationError("Invalid notBetween attribute");
            const bool notBetween = buffer[consumed++] == 1;

            result = std::make_unique<BetweenOperator>(
                    std::move(left), std::move(middle), std::move(right), notBetween);
            return consumed;
        }
        case ExpressionType::kBitwiseOrOperator: {
            return consumed
                   + deserializeBinaryExpression<BitwiseOrOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kBitwiseAndOperator: {
            return consumed
                   + deserializeBinaryExpression<BitwiseAndOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kBitwiseXorOperator: {
            return consumed
                   + deserializeBinaryExpression<BitwiseXorOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kBitwiseComplementOperator: {
            return consumed
                   + deserializeUnaryExpression<ComplementOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kRightShiftOperator: {
            return consumed
                   + deserializeBinaryExpression<RightShiftOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kLeftShiftOperator: {
            return consumed
                   + deserializeBinaryExpression<LeftShiftOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kConcatenateOperator: {
            return consumed
                   + deserializeBinaryExpression<ConcatenationOperator>(
                           buffer + consumed, length - consumed, result);
        }
        case ExpressionType::kInPredicate: {
            ExpressionPtr value;
            consumed += Expression::deserialize(buffer + consumed, length - consumed, value);

            std::uint64_t variantCount = 0;
            const int consumed1 =
                    ::decodeVarInt(buffer + consumed, length - consumed, variantCount);

            if (SIODB_UNLIKELY(consumed1 < 0))
                throw VariantDeserializationError("Corrupt variant count");

            if (SIODB_UNLIKELY(consumed1 == 0)) {
                throw VariantDeserializationError("Not enough data for the variant count: "
                                                  + std::to_string(length - consumed));
            }

            consumed += consumed1;

            std::vector<ExpressionPtr> variants;
            variants.reserve(variantCount);
            for (std::uint64_t i = 0; i < variantCount; ++i) {
                ExpressionPtr v;
                consumed += Expression::deserialize(buffer + consumed, length - consumed, v);
                variants.push_back(std::move(v));
            }

            if (SIODB_UNLIKELY(consumed == length))
                throw VariantDeserializationError("Not enough data for the notIn attribute");
            if (SIODB_UNLIKELY(buffer[consumed] > 1))
                throw VariantDeserializationError("Invalid notIn attribute");
            const bool notIn = buffer[consumed++] == 1;

            result = std::make_unique<InOperator>(std::move(value), std::move(variants), notIn);
            return consumed;
        }
        case ExpressionType::kIsPredicate: {
            ExpressionPtr left;
            consumed += Expression::deserialize(buffer + consumed, length - consumed, left);

            ExpressionPtr right;
            consumed += Expression::deserialize(buffer + consumed, length - consumed, right);

            if (SIODB_UNLIKELY(consumed == length))
                throw VariantDeserializationError("Not enough data for the isNot attribute");
            if (SIODB_UNLIKELY(buffer[consumed] > 1))
                throw VariantDeserializationError("Invalid isNot attribute");
            const bool isNot = buffer[consumed++] == 1;

            result = std::make_unique<IsOperator>(std::move(left), std::move(right), isNot);
            return consumed;
        }
        case ExpressionType::kCastOperator: {
            return consumed
                   + deserializeBinaryExpression<CastOperator>(
                           buffer + consumed, length - consumed, result);
        }
        default: {
            throw std::runtime_error("Deserailization of the expression type #"
                                     + std::to_string(expressionType) + " is not supported");
        }
    }
}

void Expression::dump(std::ostream& os) const
{
    os << '(' << getExpressionText() << ' ';
    dumpImpl(os);
    os << ')';
}

}  // namespace siodb::iomgr::dbengine::requests
