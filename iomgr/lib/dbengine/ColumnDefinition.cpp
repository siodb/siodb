// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDefinition.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDefinitionConstraintList.h"
#include "DefaultValueConstraint.h"
#include "NotNullConstraint.h"
#include "ThrowDatabaseError.h"
#include "parser/EmptyExpressionEvaluationContext.h"

namespace siodb::iomgr::dbengine {

ColumnDefinition::ColumnDefinition(Column& column)
    : m_column(column)
    , m_id(m_column.getDatabase().generateNextColumnDefinitionId(
              m_column.getTable().isSystemTable()))
    , m_constraints(std::make_shared<ColumnDefinitionConstraintList>())
    , m_openForModification(true)
{
}

ColumnDefinition::ColumnDefinition(
        Column& column, const ColumnDefinitionRecord& columnDefinitionRecord)
    : m_column(validateColumn(column, columnDefinitionRecord))
    , m_id(columnDefinitionRecord.m_id)
    , m_constraints(createConstraints(columnDefinitionRecord))
    , m_openForModification(false)
{
}

bool ColumnDefinition::hasConstraints() const noexcept
{
    return !m_constraints->empty();
}

std::size_t ColumnDefinition::getConstraintCount() const noexcept
{
    // NOTE: Cannot be moved to header due to compilation dependencies
    return m_constraints ? m_constraints->size() : 0;
}

bool ColumnDefinition::isNotNull() const noexcept
{
    if (!m_constraints) return false;
    const auto& index = m_constraints->byConstraintType();
    const auto it = index.find(ConstraintType::kNotNull);
    return (it == index.cend())
                   ? false
                   : dynamic_cast<const NotNullConstraint&>((*it)->getConstraint()).isNotNull();
}

Variant ColumnDefinition::getDefaultValue() const
{
    if (!m_constraints) return Variant();
    const auto& index = m_constraints->byConstraintType();
    const auto it = index.find(ConstraintType::kDefaultValue);
    if (it == index.cend()) return Variant();
    requests::EmptyExpressionEvaluationContext ctx;
    return dynamic_cast<const DefaultValueConstraint&>((*it)->getConstraint())
            .getExpression()
            .evaluate(ctx);
}

void ColumnDefinition::markClosedForModification()
{
    m_openForModification = false;
    getDatabase().updateColumnDefinitionRegistration(*this);
}

std::uint64_t ColumnDefinition::addConstraint(const ConstraintPtr& constraint)
{
    if (!m_openForModification) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotModifyClosedColumnDefinition,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), m_id,
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId());
    }
    auto columnDefinitionConstraint =
            std::make_shared<ColumnDefinitionConstraint>(*this, constraint);
    m_constraints->insert(columnDefinitionConstraint);
    return columnDefinitionConstraint->getId();
}

// --- internal ---

Column& ColumnDefinition::validateColumn(
        Column& column, const ColumnDefinitionRecord& columnDefinitionRecord)
{
    if (columnDefinitionRecord.m_columnId == column.getId()) return column;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnDefinitionColumn,
            columnDefinitionRecord.m_id, columnDefinitionRecord.m_columnId,
            column.getDatabaseName(), column.getTableName(), column.getName(),
            column.getDatabaseUuid(), column.getTableId(), column.getId());
}

std::shared_ptr<ColumnDefinitionConstraintList> ColumnDefinition::createEmptyConstraints()
{
    return std::make_shared<ColumnDefinitionConstraintList>();
}

std::shared_ptr<ColumnDefinitionConstraintList> ColumnDefinition::createConstraints(
        const ColumnDefinitionRecord& columnDefinitionRecord)
{
    auto constraints = std::make_shared<ColumnDefinitionConstraintList>();
    for (const auto& columnDefinitionConstraintRecord :
            columnDefinitionRecord.m_constraints.byId()) {
        constraints->emplace(*this, columnDefinitionConstraintRecord);
    }
    return constraints;
}

}  // namespace siodb::iomgr::dbengine
