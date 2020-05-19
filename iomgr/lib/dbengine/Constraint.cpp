// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Constraint.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "DatabaseObjectName.h"
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/utils/EmptyString.h>

namespace siodb::iomgr::dbengine {

Constraint::Constraint(Table& table, const std::string& name,
        const ConstConstraintDefinitionPtr& constraintDefinition)
    : m_table(table)
    , m_name(name.empty() ? name : validateConstraintName(name))
    , m_id(m_table.getDatabase().generateNextConstraintId(m_table.isSystemTable()))
    , m_state(ConstraintState::kCreating)
    , m_constraintDefinition(constraintDefinition)
{
    if (m_name.empty()) m_name = generateConstraintName();
}

Constraint::Constraint(Table& table, const ConstraintRecord& constraintRecord)
    : m_table(validateTable(table, constraintRecord))
    , m_name(validateConstraintName(constraintRecord.m_name))
    , m_id(constraintRecord.m_id)
    , m_state(constraintRecord.m_state)
    , m_constraintDefinition(
              table.getConstraintDefinitionChecked(constraintRecord.m_constraintDefinitionId))
{
}

Column* Constraint::getColumn() const noexcept
{
    return nullptr;
}

// ----- internals -----

const ConstraintDefinitionPtr& Constraint::checkConstraintType(const Table& table,
        const std::string& constaintName, const ConstraintDefinitionPtr& constraintDefinition,
        ConstraintType expectedType)
{
    table.getDatabase().checkConstraintType(
            table, nullptr, constaintName, *constraintDefinition, expectedType);
    return std::move(constraintDefinition);
}

const ConstraintRecord& Constraint::checkConstraintType(
        const Table& table, const ConstraintRecord& constraintRecord, ConstraintType expectedType)
{
    table.getDatabase().checkConstraintType(table, nullptr, constraintRecord, expectedType);
    return constraintRecord;
}

Table& Constraint::validateTable(Table& table, const ConstraintRecord& constraintRecord)
{
    if (constraintRecord.m_tableId == table.getId()) return table;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidConstraintTable, constraintRecord.m_name,
            constraintRecord.m_id, table.getDatabaseName(), table.getName(),
            table.getDatabaseUuid(), table.getId());
}

const std::string& Constraint::validateConstraintName(const std::string& constraintName) const
{
    if (isValidDatabaseObjectName(constraintName)) return constraintName;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidConstraintNameInTable,
            m_table.getDatabaseName(), m_table.getName(), constraintName);
}

std::string Constraint::generateConstraintName() const
{
    // NOTE: This is not strongly atomic correct, but should work for the most cases.
    std::ostringstream str;
    str << '$' << getConstaintNamePrefix(m_constraintDefinition->getType()) << '$'
        << m_table.getId() << '$' << m_id;
    std::string name, name0 = str.str();
    name = name0;
    std::uint64_t counter = 0;
    while (m_table.getDatabase().isConstraintExists(name)) {
        str.clear();
        str << name0 << "__" << counter;
        ++counter;
        name = str.str();
    }
    return name;
}

}  // namespace siodb::iomgr::dbengine
