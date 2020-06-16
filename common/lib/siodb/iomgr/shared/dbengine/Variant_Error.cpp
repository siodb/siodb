// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant_Error.h"

// STL headers
#include <sstream>

namespace siodb::iomgr::dbengine {

/////////////// class VariantTypeCastError ////////////////////////////////////////////////////

std::string VariantTypeCastError::makeErrorMessage(
        VariantType sourceValueType, VariantType destValueType, const char* reason)
{
    std::ostringstream oss;
    oss << "Could not cast from " << getVariantTypeName2(sourceValueType) << " ["
        << static_cast<int>(sourceValueType) << "] to " << getVariantTypeName2(destValueType)
        << " [" << static_cast<int>(destValueType) << ']';
    if (reason) oss << ": " << reason;
    return oss.str();
}

/////////////// class VariantTypeCastError ////////////////////////////////////////////////////

std::string WrongVariantTypeError::makeErrorMessage(VariantType sourceValueType, const char* reason)
{
    std::ostringstream oss;
    oss << getVariantTypeName(sourceValueType) << " type is not allowed for this operation";
    if (reason) oss << ": " << reason;
    return oss.str();
}

}  // namespace siodb::iomgr::dbengine
