// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "UserAccessKeyPtr.h"
#include "UserPtr.h"
#include "reg/UserRecord.h"

// STL headers
#include <mutex>

namespace siodb::iomgr::dbengine {

class UserPermission;

/** User ID generator */
class UserIdGenerator {
public:
    /** De-initializes object of class UserIdSource */
    virtual ~UserIdGenerator() = default;

    /**
     * Generates new unique user ID.
     * @return New user ID.
     */
    virtual std::uint32_t generateNextUserId() = 0;
};

/** Database user */
class User {
public:
    /** Super user name, must be in the upper-case. */
    static constexpr const char* kSuperUserName = "ROOT";

    /** Super user description. */
    static constexpr const char* kSuperUserDescription =
            "Super user. Has full access to the instance.";

    /** Super user ID */
    static constexpr std::uint32_t kSuperUserId = 1;

public:
    /**
     * Initializes object of class User for the new user.
     * @param userIdGenerator User ID generator object (usually system database).
     * @param name User name.
     * @param realName Real name.
     * @param active Initial state.
     */
    User(UserIdGenerator& userIdGenerator, std::string&& name,
            std::optional<std::string>&& realName, std::optional<std::string>&& description,
            bool active);

    /**
     * Initializes object of class User for an existing user.
     * @param userRecord User record.
     */
    explicit User(const UserRecord& userRecord);

    /**
     * Returns indication that this is superuser.
     * @return true is this user object refers to the superuser, false otherwise.
     */
    bool isSuperUser() const noexcept
    {
        return m_id == kSuperUserId;
    }

    /**
     * Returns user ID.
     * @return User ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns user name.
     * @return User name.
     */
    const auto& getName() const noexcept
    {
        return m_name;
    }

    /**
     * Returns user real name.
     * @return User real name.
     */
    const auto& getRealName() const
    {
        return m_realName;
    }

    /**
     * Sets real name of this user.
     * @param realName New real name.
     */
    void setRealName(std::optional<std::string>&& realName)
    {
        m_realName = std::move(realName);
    }

    /**
     * Returns user description.
     * @return User description.
     */
    const auto& getDescription() const
    {
        return m_description;
    }

    /**
     * Sets description of this user.
     * @param description New description.
     */
    void setDescription(std::optional<std::string>&& description)
    {
        m_description = std::move(description);
    }

    /**
     * Returns indication that user is active.
     * @return true if user is active, false otherwise.
     */
    bool isActive() const
    {
        return m_active;
    }

    /**
     * Sets indication that user is active.
     * @param active New indication that user is active.
     */
    void setActive(bool active)
    {
        m_active = active;
    }

    /**
     * Returns collection of user access keys.
     * @return User access key collection.
     */
    const auto& getAccessKeys() const noexcept
    {
        return m_accessKeys;
    }

    /**
     * Returns copy of user access key collection.
     * @return Copy of user access keys collection.
     */
    auto getAccessKeysCopy() const
    {
        return m_accessKeys;
    }

    /**
     * Returns existing user access key.
     * @param name User access key name.
     * @return User access key object.
     * @throw DatabaseError if user access key object doesn't exists.
     */
    UserAccessKeyPtr findUserAccessKeyChecked(const std::string& name) const;

    /**
     * Creates new user access key.
     * @param id Access key ID.
     * @param name Key name.
     * @param text Key text.
     * @param active Indication that key is active.
     * @return Created user access key.
     * @throw DatabaseError if some error has occurrred.
     */
    UserAccessKeyPtr addUserAccessKey(std::uint64_t id, std::string&& name, std::string&& text,
            std::optional<std::string>&& description, bool active);

    /**
     * Deletes existing user access key.
     * @param name Key name.
     * @throw DatabaseError if some error has occurrred.
     */
    void deleteUserAccessKey(const std::string& name);

    /**
     * Returns active key count.
     * @return Active key count.
     */
    std::size_t getActiveKeyCount() const noexcept;

private:
    /**
     * Validates user name.
     * @param userName User name.
     * @return Same user name, if it is valid.
     * @throw DatabaseError if user name is invalid.
     */
    static std::string&& validateUserName(std::string&& userName);

    /**
     * Returns existing user access key.
     * @param name User access key name.
     * @return User access key object or nullptr if it doesn't exist.
     */
    UserAccessKeyPtr findUserAccessKeyUnlocked(const std::string& name) const noexcept;

private:
    /** User name */
    const std::string m_name;

    /** Real name */
    std::optional<std::string> m_realName;

    /** User description */
    std::optional<std::string> m_description;

    /** User state */
    bool m_active;

    /** Access keys */
    std::vector<UserAccessKeyPtr> m_accessKeys;

    /** User ID */
    const std::uint32_t m_id;
};

}  // namespace siodb::iomgr::dbengine
