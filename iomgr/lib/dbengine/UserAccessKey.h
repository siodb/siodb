// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "User.h"
#include "UserAccessKeyPtr.h"
#include "reg/UserAccessKeyRecord.h"

// STL headers
#include <atomic>

namespace siodb::iomgr::dbengine {

/** User access key */
class UserAccessKey {
public:
    /** Super user initial access key ID */
    static constexpr std::uint64_t kSuperUserInitialAccessKeyId = 1;

    /** Super user initial access key name */
    static constexpr const char* kSuperUserInitialAccessKeyName = "super_user_initial_access_key";

    /** Super user initial access key description */
    static constexpr const char* kSuperUserInitialAccessKeyDescription =
            "Initial access key of the seper user";

public:
    /**
     * Initializes object of class UserAccessKey for the new access key.
     * @param user User object.
     * @param id Access key ID.
     * @param name Access key name.
     * @param text Access key text.
     * @param description Access key description.
     * @param active Indication that access key is active.
     */
    UserAccessKey(User& user, std::uint64_t id, std::string&& name, std::string&& text,
            std::optional<std::string>&& description, bool active);

    /**
     * Initializes object of class UserAccessKey for an existing access key.
     * @param accessKeyRecord Access key record.
     */
    UserAccessKey(User& user, const UserAccessKeyRecord& accessKeyRecord);

    /**
     * Returns access key ID.
     * @return access key ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns user ID.
     * @return User ID.
     */
    auto getUserId() const noexcept
    {
        return m_user.getId();
    }

    /**
     * Returns user name.
     * @return User name.
     */
    const auto& getUserName() const noexcept
    {
        return m_user.getName();
    }

    /**
     * Returns user object.
     * @return User object.
     */
    User& getUser() const noexcept
    {
        return m_user;
    }

    /**
     * Returns access key name.
     * @return Access key name.
     */
    const auto& getName() const noexcept
    {
        return m_name;
    }

    /**
     * Returns access key text.
     * @return Access key text.
     */
    const auto& getText() const noexcept
    {
        return m_text;
    }

    /**
     * Returns access key decription.
     * @return Access key description.
     */
    const auto& getDescription() const noexcept
    {
        return m_description;
    }

    /**
     * Returns indication that access key is active.
     * @return true if this access key is active, false otherwise.
     */
    bool isActive() const noexcept
    {
        return m_active;
    }

    /**
     * Sets indication that user access key is active.
     * @param active New indication that user is active.
     */
    void setActive(bool active) noexcept
    {
        m_active = active;
    }

private:
    /**
     * Validates user.
     * @param user User object.
     * @param accessKeyRecord User access key record.
     * @return User object, if user ID matches.
     * @throw DatabaseError If user ID doesn't match.
     */
    static User& validateUser(User& user, const UserAccessKeyRecord& accessKeyRecord);

    /**
     * Validates user access key name.
     * @param accessKeyName User name.
     * @return Same user access key name, if it is valid.
     * @throw DatabaseError if user access key name is invalid.
     */
    static std::string&& validateUserAccessKeyName(std::string&& accessKeyName);

private:
    /** User object */
    User& m_user;

    /** Access key ID */
    const std::uint64_t m_id;

    /** Access key name */
    std::string m_name;

    /** Access key text */
    const std::string m_text;

    /** Access key description */
    std::optional<std::string> m_description;

    /** State of the access key */
    std::atomic<bool> m_active;
};

}  // namespace siodb::iomgr::dbengine
