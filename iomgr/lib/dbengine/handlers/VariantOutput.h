// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/io/JsonWriter.h>
#include <siodb/common/protobuf/ExtendedCodedOutputStream.h>
#include <siodb/iomgr/shared/dbengine/Variant.h>

namespace siodb::iomgr::dbengine {

/**
 * Returns serialized variant size for the binary format.
 * @param value A value.
 * @return Serialized variant size for the binary format.
 * @throw DatabaseError if there is unsupported variant type.
 */
std::size_t getSerializedSize(const Variant& value);

/**
 * Writes variant value into coded output stream in the binary format.
 * @param value A value.
 * @param codedOutput Output stream.
 * @throw DatabaseError if there is unsupported variant type.
 */
void writeVariant(const Variant& value, protobuf::ExtendedCodedOutputStream& codedOutput);

/**
  * Writes variant value as JSON.
  * @param fieldName Field name.
  * @param value A value.
  * @param jsonWriter JSON writer object.
 * @throw DatabaseError if there is unsupported variant type.
  */
void writeVariant(const Variant& value, siodb::io::JsonWriter& jsonWriter);

}  // namespace siodb::iomgr::dbengine
