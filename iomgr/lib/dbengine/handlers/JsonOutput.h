// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/io/JsonWriter.h>

namespace siodb::iomgr::dbengine {
/**
 * Writes JSON prolog for GET request.
 * @param statusCode Status code.
 * @param jsonWriter JSON writer object.
 * @throw std::system_error on write error.
 */
void writeGetJsonProlog(int statusCode, siodb::io::JsonWriter& jsonWriter);

/**
 * Writes JSON prolog for POST, PATCH, DELETE requests.
 * @param statusCode Status code.
 * @param affectedRowCount Affected row count to include.
 * @param jsonWriter JSON writer object.
 * @throw std::system_error on write error.
 */
void writeModificationJsonProlog(
        int statusCode, std::size_t affectedRowCount, siodb::io::JsonWriter& jsonWriter);

/**
 * Writes JSON epilog.
 * @param jsonWriter JSON writer object.
 * @throw std::system_error on write error.
 */
void writeJsonEpilog(siodb::io::JsonWriter& jsonWriter);

}  // namespace siodb::iomgr::dbengine
