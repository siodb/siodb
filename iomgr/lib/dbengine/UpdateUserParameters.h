// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <optional>
#include <string>

namespace siodb::iomgr::dbengine {

/** Update user parameters. */
struct UpdateUserParameters {
    /** Initializes new object of class UpdateUserParameters */
    UpdateUserParameters() noexcept = default;

    /** 
     * Initializes new object of class UpdateUserParameters.
     * @param realName User real name.
     * @param description User description.
     * @param active User state.
     */
    UpdateUserParameters(std::optional<std::optional<std::string>>&& realName,
            std::optional<std::optional<std::string>>&& description,
            std::optional<bool>&& active) noexcept
        : m_realName(std::move(realName))
        , m_description(std::move(description))
        , m_active(std::move(active))
    {
    }

    /** New user real name */
    std::optional<std::optional<std::string>> m_realName;

    /** New user description */
    std::optional<std::optional<std::string>> m_description;

    /** New user state */
    std::optional<bool> m_active;
};

}  // namespace siodb::iomgr::dbengine
