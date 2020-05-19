// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ErrorCodeChecker.h"

// Project headers
#include "SignalHandlers.h"

namespace siodb::utils {

///// class DefaultErrorCodeChecker ////////////////////////////////////////////

bool DefaultErrorCodeChecker::isError(int errorCode) const noexcept
{
    return errorCode && errorCode != EINTR;
}

///// class ExitSignalAwareErrorCodeChecker ////////////////////////////////////

bool ExitSignalAwareErrorCodeChecker::isError(int errorCode) const noexcept
{
    return errorCode && (errorCode != EINTR || isExitEventSignaled());
}

}  // namespace siodb::utils
