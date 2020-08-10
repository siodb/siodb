// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <ctime>

// STL headers
#include <optional>
#include <string>

namespace siodb::iomgr::dbengine {

/** Update user token parameters. */
struct UpdateUserTokenParameters {
    /** Initializes new object of class UpdateUserTokenParameters */
    UpdateUserTokenParameters() noexcept = default;

    /** 
     * Initializes new object of class UpdateUserTokenParameters.
     * @param expirationTimestamp User token expiration timestamp.
     * @param description User token description.
     */
    UpdateUserTokenParameters(std::optional<std::optional<std::time_t>>&& expirationTimestamp,
            std::optional<std::optional<std::string>>&& description) noexcept
        : m_expirationTimestamp(std::move(expirationTimestamp))
        , m_description(std::move(description))
    {
    }

    /** New user token expiration timestamp */
    std::optional<std::optional<std::time_t>> m_expirationTimestamp;

    /** New user token description */
    std::optional<std::optional<std::string>> m_description;
};

}  // namespace siodb::iomgr::dbengine
