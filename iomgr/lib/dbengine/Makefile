# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# IO manager library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_LIB:=iomgr_dbengine

CXX_SRC:= \
	crypto/GetCipher.cpp  \
	\
	bpt/BPlusTreeIndex.cpp  \
	\
	handlers/RequestHandler_Common.cpp  \
	handlers/RequestHandler_DDL.cpp  \
	handlers/RequestHandler_DML.cpp  \
	handlers/RequestHandler_DQL.cpp  \
	handlers/RequestHandler_TC.cpp  \
	handlers/RequestHandler_UM.cpp  \
	\
	ikt/IndexKeyTraits.cpp  \
	ikt/Int16IndexKeyTraits.cpp  \
	ikt/Int32IndexKeyTraits.cpp  \
	ikt/Int64IndexKeyTraits.cpp  \
	ikt/Int8IndexKeyTraits.cpp  \
	ikt/UInt16IndexKeyTraits.cpp  \
	ikt/UInt32IndexKeyTraits.cpp  \
	ikt/UInt64IndexKeyTraits.cpp  \
	ikt/UInt8IndexKeyTraits.cpp  \
	\
	lob/ColumnBlobStream.cpp  \
	lob/ColumnClobStream.cpp  \
	lob/ColumnLobStream.cpp  \
	\
	parser/AntlrHelpers.cpp  \
	parser/DBEngineRequest.cpp  \
	parser/DBEngineRequestFactory.cpp  \
	parser/DatabaseContext.cpp  \
	parser/EmptyContext.cpp  \
	parser/SqlParser.cpp  \
	parser/antlr_wrappers/SiodbBaseListenerWrapper.cpp  \
	parser/antlr_wrappers/SiodbLexerWrapper.cpp  \
	parser/antlr_wrappers/SiodbListenerWrapper.cpp  \
	parser/antlr_wrappers/SiodbParserWrapper.cpp  \
	parser/antlr_wrappers/SiodbVisitorWrapper.cpp  \
	\
	parser/expr/AddOperator.cpp  \
	parser/expr/AllColumnsExpression.cpp  \
	parser/expr/ArithmeticBinaryOperator.cpp  \
	parser/expr/ArithmeticUnaryOperator.cpp  \
	parser/expr/BetweenOperator.cpp  \
	parser/expr/BinaryOperator.cpp  \
	parser/expr/BitwiseAndOperator.cpp  \
	parser/expr/BitwiseBinaryOperator.cpp  \
	parser/expr/BitwiseOrOperator.cpp  \
	parser/expr/BitwiseUnaryOperator.cpp  \
	parser/expr/BitwiseXorOperator.cpp  \
	parser/expr/CastOperator.cpp  \
	parser/expr/ColumnExpressionBase.cpp  \
	parser/expr/ComparisonBinaryOperator.cpp  \
	parser/expr/ComplementOperator.cpp  \
	parser/expr/ConcatenationOperator.cpp  \
	parser/expr/ConstantExpression.cpp \
	parser/expr/DivideOperator.cpp  \
	parser/expr/EqualOperator.cpp  \
	parser/expr/Expression.cpp  \
	parser/expr/ExpressionFactory.cpp  \
	parser/expr/ExpressionType.cpp  \
	parser/expr/GreaterOperator.cpp  \
	parser/expr/GreaterOrEqualOperator.cpp  \
	parser/expr/InOperator.cpp  \
	parser/expr/IsOperator.cpp  \
	parser/expr/LeftShiftOperator.cpp  \
	parser/expr/LessOperator.cpp \
	parser/expr/LessOrEqualOperator.cpp  \
	parser/expr/LikeOperator.cpp  \
	parser/expr/ListExpression.cpp  \
	parser/expr/LogicalAndOperator.cpp  \
	parser/expr/LogicalBinaryOperator.cpp  \
	parser/expr/LogicalUnaryOperator.cpp  \
	parser/expr/LogicalOrOperator.cpp  \
	parser/expr/LogicalNotOperator.cpp  \
	parser/expr/ModuloOperator.cpp  \
	parser/expr/MultiplyOperator.cpp  \
	parser/expr/NotEqualOperator.cpp  \
	parser/expr/RightShiftOperator.cpp  \
	parser/expr/SingleColumnExpression.cpp  \
	parser/expr/TernaryOperator.cpp  \
	parser/expr/SubtractOperator.cpp  \
	parser/expr/UnaryOperator.cpp  \
	parser/expr/UnaryMinusOperator.cpp  \
	parser/expr/UnaryPlusOperator.cpp  \
	\
	reg/ColumnRecord.cpp  \
	reg/ColumnDefinitionConstraintRecord.cpp  \
	reg/ColumnDefinitionRecord.cpp  \
	reg/ColumnSetColumnRecord.cpp  \
	reg/ColumnSetRecord.cpp  \
	reg/ConstraintDefinitionRecord.cpp  \
	reg/ConstraintRecord.cpp  \
	reg/DatabaseRecord.cpp  \
	reg/Helpers.cpp  \
	reg/IndexColumnRecord.cpp  \
	reg/IndexRecord.cpp  \
	reg/TableRecord.cpp  \
	reg/UserAccessKeyRecord.cpp  \
	reg/UserPermissionRecord.cpp  \
	reg/UserRecord.cpp  \
	\
	uli/FileCache.cpp  \
	uli/FileData.cpp  \
	uli/Int16UniqueLinearIndex.cpp  \
	uli/Int32UniqueLinearIndex.cpp  \
	uli/Int64UniqueLinearIndex.cpp  \
	uli/Int8UniqueLinearIndex.cpp  \
	uli/NodeCache.cpp  \
	uli/UInt16UniqueLinearIndex.cpp  \
	uli/UInt32UniqueLinearIndex.cpp  \
	uli/UInt64UniqueLinearIndex.cpp  \
	uli/UInt8UniqueLinearIndex.cpp  \
	uli/UniqueLinearIndex.cpp  \
	\
	BlockRegistry.cpp  \
	Column.cpp  \
	ColumnConstraint.cpp  \
	ColumnDataAddress.cpp  \
	ColumnDataBlock.cpp  \
	ColumnDataBlockCache.cpp  \
	ColumnDataBlockHeader.cpp  \
	ColumnDataRecord.cpp  \
	ColumnDefinition.cpp  \
	ColumnDefinitionCache.cpp  \
	ColumnDefinitionConstraint.cpp  \
	ColumnSet.cpp  \
	ColumnSetColumn.cpp  \
	ColumnSpecification.cpp  \
	Constraint.cpp  \
	ConstraintCache.cpp  \
	ConstraintDefinition.cpp  \
	ConstraintDefinitionCache.cpp  \
	DataSet.cpp  \
	DatabaseCache.cpp  \
	DatabaseMetadata.cpp  \
	Database_Common.cpp  \
	Database_Init.cpp  \
	Database_ReadObjects.cpp  \
	Database_RecordObjects.cpp  \
	Database_SysTablesIO.cpp  \
	Index.cpp  \
	IndexColumn.cpp  \
	IndexFileHeaderBase.cpp  \
	Instance.cpp  \
	LobChunkHeader.cpp  \
	MasterColumnRecord.cpp  \
	NotNullConstraint.cpp  \
	SystemDatabase.cpp  \
	Table.cpp  \
	TableCache.cpp  \
	TableColumns.cpp  \
	TableDataSet.cpp  \
	User.cpp  \
	UserAccessKey.cpp  \
	UserCache.cpp  \
	UserDatabase.cpp  \
	UserPermission.cpp

CXX_HDR:= \
	bpt/BPlusTreeIndex.h  \
	\
	crypto/GetCipher.h  \
	\
	handlers/RequestHandler.h  \
	\
	ikt/IndexKeyTraits.h  \
	ikt/Int16IndexKeyTraits.h  \
	ikt/Int32IndexKeyTraits.h  \
	ikt/Int64IndexKeyTraits.h  \
	ikt/Int8IndexKeyTraits.h  \
	ikt/UInt16IndexKeyTraits.h  \
	ikt/UInt32IndexKeyTraits.h  \
	ikt/UInt64IndexKeyTraits.h  \
	ikt/UInt8IndexKeyTraits.h  \
	\
	lob/ColumnBlobStream.h  \
	lob/ColumnClobStream.h  \
	lob/ColumnLobStream.h  \
	\
	parser/AntlrHelpers.h  \
	parser/DatabaseContext.h  \
	parser/DBEngineRequest.h  \
	parser/DBEngineRequestFactory.h  \
	parser/DBEngineRequestPtr.h  \
	parser/DBEngineRequestType.h  \
	parser/EmptyContext.h  \
	parser/SqlParser.h  \
	parser/antlr_wrappers/Antlr4RuntimeWrapper.h  \
	parser/antlr_wrappers/SiodbBaseListenerWrapper.h  \
	parser/antlr_wrappers/SiodbLexerWrapper.h  \
	parser/antlr_wrappers/SiodbListenerWrapper.h  \
	parser/antlr_wrappers/SiodbParserWrapper.h  \
	parser/antlr_wrappers/SiodbVisitorWrapper.h  \
	\
	parser/expr/AllExpressions.h  \
	parser/expr/AddOperator.h  \
	parser/expr/ArithmeticBinaryOperator.h  \
	parser/expr/ArithmeticUnaryOperator.h  \
	parser/expr/BetweenOperator.h  \
	parser/expr/BinaryOperator.h  \
	parser/expr/BitwiseAndOperator.h  \
	parser/expr/BitwiseBinaryOperator.h  \
	parser/expr/BitwiseOrOperator.h  \
	parser/expr/BitwiseUnaryOperator.h  \
	parser/expr/BitwiseXorOperator.h  \
	parser/expr/CastOperator.h  \
	parser/expr/ColumnExpressionBase.h  \
	parser/expr/ComparisonBinaryOperator.h  \
	parser/expr/ComplementOperator.h  \
	parser/expr/ConcatenationOperator.h  \
	parser/expr/ConstantExpression.h \
	parser/expr/EqualOperator.h  \
	parser/expr/Expression.h  \
	parser/expr/ExpressionFactory.h  \
	parser/expr/ExpressionType.h  \
	parser/expr/GreaterOperator.h  \
	parser/expr/GreaterOrEqualOperator.h  \
	parser/expr/InOperator.h  \
	parser/expr/IsOperator.h  \
	parser/expr/LeftShiftOperator.h  \
	parser/expr/LessOperator.h \
	parser/expr/LessOrEqualOperator.h  \
	parser/expr/LikeOperator.h  \
	parser/expr/ListExpression.h  \
	parser/expr/LogicalAndOperator.h  \
	parser/expr/LogicalBinaryOperator.h  \
	parser/expr/LogicalUnaryOperator.h  \
	parser/expr/LogicalOrOperator.h  \
	parser/expr/LogicalNotOperator.h  \
	parser/expr/ModuloOperator.h  \
	parser/expr/MultiplyOperator.h  \
	parser/expr/NotEqualOperator.h  \
	parser/expr/RightShiftOperator.h  \
	parser/expr/SingleColumnExpression.h  \
	parser/expr/SubtractOperator.h  \
	parser/expr/TernaryOperator.h  \
	parser/expr/UnaryOperator.h  \
	parser/expr/UnaryMinusOperator.h  \
	parser/expr/UnaryPlusOperator.h  \
	\
	reg/ColumnDefinitionConstraintRecord.h  \
	reg/ColumnDefinitionConstraintRegistry.h  \
	reg/ColumnDefinitionRecord.h  \
	reg/ColumnDefinitionRegistry.h  \
	reg/ColumnRecord.h  \
	reg/ColumnRegistry.h  \
	reg/ColumnSetColumnRecord.h  \
	reg/ColumnSetColumnRegistry.h  \
	reg/ColumnSetRecord.h  \
	reg/ColumnSetRegistry.h  \
	reg/ConstraintDefinitionRecord.h  \
	reg/ConstraintDefinitionRegistry.h  \
	reg/ConstraintRecord.h  \
	reg/ConstraintRegistry.h  \
	reg/DatabaseRecord.h  \
	reg/DatabaseRegistry.h  \
	reg/Helpers.h  \
	reg/IndexColumnRecord.h  \
	reg/IndexColumnRegistry.h  \
	reg/IndexRecord.h  \
	reg/IndexRegistry.h  \
	reg/TableRecord.h  \
	reg/TableRegistry.h  \
	reg/UserAccessKeyRecord.h  \
	reg/UserAccessKeyRegistry.h  \
	reg/UserPermissionRecord.h  \
	reg/UserPermissionRegistry.h  \
	reg/UserRecord.h  \
	reg/UserRegistry.h  \
	\
	uli/FileCache.h  \
	uli/FileData.h  \
	uli/FileDataPtr.h  \
	uli/Int16UniqueLinearIndex.h  \
	uli/Int32UniqueLinearIndex.h  \
	uli/Int64UniqueLinearIndex.h  \
	uli/Int8UniqueLinearIndex.h  \
	uli/Node.h  \
	uli/NodeCache.h  \
	uli/NodePtr.h  \
	uli/UInt16UniqueLinearIndex.h  \
	uli/UInt32UniqueLinearIndex.h  \
	uli/UInt64UniqueLinearIndex.h  \
	uli/UInt8UniqueLinearIndex.h  \
	uli/UniqueLinearIndex.h  \
	\
	AuthenticationResult.h  \
	BlockRegistry.h  \
	Column.h  \
	ColumnConstraint.h  \
	ColumnPtr.h  \
	ColumnDataAddress.h  \
	ColumnDataBlock.h  \
	ColumnDataBlockHeader.h  \
	ColumnDataBlockCache.h  \
	ColumnDataBlockPtr.h  \
	ColumnDataBlockState.h  \
	ColumnDataRecord.h  \
	ColumnDefinition.h  \
	ColumnDefinitionCache.h  \
	ColumnDefinitionConstraint.h  \
	ColumnDefinitionConstraintList.h  \
	ColumnDefinitionConstraintListPtr.h  \
	ColumnDefinitionConstraintPtr.h  \
	ColumnDefinitionPtr.h  \
	ColumnSet.h  \
	ColumnSetCache.h  \
	ColumnSetColumn.h  \
	ColumnSetColumnPtr.h  \
	ColumnSetPtr.h  \
	ColumnSpecification.h  \
	Constraint.h  \
	ConstraintCache.h  \
	ConstraintDefinition.h  \
	ConstraintDefinitionCache.h  \
	ConstraintDefinitionPtr.h  \
	ConstraintPtr.h  \
	DataSet.h  \
	Database.h  \
	DatabaseCache.h  \
	DatabaseError.h  \
	DatabaseMetadata.h  \
	DatabasePtr.h  \
	Debug.h  \
	DefaultValueConstraint.h  \
	FirstUserObjectId.h  \
	Index.h  \
	IndexColumn.h  \
	IndexColumnPtr.h  \
	IndexColumnSpecification.h  \
	IndexFileHeaderBase.h  \
	IndexPtr.h  \
	Instance.h  \
	InstancePtr.h  \
	LobChunkHeader.h  \
	MasterColumnRecord.h  \
	NotNullConstraint.h  \
	SessionGuard.h  \
	SimpleColumnSpecification.h  \
	SystemDatabase.h  \
	Table.h  \
	TableCache.h  \
	TableDataSet.h  \
	TablePtr.h  \
	ThrowDatabaseError.h  \
	TransactionParameters.h  \
	UpdateUserAccessKeyParameters.h  \
	UpdateUserParameters.h  \
	User.h  \
	UserAccessKey.h  \
	UserAccessKeyPtr.h  \
	UserCache.h  \
	UserDatabase.h  \
	UserPermission.h  \
	UserPtr.h


CXXFLAGS+=-I$(GENERATED_FILES_ROOT)

ANTLR_GRAMMAR:=parser/Siodb.g4

ANTLR_GENERATED_FILES_DIR:=$(THIS_GENERATED_FILES_DIR)/parser/antlr-generated

ANTLR_GENERATED_CXX_SRC:= \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbBaseListener.cpp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbLexer.cpp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbListener.cpp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbParser.cpp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbVisitor.cpp

ANTLR_GENERATED_CXX_HDR:= \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbBaseListener.h  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbLexer.h  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbListener.h  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbParser.h  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbVisitor.h

ANTLR_GENERATED_SUPPLEMENTARY_FILES:= \
	$(ANTLR_GENERATED_FILES_DIR)/Siodb.interp  \
	$(ANTLR_GENERATED_FILES_DIR)/Siodb.tokens  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbLexer.interp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbLexer.tokens

ANTLR_GENERATED_FILES:= \
	$(ANTLR_GENERATED_CXX_SRC)  \
	$(ANTLR_GENERATED_CXX_HDR)  \
	$(ANTLR_GENERATED_SUPPLEMENTARY_FILES)

MESSAGES_HDR:=$(THIS_GENERATED_FILES_DIR)/../messages/IOManagerMessageId.h

EXTRA_CXX_DEPS:= $(MESSAGES_HDR) $(ANTLR_GENERATED_CXX_HDR)

include $(MK)/Main.mk

$(MESSAGES_HDR): ../messages/iomgr.msg | $$(@D)/.
	@echo SIODBMC $@
	$(NOECHO)$(SIODBMC) -H -b int -n siodb.iomgr -e IOManagerMessageId -gp -i $< -o $@

.PRECIOUS: $(MESSAGES_HDR) $(ANTLR_GENERATED_FILES)

# ANTLR build rules
ANTLR:=antlr4
ANTLR_FLAGS:=-Dlanguage=Cpp -Xexact-output-dir -visitor

$(ANTLR_GENERATED_FILES): $(ANTLR_GRAMMAR)
	$(ANTLR) $(ANTLR_FLAGS) -o $(ANTLR_GENERATED_FILES_DIR) $<
