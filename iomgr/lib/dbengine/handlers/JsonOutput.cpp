// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "JsonOutput.h"

// Project headers
#include "RequestHandlerSharedConstants.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>

namespace siodb::iomgr::dbengine {

void writeGetJsonProlog(int statusCode, siodb::io::JsonWriter& jsonWriter)
{
    // Start top level object
    jsonWriter.writeObjectBegin();
    // Write status
    jsonWriter.writeFieldName(kRestStatusCodeFieldName, ::ct_strlen(kRestStatusCodeFieldName));
    jsonWriter.writeValue(statusCode);
    // Start rows array
    jsonWriter.writeComma();
    jsonWriter.writeFieldName(kRestRowsFieldName, ::ct_strlen(kRestRowsFieldName));
    jsonWriter.writeArrayBegin();
}

void writeModificationJsonProlog(
        int statusCode, std::size_t affectedRowCount, siodb::io::JsonWriter& jsonWriter)
{
    // Start top level object
    jsonWriter.writeObjectBegin();

    // Write status
    jsonWriter.writeFieldName(kRestStatusCodeFieldName, ::ct_strlen(kRestStatusCodeFieldName));
    jsonWriter.writeValue(statusCode);

    // Write affected row count
    constexpr const char* kAffectedRowCountFieldName = "affectedRowCount";
    jsonWriter.writeComma();
    jsonWriter.writeFieldName(kAffectedRowCountFieldName, ::ct_strlen(kAffectedRowCountFieldName));
    jsonWriter.writeValue(affectedRowCount);

    // Start rows array
    jsonWriter.writeComma();
    constexpr const char* kTridsFieldName = "trids";
    jsonWriter.writeFieldName(kTridsFieldName, ::ct_strlen(kTridsFieldName));
    jsonWriter.writeArrayBegin();
}

void writeJsonEpilog(siodb::io::JsonWriter& jsonWriter)
{
    // End rows array
    jsonWriter.writeArrayEnd();
    // End top level object
    jsonWriter.writeObjectEnd();
}

}  // namespace siodb::iomgr::dbengine
