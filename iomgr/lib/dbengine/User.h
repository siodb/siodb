// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "UserAccessKeyPtr.h"
#include "UserIdGenerator.h"
#include "UserPermission.h"
#include "UserPtr.h"
#include "UserTokenPtr.h"
#include "reg/UserRecord.h"

// STL headers
#include <mutex>

namespace siodb::iomgr::dbengine {

/** Database user */
class User {
public:
    /** Super user name, must be in the upper-case. */
    static constexpr const char* kSuperUserName = "ROOT";

    /** Super user description. */
    static constexpr const char* kSuperUserDescription =
            "Super user, has constant full access to the instance.";

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
     * Returns collection of access keys.
     * @return Collection of access key.
     */
    const auto& getAccessKeys() const noexcept
    {
        return m_accessKeys;
    }

    /**
     * Finds existing access key.
     * @param name Access key name.
     * @return Access key object.
     * @throw DatabaseError if access key object doesn't exists.
     */
    UserAccessKeyPtr findAccessKeyChecked(const std::string& name) const;

    /**
     * Creates new access key.
     * @param id Access key ID.
     * @param name Access key name.
     * @param text Access key text.
     * @param active Indication that access key is active.
     * @return New access key object.
     * @throw DatabaseError if some error has occurrred.
     */
    UserAccessKeyPtr addAccessKey(std::uint64_t id, std::string&& name, std::string&& text,
            std::optional<std::string>&& description, bool active);

    /**
     * Deletes existing access key.
     * @param name Access key name.
     * @throw DatabaseError if some error has occurrred.
     */
    void deleteAccessKey(const std::string& name);

    /**
     * Returns active access key count.
     * @return Active access key count.
     */
    std::size_t getActiveAccessKeyCount() const noexcept;

    /**
     * Returns collection of tokens.
     * @return Collection of tokens.
     */
    const auto& getTokens() const noexcept
    {
        return m_tokens;
    }

    /**
     * Finds existing token.
     * @param name Token name.
     * @return Token object.
     * @throw DatabaseError if token object doesn't exists.
     */
    UserTokenPtr findTokenChecked(const std::string& name) const;

    /**
     * Creates new token.
     * @param id Token ID.
     * @param name Token name.
     * @param value Token value.
     * @param expirationTimestamp Token expiration timestamp.
     * @param description Token description.
     * @return New token object.
     * @throw DatabaseError if some error has occurrred.
     */
    UserTokenPtr addToken(std::uint64_t id, std::string&& name, const BinaryValue& value,
            std::optional<std::time_t>&& expirationTimestamp,
            std::optional<std::string>&& description);

    /**
     * Deletes existing token.
     * @param name Token name.
     * @throw DatabaseError if some error has occurrred.
     */
    void deleteToken(const std::string& name);

    /**
     * Returns active token count.
     * @return Active token count.
     */
    std::size_t getActiveTokenCount() const noexcept;

    /**
     * Returns collection of the explicitly granted user permissions.
     * @return Collection of the explicitly granted user permissions.
     */
    const auto& getGrantedPermissions() const noexcept
    {
        return m_grantedPermissions;
    }

    /**
     * Grants specific permissions to this user.
     * @param permissionKey User permission key.
     * @param permissions Permissions bitmask.
     * @param withGrantOption Grant option flag, user will be able to grant
     *                        same permissions to others.
     * @return User permission data object. Permission ID will be zero for a new data object
     *         and nonzero for an existing data object.
     */
    UserPermissionDataEx grantPermissions(const UserPermissionKey& permissionKey,
            std::uint64_t permissions, bool withGrantOption);

    /**
     * Revokes specific permissions from this user.
     * @param permissionKey User permission key.
     * @param permissions Permissions bitmask.
     * @return Empty optional if such permissions were not granted, non-empty optional
     *         if one or more permissions are actually revoked. In the latter case, pair fields
     *         have following meaning:
     *         - first indicates new permissions if record is preserved.
     *         - second is true if permission record deleted, false if updated.
     */
    std::optional<std::pair<UserPermissionDataEx, bool>> revokePermissions(
            const UserPermissionKey& permissionKey, std::uint64_t permissions);

    /**
     * Grants specific permissions to this user.
     * @param permissionKey User permission key.
     * @param id Permission record ID.
     * @return Pair of flags: first = true if record found, false otherwise.
     *         second = true if passed ID was assigned, false otherwise (record already has and ID).
     */
    std::pair<bool, bool> setPermissionRecordId(
            const UserPermissionKey& permissionKey, std::uint64_t id);

    /**
     * Authenticates user with access key.
     * @param signature Signed challenge with user private key.
     * @param challenge Random challenge.
     * @return true on authentication success, false otherwise.
     */
    bool authenticate(const std::string& signature, const std::string& challenge) const;

    /**
     * Authenticates user with token.
     * @param tokenValue Token value.
     * @return true on authentication success, false otherwise.
     */
    bool authenticate(const std::string& tokenValue) const;

    /**
     * Check user token match.
     * @param tokenValue Token value.
     * @param allowExpiredToken Allow expired token to match.
     * @return true if token matched, false otherwise.
     */
    bool checkToken(const BinaryValue& tokenValue, bool allowExpiredToken = false) const noexcept;

private:
    /**
     * Validates user name.
     * @param userName User name.
     * @return Same user name, if it is valid.
     * @throw DatabaseError if user name is invalid.
     */
    static std::string&& validateUserName(std::string&& userName);

    /**
     * Finds existing access key.
     * @param name Access key name.
     * @return Access key object or nullptr if it doesn't exist.
     */
    UserAccessKeyPtr findAccessKeyUnlocked(const std::string& name) const noexcept;

    /**
     * Finds existing token.
     * @param name Token name.
     * @return Token object or nullptr if it doesn't exist.
     */
    UserTokenPtr findTokenUnlocked(const std::string& name) const noexcept;

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

    /** Tokens */
    std::vector<UserTokenPtr> m_tokens;

    /** Explicitly granted permissions */
    UserPermissionMapEx m_grantedPermissions;

    /** User ID */
    const std::uint32_t m_id;
};

}  // namespace siodb::iomgr::dbengine
