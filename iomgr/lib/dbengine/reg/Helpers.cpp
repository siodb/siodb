// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Helpers.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>
#include <siodb/common/utils/DeserializationError.h>

// STL Headers
#include <sstream>

namespace siodb::iomgr::dbengine::helpers {

[[noreturn]] void reportDeserializationFailure(
        const char* className, const char* fieldName, int errorCode)
{
    std::ostringstream err;
    err << "Failed to deserialize field " << className << '.' << fieldName << ": "
        << ((errorCode < 0) ? "data corruption detected" : "not enough data");
    throw utils::DeserializationError(err.str());
}

[[noreturn]] void reportDeserializationFailure(
        const char* className, const char* fieldName, const char* message)
{
    std::ostringstream err;
    err << "Failed to deserialize field " << className << '.' << fieldName << ": " << message;
    throw utils::DeserializationError(err.str());
}

}  // namespace siodb::iomgr::dbengine::helpers
