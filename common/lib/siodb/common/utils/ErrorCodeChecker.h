// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT haders
#include <cerrno>

namespace siodb::utils {

/** Provides callback method for checking error codes */
class ErrorCodeChecker {
public:
    virtual ~ErrorCodeChecker() = default;

    /**
     * Checks if error code really indicates error.
     * @param errorCode The error code.
     * @return true if error code indicates error, false otherwise.
     */
    virtual bool isError(int errorCode) const noexcept = 0;
};

/** Default error code checker. Treats as error all error codes except 0 and EINTR */
class DefaultErrorCodeChecker final : public ErrorCodeChecker {
public:
    /**
     * Checks if error code really indicates error.
     * @param errorCode The error code.
     * @return true if error code indicates error, false otherwise.
     */
    bool isError(int errorCode) const noexcept override;
};

/**
 * Error code checker that basically treats as error all error codes except 0, however
 * threats EINTR as error only if exit signal was detected.
 */
class ExitSignalAwareErrorCodeChecker final : public ErrorCodeChecker {
public:
    /**
     * Checks if error code really indicates error.
     * @param errorCode The error code.
     * @return true if error code indicates error, false otherwise.
     */
    bool isError(int errorCode) const noexcept override;
};

}  // namespace siodb::utils
