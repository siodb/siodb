// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "User.h"
#include "UserTokenPtr.h"
#include "reg/UserTokenRecord.h"

namespace siodb::iomgr::dbengine {

/** User access key */
class UserToken {
public:
    /**
     * Initializes object of class UserToken for the new token.
     * @param user User object.
     * @param id Token ID.
     * @param name Token name.
     * @param text Token value.
     * @param expirationTimestamp Expiration timestamp.
     * @param description Token description.
     */
    UserToken(User& user, std::uint64_t id, std::string&& name, BinaryValue&& value,
            std::optional<std::time_t>&& expirationTimestamp,
            std::optional<std::string>&& description);

    /**
     * Initializes object of class UserToken for an existing access key.
     * @param tokenRecord Token record.
     */
    UserToken(User& user, const UserTokenRecord& tokenRecord);

    /**
     * Returns token ID.
     * @return Token ID.
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
     * Returns token name.
     * @return Token name.
     */
    const auto& getName() const noexcept
    {
        return m_name;
    }

    /**
     * Returns token value.
     * @return Token value.
     */
    const auto& getValue() const noexcept
    {
        return m_value;
    }

    /**
     * Returns expiration timestamp.
     * @return Expiration timestamp.
     */
    const auto& getExpirationTimestamp() const noexcept
    {
        return m_expirationTimestamp;
    }

    /**
     * Sets expiration timestamp.
     * @param expirationTimestamp Expiration timestamp.
     */
    void setExpirationTimestamp(const std::optional<std::time_t>& expirationTimestamp) noexcept
    {
        m_expirationTimestamp = expirationTimestamp;
    }

    /**
     * Returns token decription.
     * @return Token description.
     */
    const auto& getDescription() const noexcept
    {
        return m_description;
    }

    /**
     * Sets token decription.
     * @param description Token description.
     */
    void setDescription(std::optional<std::string>&& description) noexcept
    {
        m_description = std::move(description);
    }

    /**
     * Returns indication that token has expired.
     * @return true if token has expired, false otherwise.
     */
    bool isExpired() const noexcept
    {
        return m_expirationTimestamp && std::time(nullptr) >= *m_expirationTimestamp;
    }

private:
    /**
     * Validates user.
     * @param user User object.
     * @param TokenRecord User token record.
     * @return User object, if user ID matches.
     * @throw DatabaseError If user ID doesn't match.
     */
    static User& validateUser(User& user, const UserTokenRecord& tokenRecord);

    /**
     * Validates user token name.
     * @param tokenName Token name.
     * @return Same user access key name, if it is valid.
     * @throw DatabaseError if user access key name is invalid.
     */
    static std::string&& validateName(std::string&& tokenName);

private:
    /** User object */
    User& m_user;

    /** Token ID */
    const std::uint64_t m_id;

    /** Token name */
    std::string m_name;

    /** Token text */
    const BinaryValue m_value;

    /** Token expiration timestamp */
    std::optional<std::time_t> m_expirationTimestamp;

    /** Token description */
    std::optional<std::string> m_description;
};

}  // namespace siodb::iomgr::dbengine
