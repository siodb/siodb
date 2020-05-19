
// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "dbengine/parser/expr/Expression.h"

void testExpressionSerialization(const siodb::iomgr::dbengine::requests::Expression& expr,
        const std::size_t expectedSerializedSize);
