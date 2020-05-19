// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

// System headers
#include <sys/types.h>

namespace siodb::utils {

/**
 * Returns OS user ID for a given user name.
 * @param name User name.
 * @return User ID.
 */
uid_t getOsUserId(const char* name);

/**
 * Returns OS user name for a given user ID.
 * @param uid User ID.
 * @return User name.
 * @throw std::runtime_error if user doesn't exist or some other error occurs.
 */
std::string getOsUserName(uid_t uid);

/**
 * Returns OS group ID for a given group name.
 * @param name Group name.
 * @return Group ID.
 */
gid_t getOsGroupId(const char* name);

/**
 * Returns OS group name for a given group ID.
 * @param  ID.
 * @return Group name.
 * @throw std::runtime_error if group doesn't exist or some other error occurs.
 */
std::string getOsGroupName(gid_t gid);

/**
 * Checks that OS user belongs to SIODB admin group.
 * @param uid User ID.
 * @param gid User group ID.
 * @return user name
 * @throw std::runtime_error if error occurs or if user doesn't belong to admin group.
 */
std::string checkUserBelongsToSiodbAdminGroup(uid_t uid, gid_t gid);

/**
 * Returns HOME directory of current user.
 * @return Home directory.
 * @throw std::system_error if error occurs.
 */
std::string getHomeDir();

}  // namespace siodb::utils
