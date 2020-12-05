// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/iomgr/shared/dbengine/parser/expr/Expression.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace requests = dbengine::requests;

class TestContext : public requests::ExpressionEvaluationContext {
private:
    const siodb::iomgr::dbengine::Variant& getColumnValue(
            std::size_t tableIndex, std::size_t columnIndex) override;

    siodb::ColumnDataType getColumnDataType(
            [[maybe_unused]] std::size_t tableIndex, std::size_t columnIndex) const override;

private:
    dbengine::Variant m_value;
};
