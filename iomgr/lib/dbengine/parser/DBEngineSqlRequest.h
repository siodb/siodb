// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "DBEngineRequest.h"
#include "expr/Expression.h"
#include "../UpdateDatabaseParameters.h"
#include "../UpdateUserAccessKeyParameters.h"
#include "../UpdateUserParameters.h"
#include "../UpdateUserTokenParameters.h"

// Common project headers
#include <siodb/common/proto/ColumnDataType.pb.h>
#include <siodb/common/utils/Uuid.h>
#include <siodb/iomgr/shared/dbengine/ConstraintType.h>

namespace siodb::iomgr::dbengine::requests {

/** Join type for tables */
enum class TableJoinType {
    kInnerJoin,
    kLeftJoin,
    kRightJoin,
    kFullJoin,
};

/** Source table specification */
struct SourceTable {
    /**
     * Initializes object of class SourceTable.
     * @param name Table name.
     * @param alias Table alias.
     * @param joinType Table join type.
     */
    SourceTable(std::string&& name, std::string&& alias,
            TableJoinType joinType = TableJoinType::kInnerJoin) noexcept
        : m_name(std::move(name))
        , m_alias(std::move(alias))
        , m_joinType(joinType)
    {
    }

    /** Table name */
    const std::string m_name;

    /** Table alias, may be empty. */
    const std::string m_alias;

    /** Join type */
    const TableJoinType m_joinType;
};

/** Column reference */
struct ColumnReference {
    /**
     * Initializes object of class ColumnReference.
     * @param table Table name.
     * @param column Column name.
     */
    ColumnReference(std::string&& table, std::string&& column) noexcept
        : m_table(std::move(table))
        , m_column(std::move(column))
    {
    }

    /** Table name, may be empty */
    const std::string m_table;

    /** Column name */
    const std::string m_column;
};

/** Resulting expression specification */
struct ResultExpression {
    /**
     * Initializes object of class ResultExpression.
     * @param table Table name.
     * @param expression Result column expression.
     * @param alias Column alias.
     */
    ResultExpression(ConstExpressionPtr&& expression, std::string&& alias) noexcept
        : m_expression(std::move(expression))
        , m_alias(std::move(alias))
    {
    }

    /** Result expression */
    ConstExpressionPtr m_expression;

    /** Column alias, may be empty */
    std::string m_alias;
};

/** Element of the ORDER BY clause */
struct OrderByExpression {
    /**
     * Initializes object of class OrderByElement.
     * @param subject ORDER BY subject
     * @param sortDescending Indicator of the descending sort order.
     */
    OrderByExpression(ConstExpressionPtr&& subject, bool sortDescending) noexcept
        : m_subject(std::move(subject))
        , m_sortDescending(sortDescending)
    {
    }

    /** ORDER BY subject */
    const ConstExpressionPtr m_subject;

    /** Indicator of the descending sort order */
    const bool m_sortDescending;
};

/** SELECT request */
struct SelectRequest : public DBEngineRequest {
    /**
     * Initializes object of class SelectRequest.
     * @param database Database name.
     * @param table List of tables.
     * @param where WHERE condition.
     * @param offset Offset value in the LIMIT clause.
     * @param limit Limit value in the LIMIT clause.
     * @param groupBy GROUP BY clause.
     * @param having HAVING condition.
     * @param orderBy ORDER BY clause.
     */
    SelectRequest(std::string&& database, std::vector<SourceTable>&& tables,
            std::vector<ResultExpression>&& columns, ConstExpressionPtr&& where = nullptr,
            std::vector<ConstExpressionPtr>&& groupBy = std::vector<ConstExpressionPtr>(),
            ConstExpressionPtr&& having = nullptr,
            std::vector<ConstExpressionPtr>&& orderBy = std::vector<ConstExpressionPtr>(),
            ConstExpressionPtr&& offset = nullptr, ConstExpressionPtr&& limit = nullptr) noexcept
        : DBEngineRequest(DBEngineRequestType::kSelect)
        , m_database(std::move(database))
        , m_tables(std::move(tables))
        , m_resultExpressions(std::move(columns))
        , m_where(std::move(where))
        , m_groupBy(std::move(groupBy))
        , m_having(std::move(having))
        , m_orderBy(std::move(orderBy))
        , m_offset(std::move(offset))
        , m_limit(std::move(limit))
    {
    }

    /** Database name */
    const std::string m_database;

    /** List of tables */
    const std::vector<SourceTable> m_tables;

    /* List of resulting columns */
    const std::vector<ResultExpression> m_resultExpressions;

    /** WHERE condition, empty if absent */
    const ConstExpressionPtr m_where;

    /** GROUP BY expressions, empty if absent */
    const std::vector<ConstExpressionPtr> m_groupBy;

    /** HAVING condition, empty if absent */
    const ConstExpressionPtr m_having;

    /** ORDER BY expressions, empty if absent */
    const std::vector<ConstExpressionPtr> m_orderBy;

    /** OFFSET expression, empty if absent */
    const ConstExpressionPtr m_offset;

    /** LIMIT expression, empty if absent */
    const ConstExpressionPtr m_limit;
};

/** INSERT request */
struct InsertRequest : public DBEngineRequest {
    /**
     * Initializes object of class InsertRequest.
     * @param database Database name.
     * @param table Table name.
     * @param columns Column names.
     * @param values Column values.
     */
    InsertRequest(std::string&& database, std::string&& table, std::vector<std::string>&& columns,
            std::vector<std::vector<ConstExpressionPtr>>&& values) noexcept
        : DBEngineRequest(DBEngineRequestType::kInsert)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_columns(std::move(columns))
        , m_values(std::move(values))
    {
    }

    /**
     * Initializes object of class InsertRequest.
     * @param database Database name.
     * @param table Table name.
     * @param values Column values.
     */
    InsertRequest(std::string&& database, std::string&& table,
            std::vector<std::vector<ConstExpressionPtr>>&& values) noexcept
        : DBEngineRequest(DBEngineRequestType::kInsert)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_values(std::move(values))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** Column names, may be empty. */
    const std::vector<std::string> m_columns;

    /** Column values */
    const std::vector<std::vector<ConstExpressionPtr>> m_values;
};

/** UPDATE request */
struct UpdateRequest : public DBEngineRequest {
    /**
     * Initializes object of class UpdateRequest.
     * @param database Database name.
     * @param table Table name.
     * @param columns List of columns.
     * @param values List of values.
     * @param where WHERE condition.
     */
    UpdateRequest(std::string&& database, SourceTable&& table,
            std::vector<ColumnReference>&& columns, std::vector<ConstExpressionPtr>&& values,
            ConstExpressionPtr&& where = nullptr) noexcept
        : DBEngineRequest(DBEngineRequestType::kUpdate)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_columns(std::move(columns))
        , m_values(std::move(values))
        , m_where(std::move(where))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const SourceTable m_table;

    /** List of columns */
    const std::vector<ColumnReference> m_columns;

    /** Column values */
    const std::vector<ConstExpressionPtr> m_values;

    /** WHERE condition, nullptr if absent */
    const ConstExpressionPtr m_where;
};

/** DELETE request */
struct DeleteRequest : public DBEngineRequest {
    /**
     * Initializes object of class DeleteRequest.
     * @param database Database name.
     * @param table Table name.
     * @param columns List of columns.
     * @param values List of values.
     * @param where WHERE condition.
     */
    DeleteRequest(std::string&& database, SourceTable&& table,
            ConstExpressionPtr&& where = nullptr) noexcept
        : DBEngineRequest(DBEngineRequestType::kDelete)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_where(std::move(where))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const SourceTable m_table;

    /** WHERE condition */
    const ConstExpressionPtr m_where;
};

/** Transaction type */
enum class TransactionType {
    kDeferred,
    kImmediate,
    kExclusive,
};

/** BEGIN TRANSACTION request */
struct BeginTransactionRequest : public DBEngineRequest {
    /**
     * Initializes object of class BeginTransactionRequest.
     * @param type Transaction type.
     * @param transaction Transaction name.
     */
    BeginTransactionRequest(TransactionType type, std::string&& transaction) noexcept
        : DBEngineRequest(DBEngineRequestType::kBeginTransaction)
        , m_type(type)
        , m_transaction(std::move(transaction))
    {
    }

    /** Transaction type */
    const TransactionType m_type;

    /** Transaction name, may be empty. */
    const std::string m_transaction;
};

/** COMMIT TRANSACTION request */
struct CommitTransactionRequest : public DBEngineRequest {
    /**
     * Initializes object of class CommitTransactionRequest.
     * @param transaction Transaction name.
     */
    CommitTransactionRequest(std::string&& transaction) noexcept
        : DBEngineRequest(DBEngineRequestType::kCommitTransaction)
        , m_transaction(std::move(transaction))
    {
    }

    /** Transaction name, may be empty. */
    const std::string m_transaction;
};

/** ROLLBACK TRANSACTION request */
struct RollbackTransactionRequest : public DBEngineRequest {
    /**
     * Initializes object of class RollbackTransactionRequest.
     * @param transaction Transaction name.
     * @param savepoint Savepoint name.
     */
    RollbackTransactionRequest(std::string&& transaction, std::string&& savepoint) noexcept
        : DBEngineRequest(DBEngineRequestType::kRollbackTransaction)
        , m_transaction(std::move(transaction))
        , m_savepoint(std::move(savepoint))
    {
    }

    /** Transaction name, may be empty. */
    const std::string m_transaction;

    /** Savepoint name, may be empty. */
    const std::string m_savepoint;
};

/** SAVEPOINT request */
struct SavepointRequest : public DBEngineRequest {
    /**
     * Initializes object of class SavepointRequest.
     * @param savepoint Savepoint name.
     */
    SavepointRequest(std::string&& savepoint) noexcept
        : DBEngineRequest(DBEngineRequestType::kSavepoint)
        , m_savepoint(std::move(savepoint))
    {
    }

    /** Savepoint name */
    const std::string m_savepoint;
};

/** RELEASE request */
struct ReleaseRequest : public DBEngineRequest {
    /**
     * Initializes object of class ReleaseRequest.
     * @param savepoint Savepoint name.
     */
    ReleaseRequest(std::string&& savepoint) noexcept
        : DBEngineRequest(DBEngineRequestType::kRelease)
        , m_savepoint(std::move(savepoint))
    {
    }

    /** Savepoint name */
    const std::string m_savepoint;
};

/** ATTACH DATABASE request */
struct AttachDatabaseRequest : public DBEngineRequest {
    /**
     * Initializes object of class AttachDatabaseRequest.
     * @param databaseUuid Database UUID.
     * @param database Database name.
     */
    AttachDatabaseRequest(const Uuid& databaseUuid, std::string&& database) noexcept
        : DBEngineRequest(DBEngineRequestType::kAttachDatabase)
        , m_databaseUuid(databaseUuid)
        , m_database(std::move(database))
    {
    }

    /** Database UUID */
    const Uuid m_databaseUuid;

    /** Database name */
    const std::string m_database;
};

/** DETACH DATABASE request */
struct DetachDatabaseRequest : public DBEngineRequest {
    /**
     * Initializes object of class DetachDatabaseRequest.
     * @param database Database name.
     * @param ifExists Indicates that operation should not fail if database doesn't exist.
     */
    explicit DetachDatabaseRequest(std::string&& database, bool ifExists = false) noexcept
        : DBEngineRequest(DBEngineRequestType::kDetachDatabase)
        , m_database(std::move(database))
        , m_ifExists(ifExists)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Indicates that operation should not fail if database doesn't exist */
    const bool m_ifExists;
};

/** CREATE DATABASE request */
struct CreateDatabaseRequest : public DBEngineRequest {
    /**
     * Initializes object of class CreateDatabaseRequest.
     * @param database Database name.
     * @param isTemporary Indicates that this is temporary database.
     * @param cipherId Cipher identifier.
     * @param cipherKeySeed Cipher key seed.
     * @param maxTableCount Maximum table count.
     */
    CreateDatabaseRequest(std::string&& database, bool isTemporary, ConstExpressionPtr&& cipherId,
            ConstExpressionPtr&& cipherKeySeed, std::uint32_t maxTableCount) noexcept
        : DBEngineRequest(DBEngineRequestType::kCreateDatabase)
        , m_database(database)
        , m_isTemporary(isTemporary)
        , m_cipherId(std::move(cipherId))
        , m_cipherKeySeed(std::move(cipherKeySeed))
        , m_maxTableCount(maxTableCount)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Indicates that this is temporary database. */
    const bool m_isTemporary;

    /** Cipher id */
    const ConstExpressionPtr m_cipherId;

    /** Cipher key seed */
    const ConstExpressionPtr m_cipherKeySeed;

    /** Maximum number of tables */
    const std::uint32_t m_maxTableCount;
};

/** DROP DATABASE request */
struct DropDatabaseRequest : public DBEngineRequest {
    /**
     * Initializes object of class DropDatabaseRequest.
     * @param database Database name.
     * @param ifExists Indicates that operation should not fail if database doesn't exist.
     */
    explicit DropDatabaseRequest(std::string&& database, bool ifExists = false) noexcept
        : DBEngineRequest(DBEngineRequestType::kDropDatabase)
        , m_database(std::move(database))
        , m_ifExists(ifExists)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Indicates that operation should not fail if database doesn't exist */
    const bool m_ifExists;
};

/** ALTER DATABASE RENAME TO request */
struct RenameDatabaseRequest : public DBEngineRequest {
    /**
     * Initializes object of class RenameDatabaseRequest.
     * @param database Database name.
     * @param newDatabase New database name.
     * @param ifExists Indicates that operation should not fail if database doesn't exist.
     */
    RenameDatabaseRequest(std::string&& database, std::string&& newDatabase, bool ifExists) noexcept
        : DBEngineRequest(DBEngineRequestType::kRenameDatabase)
        , m_database(std::move(database))
        , m_newDatabase(std::move(newDatabase))
        , m_ifExists(ifExists)
    {
    }

    /** Database name */
    const std::string m_database;

    /** New database name */
    const std::string m_newDatabase;

    /** Indicates that operation should not fail if database doesn't exist */
    const bool m_ifExists;
};

/** ALTER DATABASE SET attributes request */
struct SetDatabaseAttributesRequest : public DBEngineRequest {
    /**
     * Initializes object of class SetDatabaseAttributesRequest.
     * @param database Database name.
     * @param description New description.
     */
    SetDatabaseAttributesRequest(std::string&& database,
            std::optional<std::optional<std::string>>&& description) noexcept
        : DBEngineRequest(DBEngineRequestType::kSetDatabaseAttributes)
        , m_database(std::move(database))
        , m_params(std::move(description))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Update parameters */
    const UpdateDatabaseParameters m_params;
};

/** USE DATABASE request */
struct UseDatabaseRequest : public DBEngineRequest {
    /**
     * Initializes object of class UseDatabaseRequest.
     * @param database Database name.
     */
    explicit UseDatabaseRequest(std::string&& database) noexcept
        : DBEngineRequest(DBEngineRequestType::kUseDatabase)
        , m_database(std::move(database))
    {
    }

    /** Database name */
    const std::string m_database;
};

/** Base class for all constraints */
struct Constraint {
protected:
    /**
     * Initialized object of class Constraint.
     * @param type Constraint type.
     * @param name Constraint name.
     */
    explicit Constraint(ConstraintType type, std::string&& name) noexcept
        : m_type(type)
        , m_name(std::move(name))
    {
    }

public:
    /** De-initializes object of class Constraint. */
    virtual ~Constraint() = default;

    /**
     * Checks that given constraint type is table-only constraint.
     * @param constraintType A constraint type.
     * @return true if given constraint type is column-only constraint, false otherwise.
     */
    static bool isTableOnlyConstraint(ConstraintType constraintType) noexcept
    {
        return s_tableOnlyConstraintTypes.count(constraintType) > 0;
    }

    /**
     * Checks that given constraint type is column-only constraint.
     * @param constraintType A constraint type.
     * @return true if given constraint type is table-only constraint, false otherwise.
     */
    static bool isColumnOnlyConstraint(ConstraintType constraintType) noexcept
    {
        return s_columnOnlyConstraintTypes.count(constraintType) > 0;
    }

    /** Constraint type */
    const ConstraintType m_type;

    /** Constraint name (may be empty) */
    const std::string m_name;

private:
    /** Table-only constraint types */
    static const std::unordered_set<ConstraintType> s_tableOnlyConstraintTypes;

    /** Column-only constraint types */
    static const std::unordered_set<ConstraintType> s_columnOnlyConstraintTypes;
};

/** NULL and NOT NULL constraint */
struct NotNullConstraint : public Constraint {
    /**
     * Initializes object of class NotNullConstraint.
     * @param name Constraint name.
     * @param notNull Indicates that NULL value is not allowed.
     */
    explicit NotNullConstraint(std::string&& name, bool notNull = false) noexcept
        : Constraint(ConstraintType::kNotNull, std::move(name))
        , m_notNull(notNull)
    {
    }

    /** Indicates that NULL value id not allowed */
    const bool m_notNull;
};

/** DEFAULT value constraint */
struct DefaultValueConstraint : public Constraint {
    /**
     * Initializes object of class DefaultValueConstraint.
     * @param name Constraint name.
     * @param expression Default value expression.
     */
    explicit DefaultValueConstraint(std::string&& name, ConstExpressionPtr&& expression) noexcept
        : Constraint(ConstraintType::kDefaultValue, std::move(name))
        , m_value(std::move(expression))
    {
    }

    /** Constant value */
    const ConstExpressionPtr m_value;
};

/** UNIQUE value constraint */
struct UniqueConstraint : public Constraint {
    /**
     * Initializes object of class UniqueConstraint.
     * @param name Constraint name.
     * @param columns List of columns.
     */
    explicit UniqueConstraint(std::string&& name, std::vector<std::string>&& columns) noexcept
        : Constraint(columns.size() > 1 ? ConstraintType::kMultiColumnUnique
                                        : ConstraintType::kSingleColumnUnique,
                std::move(name))
        , m_columns(std::move(columns))
    {
    }

    /** List of columns */
    const std::vector<std::string> m_columns;
};

/** REFERENCES constraint */
struct ReferencesConstraint : public Constraint {
    /**
     * Initializes object of class ReferencesConstraint.
     * @param name Constraint name.
     * @param targetTable Target table.
     * @param targetTableColumn Target table column.
     */
    ReferencesConstraint(
            std::string&& name, std::string&& targetTable, std::string&& targetTableColumn) noexcept
        : Constraint(ConstraintType::kReferences, std::move(name))
        , m_targetTable(std::move(targetTable))
        , m_targetTableColumn(std::move(targetTableColumn))
    {
    }

    /** Target table */
    const std::string m_targetTable;

    /** Target table column */
    const std::string m_targetTableColumn;
};

/** FOREIGN KEY constraint */
struct ForeignKeyConstraint : public Constraint {
    /**
     * Initializes object of class ForeignKeyConstraint.
     * @param name Constraint name.
     * @param thisTableColumns This table columns.
     * @param targetTable Target table.
     * @param targetTableColumns Target table columns.
     */
    ForeignKeyConstraint(std::string&& name, std::vector<std::string>&& thisTableColumns,
            std::string&& targetTable, std::vector<std::string>&& targetTableColumns) noexcept
        : Constraint(ConstraintType::kForeignKey, std::move(name))
        , m_thisTableColumns(std::move(thisTableColumns))
        , m_targetTable(std::move(targetTable))
        , m_targetTableColumns(std::move(targetTableColumns))
    {
    }

    /** This table columns */
    const std::vector<std::string> m_thisTableColumns;

    /** Target table */
    const std::string m_targetTable;

    /** Target table columns */
    const std::vector<std::string> m_targetTableColumns;
};

/** CHECK constraint */
struct CheckConstraint : public Constraint {
    /**
     * Initializes object of class CheckConstraint.
     * @param name Constraint name.
     * @param expression Expression to be checked.
     */
    explicit CheckConstraint(std::string&& name, ExpressionPtr&& expression) noexcept
        : Constraint(ConstraintType::kCheck, std::move(name))
        , m_expression(std::move(expression))
    {
    }

    /** Expression to be checked */
    const ExpressionPtr m_expression;
};

/** Collation types */
enum class CollationType {
    kBinary,
    kRTrim,
    kNoCase,
};

/** COLLATE constraint */
struct CollateConstraint : public Constraint {
    /**
     * Initializes object of class CollateConstraint.
     * @param name Constraint name.
     * @param collation Collation type.
     */
    explicit CollateConstraint(std::string&& name, CollationType collation) noexcept
        : Constraint(ConstraintType::kCollate, std::move(name))
        , m_collation(collation)
    {
    }

    /** Collation type */
    const CollationType m_collation;
};

/** Table column definition */
struct ColumnDefinition {
    /**
     * Initializes object of class ColumnDefinition.
     * @param name Column name.
     * @param dataType Column data type.
     * @param dataBlockDataAreaSize Data block data area size.
     * @param constraints List of constraitns.
     */
    ColumnDefinition(std::string&& name, ColumnDataType dataType,
            std::uint32_t dataBlockDataAreaSize,
            std::vector<std::unique_ptr<Constraint>>&& constraints) noexcept
        : m_name(std::move(name))
        , m_dataType(dataType)
        , m_dataBlockDataAreaSize(dataBlockDataAreaSize)
        , m_constraints(std::move(constraints))
    {
    }

    /** Disable copy construction */
    ColumnDefinition(const ColumnDefinition&) = delete;

    /** Default move construction */
    ColumnDefinition(ColumnDefinition&&) = default;

    /** Column name */
    std::string m_name;

    /** Column data type */
    ColumnDataType m_dataType;

    /** Data block data area size */
    std::uint32_t m_dataBlockDataAreaSize;

    /** Column constraints */
    std::vector<std::unique_ptr<Constraint>> m_constraints;
};

/** CREATE TABLE request */
struct CreateTableRequest : public DBEngineRequest {
    /**
     * Initializes object of class CreateTableRequest.
     * @param database Database name.
     * @param table Table name.
     * @param columns Table columns.
     */
    CreateTableRequest(std::string&& database, std::string&& table,
            std::vector<ColumnDefinition>&& columns) noexcept
        : DBEngineRequest(DBEngineRequestType::kCreateTable)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_columns(std::move(columns))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** Column definitions */
    const std::vector<ColumnDefinition> m_columns;
};

/** DROP TABLE request */
struct DropTableRequest : public DBEngineRequest {
    /**
     * Initializes object of class DropTableRequest.
     * @param database Database name.
     * @param table Table name.
     * @param ifExists Indicates that operation should not fail if table doesn't exist.
     */
    DropTableRequest(std::string&& database, std::string&& table, bool ifExists = false) noexcept
        : DBEngineRequest(DBEngineRequestType::kDropTable)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_ifExists(ifExists)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** Indicates that operation should not fail if table doesn't exist */
    const bool m_ifExists;
};

/** RENAME TABLE request */
struct RenameTableRequest : public DBEngineRequest {
    /**
     * Initializes object of class RenameTableRequest.
     * @param database Database name.
     * @param table Table name.
     * @param ifExists Indicates that operation should not fail if table doesn't exist.
     */
    RenameTableRequest(std::string&& database, std::string&& oldTable, std::string&& newTable,
            bool ifExists = false) noexcept
        : DBEngineRequest(DBEngineRequestType::kRenameTable)
        , m_database(std::move(database))
        , m_oldTable(std::move(oldTable))
        , m_newTable(std::move(newTable))
        , m_ifExists(ifExists)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Old table name */
    const std::string m_oldTable;

    /** New table name */
    const std::string m_newTable;

    /** Indicates that operation should not fail if table doesn't exist */
    const bool m_ifExists;
};

/** ALTER TABLE SET attributes request */
struct SetTableAttributesRequest : public DBEngineRequest {
    /**
     * Initializes object of class SetTableAttributesRequest.
     * @param database Database name.
     * @param table Table name.
     * @param nextTrid Next TRID attribute.
     */
    SetTableAttributesRequest(std::string&& database, std::string&& table,
            std::optional<std::uint64_t>&& nextTrid) noexcept
        : DBEngineRequest(DBEngineRequestType::kSetTableAttributes)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_nextTrid(std::move(nextTrid))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** Next TRID attribute */
    const std::optional<std::uint64_t> m_nextTrid;
};

/** ALTER TABLE ADD COLUMN request */
struct AddColumnRequest : public DBEngineRequest {
    /**
     * Initializes object of class AddColumnRequest.
     * @param database Database name.
     * @param table Table name.
     * @param column New column definition.
     */
    AddColumnRequest(
            std::string&& database, std::string&& table, ColumnDefinition&& column) noexcept
        : DBEngineRequest(DBEngineRequestType::kAddColumn)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_column(std::move(column))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** New column definition */
    const ColumnDefinition m_column;
};

/** ALTER TABLE DROP COLUMN request */
struct DropColumnRequest : public DBEngineRequest {
    /**
     * Initializes object of class DropColumnRequest.
     * @param database Database name.
     * @param table Table name.
     * @param column Column name.
     * @param ifExists Indicates that operation should not fail if column doesn't exist.
     */
    DropColumnRequest(std::string&& database, std::string&& table, std::string&& column,
            bool ifExists = false) noexcept
        : DBEngineRequest(DBEngineRequestType::kDropColumn)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_column(std::move(column))
        , m_ifExists(ifExists)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** Column name */
    const std::string m_column;

    /** Indicates that operation should not fail if column doesn't exist */
    const bool m_ifExists;
};

/** ALTER TABLE ALTER COLUMN RENAME TO request */
struct RenameColumnRequest : public DBEngineRequest {
    /**
     * Initializes object of class RenameColumnRequest.
     * @param database Database name.
     * @param table Table name.
     * @param column Column name.
     * @param newColumn New column name.
     * @param ifExists Indicates that operation should not fail if column doesn't exist.
     */
    RenameColumnRequest(std::string&& database, std::string&& table, std::string&& column,
            std::string&& newColumn, bool ifExists = false) noexcept
        : DBEngineRequest(DBEngineRequestType::kRenameColumn)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_column(std::move(column))
        , m_newColumn(std::move(newColumn))
        , m_ifExists(ifExists)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** Column name */
    const std::string m_column;

    /** New column name */
    const std::string m_newColumn;

    /** Indicates that operation should not fail if column doesn't exist */
    const bool m_ifExists;
};

/** ALTER TABLE ALTER COLUMN request */
struct RedefineColumnRequest : public DBEngineRequest {
    /**
     * Initializes object of class RedefineColumnRequest.
     * @param database Database name.
     * @param table Table name.
     * @param newColumn New column definition.
     */
    RedefineColumnRequest(
            std::string&& database, std::string&& table, ColumnDefinition&& newColumn) noexcept
        : DBEngineRequest(DBEngineRequestType::kRedefineColumn)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_newColumn(std::move(newColumn))
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** New column definition */
    const ColumnDefinition m_newColumn;
};

/** Index column definition */
struct IndexColumnDefinition {
    /**
     * Initializes object of class IndexColumnDefinition.
     * @param name Column name.
     * @param sortDescending Indicates that sort order is descending.
     */
    explicit IndexColumnDefinition(std::string&& name, bool sortDescending = false) noexcept
        : m_name(std::move(name))
        , m_sortDescending(sortDescending)
    {
    }

    /** Column name */
    const std::string m_name;

    /** Indicates that sort order is descending */
    const bool m_sortDescending;
};

/** CREATE INDEX request */
struct CreateIndexRequest : public DBEngineRequest {
    /**
     * Initializes object of class CreateIndexRequest.
     * @param database Database name.
     * @param table Table name.
     * @param index Index name.
     * @param ifDoesntExist Indicates that operation should not fail if the index already exists.
     */
    CreateIndexRequest(std::string&& database, std::string&& table, std::string&& index,
            std::vector<IndexColumnDefinition>&& columns, bool unique,
            bool ifDoesntExist = false) noexcept
        : DBEngineRequest(DBEngineRequestType::kCreateIndex)
        , m_database(std::move(database))
        , m_table(std::move(table))
        , m_index(std::move(index))
        , m_columns(std::move(columns))
        , m_unique(unique)
        , m_ifDoesntExist(ifDoesntExist)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Table name */
    const std::string m_table;

    /** Index name */
    const std::string m_index;

    /** List of columns */
    const std::vector<IndexColumnDefinition> m_columns;

    /** Indication that index is unique */
    const bool m_unique;

    /** Indicates that operation should not fail if the index already exists */
    const bool m_ifDoesntExist;
};

/** DROP INDEX request */
struct DropIndexRequest : public DBEngineRequest {
    /**
     * Initializes object of class DropIndexRequest.
     * @param database Database name.
     * @param index Index name.
     * @param ifExists Indicates that operation should not fail if index doesn't exist.
     */
    DropIndexRequest(std::string&& database, std::string&& index, bool ifExists = false) noexcept
        : DBEngineRequest(DBEngineRequestType::kDropIndex)
        , m_database(std::move(database))
        , m_index(std::move(index))
        , m_ifExists(ifExists)
    {
    }

    /** Database name */
    const std::string m_database;

    /** Index name */
    const std::string m_index;

    /** Indicates that operation should not fail if index doesn't exist */
    const bool m_ifExists;
};

/** CREATE USER request */
struct CreateUserRequest : public DBEngineRequest {
    /**
     * Initializes object of class CreateUserRequest.
     * @param name User account name.
     * @param realName User real name.
     * @param description User description.
     * @param active Indication that user is active.
     */
    CreateUserRequest(std::string&& name, std::optional<std::string>&& realName,
            std::optional<std::string>&& description, bool active) noexcept
        : DBEngineRequest(DBEngineRequestType::kCreateUser)
        , m_name(std::move(name))
        , m_realName(std::move(realName))
        , m_description(std::move(description))
        , m_active(active)
    {
    }

    /** User account name */
    const std::string m_name;

    /** User real name */
    const std::optional<std::string> m_realName;

    /** User description */
    const std::optional<std::string> m_description;

    /** Indication that user is active */
    const bool m_active;
};

/** DROP USER request */
struct DropUserRequest : public DBEngineRequest {
    /**
     * Initializes object of class DropUserRequest.
     * @param name User account name.
     * @param ifExists Indicates that IF EXISTS conditions present.
     */
    DropUserRequest(std::string&& name, bool ifExists) noexcept
        : DBEngineRequest(DBEngineRequestType::kDropUser)
        , m_name(std::move(name))
        , m_ifExists(ifExists)
    {
    }

    /** User account name */
    const std::string m_name;

    /** IF EXISTS conditions */
    const bool m_ifExists;
};

/** ALTER USER SET attributes request */
struct SetUserAttributesRequest : public DBEngineRequest {
    /**
     * Initializes object of class SetUserAttributesRequest.
     * @param userName User name.
     * @param realName New real name.
     * @param description New description.
     * @param active New state.
     */
    SetUserAttributesRequest(std::string&& userName,
            std::optional<std::optional<std::string>>&& realName,
            std::optional<std::optional<std::string>>&& description,
            std::optional<bool>&& active) noexcept
        : DBEngineRequest(DBEngineRequestType::kSetUserAttributes)
        , m_userName(std::move(userName))
        , m_params(std::move(realName), std::move(description), std::move(active))
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Update parameters */
    const UpdateUserParameters m_params;
};

/** ALTER USER ADD ACCESS KEY request */
struct AddUserAccessKeyRequest : public DBEngineRequest {
    /**
     * Initializes object of class AddUserAccessKeyRequest.
     * @param userName User account name.
     * @param keyName Key name.
     * @param text Key text.
     * @param active Indication that key is active.
     */
    AddUserAccessKeyRequest(std::string&& userName, std::string&& keyName, std::string&& text,
            std::optional<std::string>&& description, bool active) noexcept
        : DBEngineRequest(DBEngineRequestType::kAddUserAccessKey)
        , m_userName(std::move(userName))
        , m_keyName(std::move(keyName))
        , m_text(std::move(text))
        , m_description(description)
        , m_active(active)
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Key name */
    const std::string m_keyName;

    /** Key text */
    const std::string m_text;

    /** Key description */
    const std::optional<std::string> m_description;

    /** Indication that key is active */
    const bool m_active;
};

/** ALTER USER DROP ACCESS KEY request */
struct DropUserAccessKeyRequest : public DBEngineRequest {
    /**
     * Initializes object of class DropUserAccessKeyRequest.
     * @param userName User account name.
     * @param keyName Key name.
     * @param ifExists Indicates that IF EXISTS flag present.
     */
    DropUserAccessKeyRequest(std::string&& userName, std::string&& keyName, bool ifExists) noexcept
        : DBEngineRequest(DBEngineRequestType::kDropUserAccessKey)
        , m_userName(std::move(userName))
        , m_keyName(std::move(keyName))
        , m_ifExists(ifExists)
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Key name */
    const std::string m_keyName;

    /** IF EXISTS flag */
    const bool m_ifExists;
};

/** ALTER USER ALTER ACCESS KEY SET attributes request */
struct SetUserAccessKeyAttributesRequest : public DBEngineRequest {
    /**
     * Initializes object of class SetUserAccessKeyAttributesRequest.
     * @param userName User account name.
     * @param keyName Key name.
     * @param description Key description.
     * @param active Key state.
     */
    SetUserAccessKeyAttributesRequest(std::string&& userName, std::string&& keyName,
            std::optional<std::optional<std::string>>&& description,
            std::optional<bool>&& active) noexcept
        : DBEngineRequest(DBEngineRequestType::kSetUserAccessKeyAttributes)
        , m_userName(std::move(userName))
        , m_keyName(std::move(keyName))
        , m_params(std::move(description), std::move(active))
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Key name */
    const std::string m_keyName;

    /** Update parameters */
    const UpdateUserAccessKeyParameters m_params;
};

/** ALTER USER ALTER ACCESS KEY RENAME TO request */
struct RenameUserAccessKeyRequest : public DBEngineRequest {
    /**
     * Initializes object of class RenameUserAccessKeyRequest.
     * @param userName User account name.
     * @param keyName Key name.
     * @param newKeyName New key name.
     * @param ifExists Indicates that IF EXISTS flag present.
     */
    RenameUserAccessKeyRequest(std::string&& userName, std::string&& keyName,
            std::string&& newKeyName, bool ifExists) noexcept
        : DBEngineRequest(DBEngineRequestType::kRenameUserAccessKey)
        , m_userName(std::move(userName))
        , m_keyName(std::move(keyName))
        , m_newKeyName(std::move(newKeyName))
        , m_ifExists(ifExists)
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Key name */
    const std::string m_keyName;

    /** New key name */
    const std::string m_newKeyName;

    /** IF EXISTS flag */
    const bool m_ifExists;
};

/** ALTER USER ADD TOKEN request */
struct AddUserTokenRequest : public DBEngineRequest {
    /**
     * Initializes object of class AddUserTokenRequest.
     * @param userName User account name.
     * @param tokenName Token name.
     * @param value Token value.
     * @param expirationTimestamp Token expiration timestamp.
     * @param description Token description.
     */
    AddUserTokenRequest(std::string&& userName, std::string&& tokenName,
            std::optional<BinaryValue>&& value, std::optional<std::time_t>&& expirationTimestamp,
            std::optional<std::string>&& description) noexcept
        : DBEngineRequest(DBEngineRequestType::kAddUserToken)
        , m_userName(std::move(userName))
        , m_tokenName(std::move(tokenName))
        , m_value(std::move(value))
        , m_expirationTimestamp(std::move(expirationTimestamp))
        , m_description(std::move(description))
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Token name */
    const std::string m_tokenName;

    /** Token text */
    const std::optional<BinaryValue> m_value;

    /** Token expiration timestamp */
    const std::optional<std::time_t> m_expirationTimestamp;

    /** Token description */
    const std::optional<std::string> m_description;
};

/** ALTER USER DROP TOKEN request */
struct DropUserTokenRequest : public DBEngineRequest {
    /**
     * Initializes object of class DropUserTokenRequest.
     * @param userName User account name.
     * @param tokenName Token name.
     * @param ifExists Indicates that IF EXISTS flag present.
     */
    DropUserTokenRequest(std::string&& userName, std::string&& tokenName, bool ifExists) noexcept
        : DBEngineRequest(DBEngineRequestType::kDropUserToken)
        , m_userName(std::move(userName))
        , m_tokenName(std::move(tokenName))
        , m_ifExists(ifExists)
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Token name */
    const std::string m_tokenName;

    /** IF EXISTS flag */
    const bool m_ifExists;
};

/** ALTER USER ALTER ACCESS KEY SET attributes request */
struct SetUserTokenAttributesRequest : public DBEngineRequest {
    /**
     * Initializes object of class SetUserTokenAttributesRequest.
     * @param userName User account name.
     * @param tokenName Token name.
     * @param expirationTimestamp Token expiration timestamp.
     * @param description Token description.
     */
    SetUserTokenAttributesRequest(std::string&& userName, std::string&& tokenName,
            std::optional<std::optional<std::time_t>>&& expirationTimestamp,
            std::optional<std::optional<std::string>>&& description) noexcept
        : DBEngineRequest(DBEngineRequestType::kSetUserTokenAttributes)
        , m_userName(std::move(userName))
        , m_tokenName(std::move(tokenName))
        , m_params(std::move(expirationTimestamp), std::move(description))
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Key name */
    const std::string m_tokenName;

    /** Update parameters */
    const UpdateUserTokenParameters m_params;
};

/** ALTER USER ALTER TOKEN RENAME TO request */
struct RenameUserTokenRequest : public DBEngineRequest {
    /**
     * Initializes object of class RenameUserTokenRequest.
     * @param userName User account name.
     * @param tokenName Token name.
     * @param newTokenName New token name.
     * @param ifExists Indicates that IF EXISTS flag present.
     */
    RenameUserTokenRequest(std::string&& userName, std::string&& tokenName,
            std::string&& newTokenName, bool ifExists) noexcept
        : DBEngineRequest(DBEngineRequestType::kRenameUserToken)
        , m_userName(std::move(userName))
        , m_tokenName(std::move(tokenName))
        , m_newTokenName(std::move(newTokenName))
        , m_ifExists(ifExists)
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Token name */
    const std::string m_tokenName;

    /** New token name */
    const std::string m_newTokenName;

    /** IF EXISTS flag */
    const bool m_ifExists;
};

/** CHECK TOKEN request */
struct CheckUserTokenRequest : public DBEngineRequest {
    /**
     * Initializes object of class RenameUserTokenRequest.
     * @param userName User account name.
     * @param tokenName Token name.
     * @param tokenValue Token value.
     */
    CheckUserTokenRequest(
            std::string&& userName, std::string&& tokenName, BinaryValue&& tokenValue) noexcept
        : DBEngineRequest(DBEngineRequestType::kCheckUserToken)
        , m_userName(std::move(userName))
        , m_tokenName(std::move(tokenName))
        , m_tokenValue(std::move(tokenValue))
    {
    }

    /** User account name */
    const std::string m_userName;

    /** Token name */
    const std::string m_tokenName;

    /** New token name */
    const BinaryValue m_tokenValue;
};

/** SHOW DATABASES request */
struct ShowDatabasesRequest : public DBEngineRequest {
    /** Initializes object of class ShowDatabasesRequest */
    ShowDatabasesRequest() noexcept
        : DBEngineRequest(DBEngineRequestType::kShowDatabases)
    {
    }
};

/** SHOW TABLES request */
struct ShowTablesRequest : public DBEngineRequest {
    /** Initializes object of class ShowTablesRequest */
    ShowTablesRequest() noexcept
        : DBEngineRequest(DBEngineRequestType::kShowTables)
    {
    }

    /** List of tables */
    const std::vector<SourceTable> m_tables;

    /* List of resulting columns */
    const std::vector<ResultExpression> m_resultExpressions;
};

}  // namespace siodb::iomgr::dbengine::requests
