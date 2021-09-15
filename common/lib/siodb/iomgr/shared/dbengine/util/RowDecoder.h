// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../Variant.h"

// Common project headers
#include <siodb/common/proto/ColumnDataType.pb.h>

namespace siodb::iomgr::dbengine::util {

/**
 * Decodes single data row from memory into series of Variants.
 * @note For use in unit tests only and as reference implementation of the data decoder.
 * @param buffer Row data buffer.
 * @param length Row data buffer length.
 * @param totalColumnCount Total number of columns.
 * @param columnsToDecodeCount Number of columns to decode.
 * @param dataTypes Array of column data types.
 * @param hasNullableColumns Indication that there are nullable columns.
 * @return Vector of column values.
 * @throw std::invalid_argument if there is not enough data or invalid data encountered.
 */
std::vector<Variant> decodeRow(const std::uint8_t* buffer, std::size_t length,
        std::size_t totalColumnCount, std::size_t columnsToDecodeCount,
        const ColumnDataType* dataTypes, bool hasNullableColumns = false);

}  // namespace siodb::iomgr::dbengine::util
