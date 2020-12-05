# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

CXX_SRC+= \
	dbengine/parser/expr/AddOperator.cpp  \
	dbengine/parser/expr/AllColumnsExpression.cpp  \
	dbengine/parser/expr/ArithmeticBinaryOperator.cpp  \
	dbengine/parser/expr/ArithmeticUnaryOperator.cpp  \
	dbengine/parser/expr/BetweenOperator.cpp  \
	dbengine/parser/expr/BinaryOperator.cpp  \
	dbengine/parser/expr/BitwiseAndOperator.cpp  \
	dbengine/parser/expr/BitwiseBinaryOperator.cpp  \
	dbengine/parser/expr/BitwiseOrOperator.cpp  \
	dbengine/parser/expr/BitwiseUnaryOperator.cpp  \
	dbengine/parser/expr/BitwiseXorOperator.cpp  \
	dbengine/parser/expr/CastOperator.cpp  \
	dbengine/parser/expr/ColumnExpressionBase.cpp  \
	dbengine/parser/expr/ComparisonBinaryOperator.cpp  \
	dbengine/parser/expr/ComplementOperator.cpp  \
	dbengine/parser/expr/ConcatenationOperator.cpp  \
	dbengine/parser/expr/ConstantExpression.cpp \
	dbengine/parser/expr/DivideOperator.cpp  \
	dbengine/parser/expr/EqualOperator.cpp  \
	dbengine/parser/expr/Expression.cpp  \
	dbengine/parser/expr/ExpressionType.cpp  \
	dbengine/parser/expr/GreaterOperator.cpp  \
	dbengine/parser/expr/GreaterOrEqualOperator.cpp  \
	dbengine/parser/expr/InOperator.cpp  \
	dbengine/parser/expr/IsOperator.cpp  \
	dbengine/parser/expr/LeftShiftOperator.cpp  \
	dbengine/parser/expr/LessOperator.cpp \
	dbengine/parser/expr/LessOrEqualOperator.cpp  \
	dbengine/parser/expr/LikeOperator.cpp  \
	dbengine/parser/expr/ListExpression.cpp  \
	dbengine/parser/expr/LogicalAndOperator.cpp  \
	dbengine/parser/expr/LogicalBinaryOperator.cpp  \
	dbengine/parser/expr/LogicalUnaryOperator.cpp  \
	dbengine/parser/expr/LogicalOrOperator.cpp  \
	dbengine/parser/expr/LogicalNotOperator.cpp  \
	dbengine/parser/expr/ModuloOperator.cpp  \
	dbengine/parser/expr/MultiplyOperator.cpp  \
	dbengine/parser/expr/NotEqualOperator.cpp  \
	dbengine/parser/expr/RightShiftOperator.cpp  \
	dbengine/parser/expr/SingleColumnExpression.cpp  \
	dbengine/parser/expr/TernaryOperator.cpp  \
	dbengine/parser/expr/SubtractOperator.cpp  \
	dbengine/parser/expr/UnaryOperator.cpp  \
	dbengine/parser/expr/UnaryMinusOperator.cpp  \
	dbengine/parser/expr/UnaryPlusOperator.cpp

CXX_HDR+= \
	dbengine/parser/expr/AllExpressions.h  \
	dbengine/parser/expr/AddOperator.h  \
	dbengine/parser/expr/ArithmeticBinaryOperator.h  \
	dbengine/parser/expr/ArithmeticUnaryOperator.h  \
	dbengine/parser/expr/BetweenOperator.h  \
	dbengine/parser/expr/BinaryOperator.h  \
	dbengine/parser/expr/BitwiseAndOperator.h  \
	dbengine/parser/expr/BitwiseBinaryOperator.h  \
	dbengine/parser/expr/BitwiseOrOperator.h  \
	dbengine/parser/expr/BitwiseUnaryOperator.h  \
	dbengine/parser/expr/BitwiseXorOperator.h  \
	dbengine/parser/expr/CastOperator.h  \
	dbengine/parser/expr/ColumnExpressionBase.h  \
	dbengine/parser/expr/ComparisonBinaryOperator.h  \
	dbengine/parser/expr/ComplementOperator.h  \
	dbengine/parser/expr/ConcatenationOperator.h  \
	dbengine/parser/expr/ConstantExpression.h \
	dbengine/parser/expr/EqualOperator.h  \
	dbengine/parser/expr/Expression.h  \
	dbengine/parser/expr/ExpressionEvaluationContext.h  \
	dbengine/parser/expr/ExpressionType.h  \
	dbengine/parser/expr/GreaterOperator.h  \
	dbengine/parser/expr/GreaterOrEqualOperator.h  \
	dbengine/parser/expr/InOperator.h  \
	dbengine/parser/expr/IsOperator.h  \
	dbengine/parser/expr/LeftShiftOperator.h  \
	dbengine/parser/expr/LessOperator.h \
	dbengine/parser/expr/LessOrEqualOperator.h  \
	dbengine/parser/expr/LikeOperator.h  \
	dbengine/parser/expr/ListExpression.h  \
	dbengine/parser/expr/LogicalAndOperator.h  \
	dbengine/parser/expr/LogicalBinaryOperator.h  \
	dbengine/parser/expr/LogicalUnaryOperator.h  \
	dbengine/parser/expr/LogicalOrOperator.h  \
	dbengine/parser/expr/LogicalNotOperator.h  \
	dbengine/parser/expr/ModuloOperator.h  \
	dbengine/parser/expr/MultiplyOperator.h  \
	dbengine/parser/expr/NotEqualOperator.h  \
	dbengine/parser/expr/RightShiftOperator.h  \
	dbengine/parser/expr/SingleColumnExpression.h  \
	dbengine/parser/expr/SubtractOperator.h  \
	dbengine/parser/expr/TernaryOperator.h  \
	dbengine/parser/expr/UnaryOperator.h  \
	dbengine/parser/expr/UnaryMinusOperator.h  \
	dbengine/parser/expr/UnaryPlusOperator.h
