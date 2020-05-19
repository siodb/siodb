// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Wrapper file for the antlr-generated/SiodbLexer.h

// We have to use such wrappers to reduce strength of compile-time checks
// on the ANTLR generated code and at the same time keep strong compile-time checks
// on our own code.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wattributes"

#include <siodb-generated/iomgr/lib/dbengine/parser/antlr-generated/SiodbLexer.h>

#pragma GCC diagnostic pop
