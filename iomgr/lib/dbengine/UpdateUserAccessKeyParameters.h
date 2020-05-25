// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <optional>
#include <string>

namespace siodb::iomgr::dbengine {

/** Update user acces key parameters. */
struct UpdateUserAccessKeyParameters {
    /** Initializes new object of class UpdateUserAccessKeyParameters */
    UpdateUserAccessKeyParameters() noexcept = default;

    /** 
     * Initializes new object of class UpdateUserAccessKeyParameters.
     * @param description User description.
     * @param active User state.
     */
    UpdateUserAccessKeyParameters(std::optional<std::optional<std::string>>&& description,
            std::optional<bool>&& active) noexcept
        : m_description(std::move(description))
        , m_active(std::move(active))
    {
    }

    /** New user access key description */
    std::optional<std::optional<std::string>> m_description;

    /** New user access key state */
    std::optional<bool> m_active;
};

}  // namespace siodb::iomgr::dbengine
