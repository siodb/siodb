# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

include parser/antlr_wrappers/Files.mk

CXX_SRC+= \
	parser/AntlrHelpers.cpp  \
	parser/DBEngineRestRequestFactory.cpp  \
	parser/DBEngineSqlRequest.cpp  \
	parser/DBEngineSqlRequestFactory.cpp  \
	parser/DBExpressionEvaluationContext.cpp  \
	parser/EmptyExpressionEvaluationContext.cpp  \
	parser/ExpressionFactory.cpp  \
	parser/RowDataJsonSaxParser.cpp  \
	parser/SqlParser.cpp

CXX_HDR+= \
	parser/AntlrHelpers.h  \
	parser/DBEngineRequest.h  \
	parser/DBEngineRequestFactoryError.h  \
	parser/DBEngineRequestPtr.h  \
	parser/DBEngineRequestType.h  \
	parser/DBEngineRestRequestFactory.h  \
	parser/DBEngineSqlRequest.h  \
	parser/DBEngineSqlRequestFactory.h  \
	parser/DBExpressionEvaluationContext.h  \
	parser/EmptyExpressionEvaluationContext.h  \
	parser/ExpressionFactory.h  \
	parser/JsonParserError.h  \
	parser/RowDataJsonSaxParser.h  \
	parser/SqlParser.h
