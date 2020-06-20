# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# IO manager library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

TARGET_LIB:=iomgr

CXX_SRC:= \
	main/IOMgrConnectionHandler.cpp  \
	main/IOMgrConnectionManager.cpp  \
	main/IOMgrMain.cpp  \
	main/IORequest.cpp  \
	main/UniversalWorker.cpp  \
	main/WorkerBase.cpp  \
	\
	dbengine/crypto/GetCipher.cpp  \
	\
	dbengine/bpt/BPlusTreeIndex.cpp  \
	\
	dbengine/handlers/RequestHandler_Common.cpp  \
	dbengine/handlers/RequestHandler_DDL.cpp  \
	dbengine/handlers/RequestHandler_DML.cpp  \
	dbengine/handlers/RequestHandler_DQL.cpp  \
	dbengine/handlers/RequestHandler_TC.cpp  \
	dbengine/handlers/RequestHandler_UM.cpp  \
	\
	dbengine/ikt/IndexKeyTraits.cpp  \
	dbengine/ikt/Int16IndexKeyTraits.cpp  \
	dbengine/ikt/Int32IndexKeyTraits.cpp  \
	dbengine/ikt/Int64IndexKeyTraits.cpp  \
	dbengine/ikt/Int8IndexKeyTraits.cpp  \
	dbengine/ikt/UInt16IndexKeyTraits.cpp  \
	dbengine/ikt/UInt32IndexKeyTraits.cpp  \
	dbengine/ikt/UInt64IndexKeyTraits.cpp  \
	dbengine/ikt/UInt8IndexKeyTraits.cpp  \
	\
	dbengine/lob/ColumnBlobStream.cpp  \
	dbengine/lob/ColumnClobStream.cpp  \
	dbengine/lob/ColumnLobStream.cpp  \
	\
	dbengine/parser/AntlrHelpers.cpp  \
	dbengine/parser/DBEngineRequest.cpp  \
	dbengine/parser/DBEngineRequestFactory.cpp  \
	dbengine/parser/DatabaseContext.cpp  \
	dbengine/parser/EmptyContext.cpp  \
	dbengine/parser/SqlParser.cpp  \
	dbengine/parser/antlr_wrappers/SiodbBaseListenerWrapper.cpp  \
	dbengine/parser/antlr_wrappers/SiodbLexerWrapper.cpp  \
	dbengine/parser/antlr_wrappers/SiodbListenerWrapper.cpp  \
	dbengine/parser/antlr_wrappers/SiodbParserWrapper.cpp  \
	dbengine/parser/antlr_wrappers/SiodbVisitorWrapper.cpp  \
	\
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
	dbengine/parser/expr/ExpressionFactory.cpp  \
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
	dbengine/parser/expr/UnaryPlusOperator.cpp  \
	\
	dbengine/reg/ColumnRecord.cpp  \
	dbengine/reg/ColumnDefinitionConstraintRecord.cpp  \
	dbengine/reg/ColumnDefinitionRecord.cpp  \
	dbengine/reg/ColumnSetColumnRecord.cpp  \
	dbengine/reg/ColumnSetRecord.cpp  \
	dbengine/reg/ConstraintDefinitionRecord.cpp  \
	dbengine/reg/ConstraintRecord.cpp  \
	dbengine/reg/DatabaseRecord.cpp  \
	dbengine/reg/Helpers.cpp  \
	dbengine/reg/IndexColumnRecord.cpp  \
	dbengine/reg/IndexRecord.cpp  \
	dbengine/reg/TableRecord.cpp  \
	dbengine/reg/UserAccessKeyRecord.cpp  \
	dbengine/reg/UserPermissionRecord.cpp  \
	dbengine/reg/UserRecord.cpp  \
	\
	dbengine/uli/FileCache.cpp  \
	dbengine/uli/FileData.cpp  \
	dbengine/uli/Int16UniqueLinearIndex.cpp  \
	dbengine/uli/Int32UniqueLinearIndex.cpp  \
	dbengine/uli/Int64UniqueLinearIndex.cpp  \
	dbengine/uli/Int8UniqueLinearIndex.cpp  \
	dbengine/uli/NodeCache.cpp  \
	dbengine/uli/UInt16UniqueLinearIndex.cpp  \
	dbengine/uli/UInt32UniqueLinearIndex.cpp  \
	dbengine/uli/UInt64UniqueLinearIndex.cpp  \
	dbengine/uli/UInt8UniqueLinearIndex.cpp  \
	dbengine/uli/UniqueLinearIndex.cpp  \
	\
	dbengine/BlockRegistry.cpp  \
	dbengine/Column.cpp  \
	dbengine/ColumnConstraint.cpp  \
	dbengine/ColumnDataAddress.cpp  \
	dbengine/ColumnDataBlock.cpp  \
	dbengine/ColumnDataBlockCache.cpp  \
	dbengine/ColumnDataBlockHeader.cpp  \
	dbengine/ColumnDataRecord.cpp  \
	dbengine/ColumnDefinition.cpp  \
	dbengine/ColumnDefinitionCache.cpp  \
	dbengine/ColumnDefinitionConstraint.cpp  \
	dbengine/ColumnSet.cpp  \
	dbengine/ColumnSetColumn.cpp  \
	dbengine/ColumnSpecification.cpp  \
	dbengine/Constraint.cpp  \
	dbengine/ConstraintCache.cpp  \
	dbengine/ConstraintDefinition.cpp  \
	dbengine/ConstraintDefinitionCache.cpp  \
	dbengine/DataSet.cpp  \
	dbengine/DatabaseCache.cpp  \
	dbengine/DatabaseMetadata.cpp  \
	dbengine/Database_Common.cpp  \
	dbengine/Database_Init.cpp  \
	dbengine/Database_ReadObjects.cpp  \
	dbengine/Database_RecordObjects.cpp  \
	dbengine/Database_SysTablesIO.cpp  \
	dbengine/Index.cpp  \
	dbengine/IndexColumn.cpp  \
	dbengine/IndexFileHeaderBase.cpp  \
	dbengine/Instance.cpp  \
	dbengine/LobChunkHeader.cpp  \
	dbengine/MasterColumnRecord.cpp  \
	dbengine/NotNullConstraint.cpp  \
	dbengine/SystemDatabase.cpp  \
	dbengine/Table.cpp  \
	dbengine/TableCache.cpp  \
	dbengine/TableColumns.cpp  \
	dbengine/TableDataSet.cpp  \
	dbengine/User.cpp  \
	dbengine/UserAccessKey.cpp  \
	dbengine/UserCache.cpp  \
	dbengine/UserDatabase.cpp  \
	dbengine/UserPermission.cpp

CXX_HDR:= \
	main/ClientSession.h  \
	main/IOMgrConnectionHandler.h  \
	main/IOMgrConnectionManager.h  \
	main/IORequest.h  \
	main/UniversalWorker.h  \
	main/WorkerBase.h  \
	\
	dbengine/bpt/BPlusTreeIndex.h  \
	\
	dbengine/crypto/GetCipher.h  \
	\
	dbengine/handlers/RequestHandler.h  \
	\
	dbengine/ikt/IndexKeyTraits.h  \
	dbengine/ikt/Int16IndexKeyTraits.h  \
	dbengine/ikt/Int32IndexKeyTraits.h  \
	dbengine/ikt/Int64IndexKeyTraits.h  \
	dbengine/ikt/Int8IndexKeyTraits.h  \
	dbengine/ikt/UInt16IndexKeyTraits.h  \
	dbengine/ikt/UInt32IndexKeyTraits.h  \
	dbengine/ikt/UInt64IndexKeyTraits.h  \
	dbengine/ikt/UInt8IndexKeyTraits.h  \
	\
	dbengine/lob/ColumnBlobStream.h  \
	dbengine/lob/ColumnClobStream.h  \
	dbengine/lob/ColumnLobStream.h  \
	\
	dbengine/parser/AntlrHelpers.h  \
	dbengine/parser/DatabaseContext.h  \
	dbengine/parser/DBEngineRequest.h  \
	dbengine/parser/DBEngineRequestFactory.h  \
	dbengine/parser/DBEngineRequestType.h  \
	dbengine/parser/EmptyContext.h  \
	dbengine/parser/SqlParser.h  \
	dbengine/parser/antlr_wrappers/Antlr4RuntimeWrapper.h  \
	dbengine/parser/antlr_wrappers/SiodbBaseListenerWrapper.h  \
	dbengine/parser/antlr_wrappers/SiodbLexerWrapper.h  \
	dbengine/parser/antlr_wrappers/SiodbListenerWrapper.h  \
	dbengine/parser/antlr_wrappers/SiodbParserWrapper.h  \
	dbengine/parser/antlr_wrappers/SiodbVisitorWrapper.h  \
	\
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
	dbengine/parser/expr/ExpressionFactory.h  \
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
	dbengine/parser/expr/UnaryPlusOperator.h  \
	\
	dbengine/reg/ColumnDefinitionConstraintRecord.h  \
	dbengine/reg/ColumnDefinitionConstraintRegistry.h  \
	dbengine/reg/ColumnDefinitionRecord.h  \
	dbengine/reg/ColumnDefinitionRegistry.h  \
	dbengine/reg/ColumnRecord.h  \
	dbengine/reg/ColumnRegistry.h  \
	dbengine/reg/ColumnSetColumnRecord.h  \
	dbengine/reg/ColumnSetColumnRegistry.h  \
	dbengine/reg/ColumnSetRecord.h  \
	dbengine/reg/ColumnSetRegistry.h  \
	dbengine/reg/ConstraintDefinitionRecord.h  \
	dbengine/reg/ConstraintDefinitionRegistry.h  \
	dbengine/reg/ConstraintRecord.h  \
	dbengine/reg/ConstraintRegistry.h  \
	dbengine/reg/DatabaseRecord.h  \
	dbengine/reg/DatabaseRegistry.h  \
	dbengine/reg/Helpers.h  \
	dbengine/reg/IndexColumnRecord.h  \
	dbengine/reg/IndexColumnRegistry.h  \
	dbengine/reg/IndexRecord.h  \
	dbengine/reg/IndexRegistry.h  \
	dbengine/reg/TableRecord.h  \
	dbengine/reg/TableRegistry.h  \
	dbengine/reg/UserAccessKeyRecord.h  \
	dbengine/reg/UserAccessKeyRegistry.h  \
	dbengine/reg/UserPermissionRecord.h  \
	dbengine/reg/UserPermissionRegistry.h  \
	dbengine/reg/UserRecord.h  \
	dbengine/reg/UserRegistry.h  \
	\
	dbengine/uli/FileCache.h  \
	dbengine/uli/FileData.h  \
	dbengine/uli/FileDataPtr.h  \
	dbengine/uli/Int16UniqueLinearIndex.h  \
	dbengine/uli/Int32UniqueLinearIndex.h  \
	dbengine/uli/Int64UniqueLinearIndex.h  \
	dbengine/uli/Int8UniqueLinearIndex.h  \
	dbengine/uli/Node.h  \
	dbengine/uli/NodeCache.h  \
	dbengine/uli/NodePtr.h  \
	dbengine/uli/UInt16UniqueLinearIndex.h  \
	dbengine/uli/UInt32UniqueLinearIndex.h  \
	dbengine/uli/UInt64UniqueLinearIndex.h  \
	dbengine/uli/UInt8UniqueLinearIndex.h  \
	dbengine/uli/UniqueLinearIndex.h  \
	\
	dbengine/BlockRegistry.h  \
	dbengine/Column.h  \
	dbengine/ColumnConstraint.h  \
	dbengine/ColumnPtr.h  \
	dbengine/ColumnDataAddress.h  \
	dbengine/ColumnDataBlock.h  \
	dbengine/ColumnDataBlockHeader.h  \
	dbengine/ColumnDataBlockCache.h  \
	dbengine/ColumnDataBlockPtr.h  \
	dbengine/ColumnDataBlockState.h  \
	dbengine/ColumnDataRecord.h  \
	dbengine/ColumnDefinition.h  \
	dbengine/ColumnDefinitionCache.h  \
	dbengine/ColumnDefinitionConstraint.h  \
	dbengine/ColumnDefinitionConstraintList.h  \
	dbengine/ColumnDefinitionConstraintListPtr.h  \
	dbengine/ColumnDefinitionConstraintPtr.h  \
	dbengine/ColumnDefinitionPtr.h  \
	dbengine/ColumnSet.h  \
	dbengine/ColumnSetCache.h  \
	dbengine/ColumnSetColumn.h  \
	dbengine/ColumnSetColumnPtr.h  \
	dbengine/ColumnSetPtr.h  \
	dbengine/ColumnSpecification.h  \
	dbengine/Constraint.h  \
	dbengine/ConstraintCache.h  \
	dbengine/ConstraintDefinition.h  \
	dbengine/ConstraintDefinitionCache.h  \
	dbengine/ConstraintDefinitionPtr.h  \
	dbengine/ConstraintPtr.h  \
	dbengine/DataSet.h  \
	dbengine/Database.h  \
	dbengine/DatabaseCache.h  \
	dbengine/DatabaseError.h  \
	dbengine/DatabaseMetadata.h  \
	dbengine/DatabasePtr.h  \
	dbengine/Debug.h  \
	dbengine/DefaultValueConstraint.h  \
	dbengine/FirstUserObjectId.h  \
	dbengine/Index.h  \
	dbengine/IndexColumn.h  \
	dbengine/IndexColumnPtr.h  \
	dbengine/IndexColumnSpecification.h  \
	dbengine/IndexFileHeaderBase.h  \
	dbengine/IndexPtr.h  \
	dbengine/Instance.h  \
	dbengine/InstancePtr.h  \
	dbengine/LobChunkHeader.h  \
	dbengine/MasterColumnRecord.h  \
	dbengine/NotNullConstraint.h  \
	dbengine/SessionGuard.h  \
	dbengine/SimpleColumnSpecification.h  \
	dbengine/SystemDatabase.h  \
	dbengine/Table.h  \
	dbengine/TableCache.h  \
	dbengine/TableDataSet.h  \
	dbengine/TablePtr.h  \
	dbengine/ThrowDatabaseError.h  \
	dbengine/TransactionParameters.h  \
	dbengine/UpdateUserAccessKeyParameters.h  \
	dbengine/UpdateUserParameters.h  \
	dbengine/User.h  \
	dbengine/UserAccessKey.h  \
	dbengine/UserAccessKeyPtr.h  \
	dbengine/UserCache.h  \
	dbengine/UserDatabase.h  \
	dbengine/UserPermission.h  \
	dbengine/UserPtr.h


CXXFLAGS+=-I$(GENERATED_FILES_ROOT)

ANTLR_GRAMMAR:=dbengine/parser/Siodb.g4

ANTLR_GENERATED_FILES_DIR:=$(THIS_GENERATED_FILES_DIR)/dbengine/parser/antlr-generated

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

MESSAGES_HDR:=$(THIS_GENERATED_FILES_DIR)messages/IOManagerMessageId.h

EXTRA_CXX_DEPS:= $(MESSAGES_HDR) $(ANTLR_GENERATED_CXX_HDR)

CXX_FLAGS+=$(THIS_GENERATED_FILES_DIR)

include $(MK)/Main.mk

$(MESSAGES_HDR): messages/iomgr.msg | $$(@D)/.
	@echo SIODBMC $@
	@echo EXTRA_DEPS $(EXTRA_CXX_DEPS)
	$(NOECHO)$(SIODBMC) -H -b int -n siodb.iomgr -e IOManagerMessageId -gp -i $< -o $@

.PRECIOUS: $(MESSAGES_HDR) $(ANTLR_GENERATED_FILES)

# ANTLR build rules
ANTLR:=antlr4
ANTLR_FLAGS:=-Dlanguage=Cpp -Xexact-output-dir -visitor

$(ANTLR_GENERATED_FILES): $(ANTLR_GRAMMAR)
	$(ANTLR) $(ANTLR_FLAGS) -o $(ANTLR_GENERATED_FILES_DIR) $<
