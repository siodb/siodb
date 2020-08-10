// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <optional>
#include <string>

namespace siodb::iomgr::dbengine {

/** Update user token parameters. */
struct UpdateDatabaseParameters {
    /** Initializes new object of class UpdateDatabaseParameters */
    UpdateDatabaseParameters() noexcept = default;

    /** 
     * Initializes new object of class UpdateDatabaseParameters.
     * @param description User token description.
     */
    UpdateDatabaseParameters(std::optional<std::optional<std::string>>&& description) noexcept
        : m_description(std::move(description))
    {
    }

    /** New user token description */
    std::optional<std::optional<std::string>> m_description;
};

}  // namespace siodb::iomgr::dbengine
