// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "dbengine/parser/expr/Expression.h"

// Common project headers
#include <siodb/common/utils/Debug.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace requests = dbengine::requests;

// Class for simulating database context for the expression test
// Test context is exist of values below in TestTbl table
class TestContext : public requests::Expression::Context {
private:
    const dbengine::Variant& getColumnValue(
            std::size_t tableIndex, std::size_t columnIndex) override;

    siodb::ColumnDataType getColumnDataType(
            std::size_t tableIndex, std::size_t columnIndex) const override;

private:
    std::vector<dbengine::Variant> m_values = {
            std::uint64_t(1),
            std::string("121 Anselmo str."),
            std::int32_t(-25),
            1230.0165432,
            siodb::RawDateTime("2019-12-19", dbengine::Variant::kDefaultDateFormat),
            nullptr,
    };
};
